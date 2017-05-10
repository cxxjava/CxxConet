#include "main.hh"
#include "ENaf.hh"

#define LOG(fmt,...) //ESystem::out->println(fmt, ##__VA_ARGS__)

static void onConnection(NSocketSession* session) {
	LOG("onConnection...");

	sp<NIoBuffer> request;
	try {
		request = dynamic_pointer_cast<NIoBuffer>(session->read());
	} catch (ESocketTimeoutException& e) {
		LOG("session read timeout.");
		return;
	} catch (EIOException& e) {
		LOG("session read error.");
		return;
	}
	if (request == null) {
		LOG("session client closed.");
		return;
	}

	// echo.
	#define TEST_HTTP_DATA "HTTP/1.1 200 OK\r\nContent-Length: 3\r\n\r\nOK!"
	int len = strlen(TEST_HTTP_DATA);
	sp<NIoBuffer> respone = NIoBuffer::allocate(len);
	respone->put(TEST_HTTP_DATA, len);
	respone->flip();
	session->write(respone);

	LOG("Out of Connection.");
}

static void test_echo_performance() {
	NSocketAcceptor sa;
	sa.setConnectionHandler(onConnection);
	sa.setSoTimeout(3000);
	sa.setSessionIdleTime(NIdleStatus::WRITER_IDLE, 30);
	sa.bind("0.0.0.0", 8888);
	sa.listen();
}

MAIN_IMPL(testnaf_benchmark) {
	printf("main()\n");

	ESystem::init(argc, argv);
	ELoggerManager::init("log4e.conf");

	printf("inited.\n");

	int i = 0;
	try {
		boolean loop = EBoolean::parseBoolean(ESystem::getProgramArgument("loop"));
		do {
			test_echo_performance();

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
