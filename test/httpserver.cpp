#include "es_main.h"
#include "ENaf.hh"
#include "../inc/EHttpAcceptor.hh"

#define LOG(fmt,...) ESystem::out->printfln(fmt, ##__VA_ARGS__)

#define PRINT_STATISTICS 0

static EHttpAcceptor* g_sa = NULL;
static boolean g_shutdown = false;

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

static void onConnection(sp<ESocketSession>& session, ESocketAcceptor::Service* service) {
	LOG("onConnection: service=%s", service->toString().c_str());
}

class HttpHandler : public EHttpHandler {
public:
	virtual void service(sp<EHttpSession>& session, sp<EHttpRequest>& request, sp<EHttpResponse>& response) {
		LOG("service()...");
	}

	virtual void doGet(sp<EHttpSession>& session, sp<EHttpRequest>& request, sp<EHttpResponse>& response) {
		LOG("doGet()...");

		HeaderMapPtr headers = request->getHeaderMap();
		headers->iterate(
				  [](const HeaderEntry& header, void* context) -> HeaderMap::Iterate {
					printf("  '%s':'%s'",
									 header.key().c_str(), header.value().c_str());
					return HeaderMap::Iterate::Continue;
				  },
				  NULL);
		printf("\n");

		ELinkedList<sp<EIoBuffer> > body = request->getBodyData();
		for (auto buffer : body) {
			printf("%s\n", buffer->current());
		}

		// response.
		response->getHeaderMap().addReferenceKey(Headers::get().ContentLength, 5);
		response->write("kkk", 3);
		response->write("aaa", 3);
	}

	virtual void doPost(sp<EHttpSession>& session, sp<EHttpRequest>& request, sp<EHttpResponse>& response) {
			LOG("doPost()...");

			HeaderMapPtr headers = request->getHeaderMap();
			headers->iterate(
					  [](const HeaderEntry& header, void* context) -> HeaderMap::Iterate {
						printf("  '%s':'%s'",
										 header.key().c_str(), header.value().c_str());
						return HeaderMap::Iterate::Continue;
					  },
					  NULL);
			printf("\n");

			ELinkedList<sp<EIoBuffer> > body = request->getBodyData();
			for (auto buffer : body) {
				printf("%s\n", buffer->current());
			}

			// response.
			response->write("kkk", 3);
		}
};

static void test_echo_server() {
	EHttpAcceptor sa(false, true);
	g_sa = &sa;

	EBlacklistFilter blf;
	EWhitelistFilter wlf;
	HttpHandler handler;
	blf.block("localhost");
//sa.getFilterChainBuilder()->addFirst("black", &blf);
	wlf.allow("localhost");
//sa.getFilterChainBuilder()->addFirst("white", &wlf);
//	EHttpCodecFilter hcf;
//sa.getFilterChainBuilder()->addLast("http", &hcf);
	sa.setListeningHandler(onListening);
	sa.setConnectionHandler(onConnection);
//sa.setMaxConnections(10);
//sa.setSoTimeout(30000);
//sa.setSessionIdleTime(EIdleStatus::WRITER_IDLE, 30);
	sa.setReuseAddress(true);
	sa.setHttpHandler(&handler);
	sa.bind("0.0.0.0", 8887, false, "serviceA", [](ESocketAcceptor::Service& service){
		LOG("service [%s] is active.", service.toString().c_str());
	});
	sa.bind("localhost", 8889, true, "serviceB", [](ESocketAcceptor::Service& service){
		LOG("service [%s] is active.", service.toString().c_str());

		if (service.sslActive) {
			sp<ESSLServerSocket> sss = dynamic_pointer_cast<ESSLServerSocket>(service.ss);
			sss->setSSLParameters(
						"./certs/tests-cert.pem",
						"./certs/tests-key.pem",
						null);
			sss->setNegotiatedProtocols("h2", "http/1.1", NULL);
		}
	});
	sa.listen();
}

static void test_test(int argc, const char** argv) {
	test_echo_server();
}

static void sigfunc(int sig_no) {
	if (!g_shutdown) {
		g_shutdown = true;
	} else {
		ESystem::exit(-1);
	}

	/**
	 * signal handler is running in main thread!!!
	 */
	LOG("signaled.");
	g_sa->shutdown();
}

MAIN_IMPL(testnaf_http) {
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
