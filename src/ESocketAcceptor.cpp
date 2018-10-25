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

EFiberScheduler& ESocketAcceptor::getFiberScheduler() {
	return scheduler;
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

void ESocketAcceptor::setConnectionHandler(std::function<void(sp<ESocketSession>& session, Service* service)> handler) {
	connectionCallback_ = handler;
}

void ESocketAcceptor::bind(int port, boolean ssl, const char* name, std::function<void(Service& service)> listener) {
	Service* svc = new Service(name, ssl, "127.0.0.1", port);
	Services_.add(svc);
	if (listener != null) { listener(*svc); }
}

void ESocketAcceptor::bind(const char* hostname, int port, boolean ssl, const char* name, std::function<void(Service& service)> listener) {
	Service* svc = new Service(name, ssl, hostname, port);
	Services_.add(svc);
	if (listener != null) { listener(*svc); }
}

void ESocketAcceptor::bind(EInetSocketAddress* localAddress, boolean ssl, const char* name, std::function<void(Service& service)> listener) {
	if (!localAddress) {
		throw ENullPointerException(__FILE__, __LINE__, "localAddress");
	}
	Service* svc = new Service(name, ssl, localAddress);
	Services_.add(svc);
	if (listener != null) { listener(*svc); }
}

void ESocketAcceptor::bind(EIterable<EInetSocketAddress*>* localAddresses, boolean ssl, const char* name, std::function<void(Service& service)> listener) {
	if (!localAddresses) {
		throw ENullPointerException(__FILE__, __LINE__, "localAddresses");
	}
	sp < EIterator<EInetSocketAddress*> > iter = localAddresses->iterator();
	while (iter->hasNext()) {
		Service* svc = new Service(name, ssl, iter->next());
		Services_.add(svc);
		if (listener != null) { listener(*svc); }
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

		// create clean idle socket fibers for per-conn-thread.
		if (idleTimeForRead_.value() > 0 || idleTimeForWrite_.value() > 0) {
			for (int i=1; i<workThreads_; i++) {
				this->startClean(scheduler, i);
			}
		}

		// accept loop
		sp<EIterator<Service*> > iter = Services_.iterator();
		while (iter->hasNext()) {
			this->startAccept(scheduler, iter->next());
		}

		// start statistics
		this->startStatistics(scheduler);

		// on listening callback
		this->onListeningHandle();

		// wait for fibers work done.
		scheduler.join(EOS::active_processor_count());
	} catch (EInterruptedException& e) {
		logger->info__(__FILE__, __LINE__, "interrupted");
	} catch (EException& e) {
		logger->error__(__FILE__, __LINE__, e.toString().c_str());
	}
}

void ESocketAcceptor::signalAccept() {
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

void ESocketAcceptor::dispose() {
	// set dispose flag
	status_ = DISPOSED;

	// fibier interrupt
	scheduler.interrupt();

	// accept notify
	signalAccept();
}

void ESocketAcceptor::shutdown() {
	// set dispose flag
	status_ = DISPOSING;

	// accept notify
	signalAccept();
}

boolean ESocketAcceptor::isDisposed() {
	return status_ >= DISPOSING;
}

//=============================================================================

void ESocketAcceptor::startAccept(EFiberScheduler& scheduler, Service* service) {
	EInetSocketAddress& socketAddress = service->boundAddress;

	sp<EFiber> acceptFiber = new EFiberTarget([&,service,this](){
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
						sp<ESocketSession> session = newSession(this, socket);
						session->init(); // enable shared from this.

						// reach the max connections.
						int maxconns = maxConns_.value();
						if (isDisposed() || (maxconns > 0 && connections_.value() >= maxconns)) {
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

							try {
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

								// on connection.
								sp<ESocketSession> noconstss = session;
								this->onConnectionHandle(noconstss, service);
							} catch (EThrowable& t) {
								logger->error__(__FILE__, __LINE__, t.toString().c_str());
							} catch (...) {
								logger->error__(__FILE__, __LINE__, "error");
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

	scheduler.schedule(acceptFiber);
}

void ESocketAcceptor::startClean(EFiberScheduler& scheduler, int tag) {
	sp<EFiber> cleanFiber = new EFiberTarget([&,this](){
		logger->debug__(__FILE__, __LINE__, "I'm clean fiber, thread id=%ld", EThread::currentThread()->getId());

		try {
			EHashMap<int, EIoSession*>* threadManagedSessions = managedSessions_->getCurrentThreadManagedSessions();

			while (status_ == RUNNING || (connections_.value() > 0)) {
				int maxsecs = 3; // sleep for 1s-10s second.
				int idleread = idleTimeForRead_.value();
				int idlewrite = idleTimeForWrite_.value();
                int idlesecs = EInteger::MAX_VALUE;
				int status = 0;
				if (idleread > 0) {
					status |= READER_IDLE;
					maxsecs = ES_MIN(idleread, maxsecs);
                    idlesecs = ES_MIN(idlesecs, idleread);
				}
				if (idlewrite > 0) {
					status |= WRITER_IDLE;
					maxsecs = ES_MIN(idlewrite, maxsecs);
                    idlesecs = ES_MIN(idlesecs, idlewrite);
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

					if (currTime - lastIoTime > (idlesecs * 1000)) {
						//shutdown socket by server.
						session->getSocket()->shutdownInput();
					}
				}

				if (status_ == DISPOSED) {
					logger->info__(__FILE__, __LINE__, "disposed");
					break; //!
				}
			}
		} catch (EInterruptedException& e) {
			logger->info__(__FILE__, __LINE__, "interrupted");
		} catch (EThrowable& t) {
			logger->error__(__FILE__, __LINE__, t.toString().c_str());
		}

		logger->info__(__FILE__, __LINE__, "exit clean fiber.");
	});
	cleanFiber->setTag(tag);  //tag: 1-N

	scheduler.schedule(cleanFiber);
}

void ESocketAcceptor::startStatistics(EFiberScheduler& scheduler) {
	sp<EFiber> statisticsFiber = new EFiberTarget([this](){
		try {
			llong currentTime = ESystem::currentTimeMillis();
			int count = 0;

			while (status_ == RUNNING || (connections_.value() > 0)) {
				int seconds = this->getStatistics()->getThroughputCalculationInterval();

				sleep(seconds); //!

				currentTime += seconds * 1000;
				// do time rectification
				if (count > 100) { //100?
					currentTime = ESystem::currentTimeMillis();
					count = 0;
				}

				this->getStatistics()->updateThroughput(currentTime);

				if (status_ == DISPOSED) {
					logger->info__(__FILE__, __LINE__, "disposed");
					break; //!
				}
			}
		} catch (EInterruptedException& e) {
			logger->info__(__FILE__, __LINE__, "interrupted");
		} catch (EThrowable& t) {
			logger->error__(__FILE__, __LINE__, t.toString().c_str());
		}

		logger->info__(__FILE__, __LINE__, "exit statistics fiber.");
	});
	statisticsFiber->setTag(0); //tag: 0

	scheduler.schedule(statisticsFiber);
}

sp<ESocketSession> ESocketAcceptor::newSession(EIoService *service, sp<ESocket>& socket) {
	return new ESocketSession(this, socket);
}

void ESocketAcceptor::onListeningHandle() {
	if (listeningCallback_ != null) {
		scheduler.schedule([this](){
			listeningCallback_(this);
		});
	}
}

void ESocketAcceptor::onConnectionHandle(sp<ESocketSession>& session, Service* service) {
	if (connectionCallback_ != null) {
		try {
			connectionCallback_(session, service);
		} catch (EIOException& e) {
			logger->error__(__FILE__, __LINE__, e.toString().c_str());
		} catch (...) {
			logger->error__(__FILE__, __LINE__, "error");
		}
	}
}

} /* namespace naf */
} /* namespace efc */
