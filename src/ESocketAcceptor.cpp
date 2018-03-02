/*
 * ESocketAcceptor.cpp
 *
 *  Created on: 2017-3-16
 *      Author: cxxjava@163.com
 */

#include "../inc/ESocketAcceptor.hh"
#include "./EManagedSession.hh"

namespace efc {
namespace naf {

#define SOCKET_BACKLOG_MIN 512

sp<ELogger> ESocketAcceptor::logger = ELoggerManager::getLogger("ESocketAcceptor");

ESocketAcceptor::~ESocketAcceptor() {
	delete managedSessions_;
}

ESocketAcceptor::ESocketAcceptor() :
		status_(INITED),
		reuseAddress_(false),
		backlog_(SOCKET_BACKLOG_MIN),
		timeout_(0),
		bufsize_(-1),
		maxConns_(-1),
		workThreads_(EOS::active_processor_count()),
		stats_(this) {
	managedSessions_ = new EManagedSession(this);
}

void ESocketAcceptor::setSSLParameters(const char* dh_file, const char* cert_file,
		const char* private_key_file, const char* passwd,
		const char* CAfile) {
	ssl_ = new SSL();
	ssl_->dh_file = dh_file;
	ssl_->cert_file = cert_file;
	ssl_->private_key_file = private_key_file;
	ssl_->passwd = passwd;
	ssl_->CAfile = CAfile;
}

boolean ESocketAcceptor::isReuseAddress() {
	return reuseAddress_;
}

void ESocketAcceptor::setReuseAddress(boolean on) {
	reuseAddress_ = on;
}

int ESocketAcceptor::getBacklog() {
	return backlog_;
}

void ESocketAcceptor::setBacklog(int backlog) {
	backlog_ = ES_MAX(backlog, backlog_);
}

void ESocketAcceptor::setSoTimeout(int timeout) {
	timeout_ = timeout;
}

int ESocketAcceptor::getSoTimeout() {
	return timeout_;
}

void ESocketAcceptor::setReceiveBufferSize (int size) {
	bufsize_ = size;
}

int ESocketAcceptor::getReceiveBufferSize () {
	return bufsize_;
}

void ESocketAcceptor::setMaxConnections(int connections) {
	maxConns_ = connections;
}

int ESocketAcceptor::getMaxConnections() {
	return maxConns_.value();
}

void ESocketAcceptor::setSessionIdleTime(EIdleStatus status, int seconds) {
	if (seconds < 0) {
		throw EIllegalArgumentException(__FILE__, __LINE__, EString::formatOf("Illegal idle time: %d", seconds).c_str());
	}

	if ((status & READER_IDLE) == READER_IDLE) {
		idleTimeForRead_ = seconds;
	}
	if ((status & WRITER_IDLE) == WRITER_IDLE) {
		idleTimeForWrite_ = seconds;
	}
}

int ESocketAcceptor::getSessionIdleTime(EIdleStatus status) {
	if ((status & READER_IDLE) == READER_IDLE) {
		return idleTimeForRead_.value();
	}
	if ((status & WRITER_IDLE) == WRITER_IDLE) {
		return idleTimeForWrite_.value();
	}
	throw EIllegalArgumentException(__FILE__, __LINE__, EString::formatOf("Unknown idle status: %d", status).c_str());
}

int ESocketAcceptor::getWorkThreads() {
	return workThreads_;
}

int ESocketAcceptor::getManagedSessionCount() {
	return managedSessions_->getManagedSessionCount();
}

EIoFilterChainBuilder* ESocketAcceptor::getFilterChainBuilder() {
	return &defaultFilterChain;
}

EIoServiceStatistics* ESocketAcceptor::getStatistics() {
	return &stats_;
}

void ESocketAcceptor::setListeningHandler(std::function<void(ESocketAcceptor* acceptor)> handler) {
	listeningCallback_ = handler;
}

void ESocketAcceptor::setConnectionHandler(std::function<void(ESocketSession* session, Service* service)> handler) {
	connectionCallback_ = handler;
}

void ESocketAcceptor::bind(int port, boolean ssl, const char* name) {
	Services_.add(new Service(name, ssl, "127.0.0.1", port));
}

void ESocketAcceptor::bind(const char* hostname, int port, boolean ssl, const char* name) {
	Services_.add(new Service(name, ssl, hostname, port));
}

void ESocketAcceptor::bind(EInetSocketAddress* localAddress, boolean ssl, const char* name) {
	if (!localAddress) {
		throw ENullPointerException(__FILE__, __LINE__, "localAddress");
	}
	Services_.add(new Service(name, ssl, localAddress));
}

void ESocketAcceptor::bind(EIterable<EInetSocketAddress*>* localAddresses, boolean ssl, const char* name) {
	if (!localAddresses) {
		throw ENullPointerException(__FILE__, __LINE__, "localAddresses");
	}
	sp < EIterator<EInetSocketAddress*> > iter = localAddresses->iterator();
	while (iter->hasNext()) {
		Services_.add(new Service(name, ssl, iter->next()));
	}
}

void ESocketAcceptor::listen() {
	try {
		// fibers balance
		scheduler.setBalanceCallback([](EFiber* fiber, int threadNums){
			long tag = fiber->getTag();
			if (tag == 0) {
				return 0;   // accept fibers
			} else if (tag > 0) {
				return (int)tag; // clean fibers
			} else {
				int fid = fiber->getId();
				return fid % (threadNums - 1) + 1; // balance to other's threads.
			}
		});

		status_ = RUNNING;

		// create client socket work-thread's leader fiber for clean idle socket.
		for (int i=1; i<workThreads_; i++) {
			this->startClean(scheduler, i);
		}

		// accept loop
		sp<EIterator<Service*> > iter = Services_.iterator();
		while (iter->hasNext()) {
			this->startAccept(scheduler, iter->next());
		}

		// start statistics
		this->startStatistics(scheduler);

		// on listening callback
		if (listeningCallback_ != null) {
			scheduler.schedule([this](){
				listeningCallback_(this);
			});
		}

		// wait for fibers work done.
		scheduler.join(EOS::active_processor_count());
	}
	catch (EException& e) {
		logger->error__(__FILE__, __LINE__, e.toString().c_str());
	}
}

void ESocketAcceptor::dispose() {
	// set dispose flag
	status_ = DISPOSED;

	// fibier interrupt
	scheduler.interrupt();

	// accept notify
	sp<EIterator<Service*> > iter = Services_.iterator();
	while (iter->hasNext()) {
		Service* sv = iter->next();
		if (sv->ss != null) {
			//FIXME: http://bbs.chinaunix.net/forum.php?mod=viewthread&action=printable&tid=1844321
			//sv->ss->close();
			ESocket s("127.0.0.1", sv->ss->getLocalPort());
		}
	}
}

void ESocketAcceptor::shutdown() {
	// set dispose flag
	status_ = DISPOSING;

	// accept notify
	sp<EIterator<Service*> > iter = Services_.iterator();
	while (iter->hasNext()) {
		Service* sv = iter->next();
		if (sv->ss != null) {
			//FIXME: http://bbs.chinaunix.net/forum.php?mod=viewthread&action=printable&tid=1844321
			//sv->ss->close();
			ESocket s("127.0.0.1", sv->ss->getLocalPort());
		}
	}
}

boolean ESocketAcceptor::isDisposed() {
	return status_ == DISPOSED;
}

//=============================================================================

void ESocketAcceptor::startAccept(EFiberScheduler& scheduler, Service* service) {
	EInetSocketAddress& socketAddress = service->boundAddress;

	sp<EFiber> acceptFiber = scheduler.schedule([&,service,this](){
		if (ssl_ != null && service->sslActive) {
			ESSLServerSocket* sss = new ESSLServerSocket();
			boolean r = sss->setSSLParameters(ssl_->dh_file.c_str(),
						ssl_->cert_file.c_str(),
						ssl_->private_key_file.c_str(),
						ssl_->passwd.c_str(),
						ssl_->CAfile.c_str());
			if (!r) {
				throw EIOException(__FILE__, __LINE__, "SSL error.");
			}
			service->ss = sss;
		} else {
			service->ss = new EServerSocket();
		}
		service->ss->setReuseAddress(reuseAddress_);
		if (timeout_ > 0) {
			service->ss->setSoTimeout(timeout_);
		}
		if (bufsize_ > 0) {
			service->ss->setReceiveBufferSize(bufsize_);
		}
		service->ss->bind(&socketAddress, backlog_);

		while (status_ == RUNNING) {
			try {
				// accept
				sp<ESocket> socket = service->ss->accept();
				if (socket != null) {
					try {
						sp<ESocketSession> session(new ESocketSession(this, socket));

						// reach the max connections.
						int maxconns = maxConns_.value();
						if ((status_ >= DISPOSING) || (maxconns > 0 && connections_.value() >= maxconns)) {
							socket->close();
							EThread::yield(); //?
							continue;
						}
						connections_++;

						// statistics
						stats_.cumulativeManagedSessionCount.incrementAndGet();
						if (connections_.value() > stats_.largestManagedSessionCount.get()) {
							stats_.largestManagedSessionCount.set(connections_.value());
						}

						scheduler.schedule([session,service,this](){
							ON_SCOPE_EXIT(
								connections_--;

								// remove from session manager.
								managedSessions_->removeSession(session->getSocket()->getFD());
								session->close();
							);

							// add to session manager.
							managedSessions_->addSession(session->getSocket()->getFD(), session.get());

							// set so_timeout option.
							if (timeout_ > 0) {
								session->getSocket()->setSoTimeout(timeout_);
							}

							// on session create.
							boolean created = session->getFilterChain()->fireSessionCreated();
							if (!created) {
								return;
							}

							if (connectionCallback_ != null) {
								try {
									connectionCallback_(session.get(), service);
								} catch (EIOException& e) {
									logger->error__(__FILE__, __LINE__, e.toString().c_str());
								} catch (...) {
									logger->error__(__FILE__, __LINE__, "error");
								}
							}
						});
					} catch (EThrowable& t) {
						logger->error__(__FILE__, __LINE__, t.toString().c_str());
					} catch (...) {
						logger->error__(__FILE__, __LINE__, "error");
					}
				}
			} catch (EInterruptedException& e) {
				logger->info__(__FILE__, __LINE__, "interrupted");
				break;
			} catch (ESocketTimeoutException& e) {
				// nothing to do.
			} catch (EThrowable& t) {
				logger->error__(__FILE__, __LINE__, t.toString().c_str());
				break;
			}
		}

		logger->info__(__FILE__, __LINE__, "accept closed.");
		service->ss->close();
	});
	acceptFiber->setTag(0); //tag: 0
}

void ESocketAcceptor::startClean(EFiberScheduler& scheduler, int tag) {
	sp<EFiber> leadFiber = scheduler.schedule([&,this](){
		logger->debug__(__FILE__, __LINE__, "I'm clean fiber, thread id=%ld", EThread::currentThread()->getId());

		try {
			EHashMap<int, EIoSession*>* threadManagedSessions = managedSessions_->getCurrentThreadManagedSessions();

			while (status_ == RUNNING || (status_ == DISPOSING && connections_.value() > 0)) {
				int maxsecs = 3; // sleep for 1s-10s second.
				int idleread = idleTimeForRead_.value();
				int idlewrite = idleTimeForWrite_.value();
				int status = 0;
				if (idleread > 0) {
					status |= READER_IDLE;
					maxsecs = ES_MIN(idleread, maxsecs);
				}
				if (idlewrite > 0) {
					status |= WRITER_IDLE;
					maxsecs = ES_MIN(idlewrite, maxsecs);
				}
				int seconds = ES_MAX(maxsecs, 1);

				sleep(seconds); //!

				llong currTime = ESystem::currentTimeMillis();
				sp<EIterator<EIoSession*> > iter = threadManagedSessions->values()->iterator();
				while (iter->hasNext()) {
					ESocketSession* session = dynamic_cast<ESocketSession*>(iter->next());

					llong lastIoTime = ELLong::MAX_VALUE;
					if ((status & READER_IDLE) == READER_IDLE) {
						lastIoTime = ES_MIN(session->getLastReadTime(), lastIoTime);
					}
					if ((status & WRITER_IDLE) == WRITER_IDLE) {
						lastIoTime = ES_MIN(session->getLastWriteTime(), lastIoTime);
					}

					if (currTime - lastIoTime > (maxsecs * 1000)) {
						//shutdown socket by server.
						session->getSocket()->shutdownInput();
					}
				}
			}
		} catch (EInterruptedException& e) {
			logger->info__(__FILE__, __LINE__, "interrupted");
		} catch (EThrowable& t) {
			logger->error__(__FILE__, __LINE__, t.toString().c_str());
		}
	});
	leadFiber->setTag(tag);  //tag: 1-N
}

void ESocketAcceptor::startStatistics(EFiberScheduler& scheduler) {
	sp<EFiber> staticsticsFiber = scheduler.schedule([this](){
		try {
			llong currentTime = ESystem::currentTimeMillis();
			int count = 0;

			while (status_ == RUNNING || (status_ == DISPOSING && connections_.value() > 0)) {
				int seconds = this->getStatistics()->getThroughputCalculationInterval();

				sleep(seconds); //!

				currentTime += seconds * 1000;
				// do time rectification
				if (count > 100) { //100?
					currentTime = ESystem::currentTimeMillis();
					count = 0;
				}

				this->getStatistics()->updateThroughput(currentTime);
			}
		} catch (EInterruptedException& e) {
			logger->info__(__FILE__, __LINE__, "interrupted");
		} catch (EThrowable& t) {
			logger->error__(__FILE__, __LINE__, t.toString().c_str());
		}
	});
	staticsticsFiber->setTag(0); //tag: 0
}

} /* namespace naf */
} /* namespace efc */
