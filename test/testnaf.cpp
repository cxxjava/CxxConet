#include "main.hh"
#include "ENaf.hh"
#include "http_parser.h"

#define LOG(fmt,...) ESystem::out->println(fmt, ##__VA_ARGS__)

#define USE_SSL 0
#define SSL_FILE_PATH ""

#define USE_HTTP_FILTER 1
#define PRINT_STATISTICS 1

static void onListening(NSocketAcceptor* acceptor) {
	LOG("onListening...");

#if PRINT_STATISTICS
	while (!acceptor->isDisposed()) {
		sleep(10);

		NIoServiceStatistics* ss = acceptor->getStatistics();
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
static void onConnection(NSocketSession* session) {
	LOG("onConnection...");

//	sleep(10);

	sp<NHttpRequest> request;
	while ((request = dynamic_pointer_cast<NHttpRequest>(session->read())) != null) {
		LOG("%.*s", request->m_BodyLen+request->m_HeadLen, request->m_HttpData);

		session->write(new NHttpResponse());  // send text.
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
	NSocketSession* session = (NSocketSession*)p->data;

	sp<NIoBuffer> response(NIoBuffer::allocate(256)->setAutoExpand(true));
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

static void onConnection(NSocketSession* session) {
	LOG("onConnection...");

	struct http_parser parser;
	http_parser_init(&parser, HTTP_REQUEST);

	sp<NIoBuffer> request;
	while(!session->getService()->isDisposed()) {
		try {
			request = dynamic_pointer_cast<NIoBuffer>(session->read());
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

		parser.data = session;

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
	NSocketAcceptor sa;
	NBlacklistFilter blf;
	NWhitelistFilter wlf;
	blf.block("localhost");
//	sa.getFilterChainBuilder()->addFirst("black", &blf);
	wlf.allow("localhost");
//	sa.getFilterChainBuilder()->addFirst("white", &wlf);
	NHttpCodecFilter hcf;
#if USE_HTTP_FILTER
	sa.getFilterChainBuilder()->addLast("http", &hcf);
#endif
	sa.setListeningHandler(onListening);
	sa.setConnectionHandler(onConnection);
//	sa.setMaxConnections(10);
	sa.setSoTimeout(3000);
	sa.setSessionIdleTime(NIdleStatus::WRITER_IDLE, 30);
	sa.bind("0.0.0.0", 8887);
//	sa.bind("localhost", 8889);
#if USE_SSL
	sa.setSSLParameters(null,
			SSL_FILE_PATH "./certs/tests-cert.pem",
			SSL_FILE_PATH "./certs/tests-key.pem",
			null, null);
#endif
	sa.listen();
}

static void test_test(int argc, const char** argv) {
	test_echo_server();
}

MAIN_IMPL(testnaf) {
	printf("main()\n");

	ESystem::init(argc, argv);
	ELoggerManager::init("log4e.conf");

	printf("inited.\n");

	int i = 0;
	try {
		boolean loop = EBoolean::parseBoolean(ESystem::getProgramArgument("loop"));
		do {
			test_test(argc, argv);

//		} while (++i < 5);
		} while (1);
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
