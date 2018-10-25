#include "es_main.h"
#include "ENaf.hh"

#include "http_parser/http_parser.h"

#include "../filter/http/EHttpCodecFilter.hh"
#include "../filter/http/EHttpRequest.hh"
#include "../filter/http/EHttpResponse.hh"

using namespace filter::http;

#define LOG(fmt,...) ESystem::out->printfln(fmt, ##__VA_ARGS__)

#define SSL_FILE_PATH ""

#define USE_HTTP_FILTER 1
#define PRINT_STATISTICS 0

static ESocketAcceptor g_sa;

static void onListening(ESocketAcceptor* acceptor) {
	LOG("onListening...");

#if PRINT_STATISTICS
	while (!acceptor->isDisposed()) {
		sleep(10);

		EIoServiceStatistics* ss = acceptor->getStatistics();
		LOG("CumulativeManagedSessionCount=%ld", ss->getCumulativeManagedSessionCount());
		LOG("LargestManagedSessionCount=%ld", ss->getLargestManagedSessionCount());
		LOG("LargestReadBytesThroughput=%lf", ss->getLargestReadBytesThroughput());
		LOG("LargestReadMessagesThroughput=%lf", ss->getLargestReadMessagesThroughput());
		LOG("LargestWrittenBytesThroughput=%lf", ss->getLargestWrittenBytesThroughput());
		LOG("getLargestWrittenMessagesThroughput=%lf", ss->getLargestWrittenMessagesThroughput());
		LOG("LastIoTime=%ld", ss->getLastIoTime());
		LOG("LastReadTime=%ld", ss->getLastReadTime());
		LOG("LastWriteTime=%ld", ss->getLastWriteTime());
		LOG("ReadBytes=%ld", ss->getReadBytes());
		LOG("ReadBytesThroughput=%lf", ss->getReadBytesThroughput());
		LOG("ReadMessages=%ld", ss->getReadMessages());
		LOG("ReadMessagesThroughput=%lf", ss->getReadMessagesThroughput());
		LOG("WrittenBytes=%ld", ss->getWrittenBytes());
		LOG("WrittenBytesThroughput=%lf", ss->getWrittenBytesThroughput());
		LOG("WrittenMessages=%ld", ss->getWrittenMessages());
		LOG("WrittenMessagesThroughput=%lf", ss->getWrittenMessagesThroughput());
		LOG("\n");
	}
#endif
}

#if USE_HTTP_FILTER
static void onConnection(sp<ESocketSession>& session, ESocketAcceptor::Service* service) {
	LOG("onConnection: service=%s", service->toString().c_str());

//	sleep(10);

	sp<EHttpRequest> request;
	while ((request = dynamic_pointer_cast<EHttpRequest>(session->read())) != null) {
		LOG("%.*s", request->getBodyLen()+request->getHeadLen(), request->getHttpData());

		session->write(new EHttpResponse("OK"));  // send text.
//		session->write(new EFile("xxx.zip")); // send file.

//		break;
	}

	LOG("Out of Connection.");
}
#else

static int on_info(http_parser* p) {
	return 0;
}

static int on_message_complete(http_parser* p) {
	ESocketSession* session = (ESocketSession*)p->data;

	sp<EIoBuffer> response(EIoBuffer::allocate(256)->setAutoExpand(true));
	#define TEST_HTTP_DATA "HTTP/1.1 200 OK\r\nContent-Length: 3\r\n\r\nOK!"
	response->putString(TEST_HTTP_DATA);
	response->flip();
	session->write(response);

	return 0;
}

static int on_data(http_parser* p, const char *at, size_t length) {
	return 0;
}

static int on_body(http_parser* p, const char *at, size_t length) {
	LOG("%.*s", length, at);
	return 0;
}

static http_parser_settings settings = {
	.on_message_begin = on_info,
	.on_headers_complete = on_info,
	.on_message_complete = on_message_complete,
	.on_header_field = on_data,
	.on_header_value = on_data,
	.on_url = on_data,
	.on_status = on_data,
	.on_body = on_body
};

static void onConnection(sp<ESocketSession>& session, ESocketAcceptor::Service* service) {
	LOG("onConnection...");

	struct http_parser parser;
	http_parser_init(&parser, HTTP_REQUEST);

	sp<EIoBuffer> request;
	while(!session->getService()->isDisposed()) {
		try {
			request = dynamic_pointer_cast<EIoBuffer>(session->read());
		} catch (ESocketTimeoutException& e) {
			LOG("session read timeout.");
			continue;
		} catch (EIOException& e) {
			LOG("session read error.");
			break;
		}
		if (request == null) {
			LOG("session client closed.");
			break;
		}

		parser.data = session.get();

		size_t parsed = http_parser_execute(&parser, &settings, (const char *)request->current(), request->limit());
		if (parsed != request->limit()) {
			throw EIOException(__FILE__, __LINE__, "Not http data.");
		}
		LOG("%s", request->getString().c_str());
	}

	LOG("Out of Connection.");
}
#endif

static void test_echo_server() {
	EBlacklistFilter blf;
	EWhitelistFilter wlf;
	blf.block("localhost");
//	g_sa.getFilterChainBuilder()->addFirst("black", &blf);
	wlf.allow("localhost");
//	g_sa.getFilterChainBuilder()->addFirst("white", &wlf);
#if USE_HTTP_FILTER
	EHttpCodecFilter hcf;
	g_sa.getFilterChainBuilder()->addLast("http", &hcf);
#endif
	g_sa.setListeningHandler(onListening);
	g_sa.setConnectionHandler(onConnection);
//	sa.setMaxConnections(10);
	g_sa.setSoTimeout(30000);
	g_sa.setSessionIdleTime(EIdleStatus::WRITER_IDLE, 30);
	g_sa.bind("0.0.0.0", 8887, false, "serviceA");
	g_sa.bind("localhost", 8889, true, "serviceB", [](ESocketAcceptor::Service& service){
		LOG("service [%s] is active.", service.toString().c_str());

		if (service.sslActive) {
			sp<ESSLServerSocket> sss = dynamic_pointer_cast<ESSLServerSocket>(service.ss);
			sss->setSSLParameters(
						"./certs/tests-cert.pem",
						"./certs/tests-key.pem",
						null);
		}
	});
	g_sa.listen();
}

static void test_test(int argc, const char** argv) {
	test_echo_server();
}

static void sigfunc(int sig_no) {
	/**
	 * signal handler is running in main thread!!!
	 */
	LOG("signaled.");
	g_sa.shutdown();
}

MAIN_IMPL(testnaf) {
	printf("main()\n");

	ESystem::init(argc, argv);
	ELoggerManager::init("log4e.conf");

	signal(SIGINT, sigfunc);

	printf("inited.\n");

	int i = 0;
	try {
		boolean loop = EBoolean::parseBoolean(ESystem::getProgramArgument("loop"));
		do {
			test_test(argc, argv);

//		} while (++i < 5);
		} while (0);
	}
	catch (EException& e) {
		e.printStackTrace();
	}
	catch (...) {
		printf("catch all...\n");
	}

	printf("exit...\n");

	ESystem::exit(0);

	return 0;
}
