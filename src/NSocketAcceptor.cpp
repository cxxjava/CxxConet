/*
 * NSocketAcceptor.cpp
 *
 *  Created on: 2017年3月16日
 *      Author: cxxjava@163.com
 */

#include "NSocketAcceptor.hh"
#include "NManagedSession.hh"

namespace efc {
namespace naf {

#define SOCKET_BACKLOG_MIN 512

sp<ELogger> NSocketAcceptor::logger = ELoggerManager::getLogger("NSocketAcceptor");

NSocketAcceptor::~NSocketAcceptor() {
	delete managedSessions_;
}

NSocketAcceptor::NSocketAcceptor() :
		disposing_(false),
		reuseAddress_(false),
		backlog_(SOCKET_BACKLOG_MIN),
		timeout_(0),
		bufsize_(-1),
		maxConns_(-1),
		workThreads_(EOS::active_processor_count()),
		stats_(this) {
	managedSessions_ = new NManagedSession(this);
}

void NSocketAcceptor::setSSLParameters(const char* dh_file, const char* cert_file,
		const char* private_key_file, const char* passwd,
		const char* CAfile) {
	ssl_ = new SSL();
	ssl_->dh_file = dh_file;
	ssl_->cert_file = cert_file;
	ssl_->private_key_file = private_key_file;
	ssl_->passwd = passwd;
	ssl_->CAfile = CAfile;
}

boolean NSocketAcceptor::isReuseAddress() {
	return reuseAddress_;
}

void NSocketAcceptor::setReuseAddress(boolean on) {
	reuseAddress_ = on;
}

int NSocketAcceptor::getBacklog() {
	return backlog_;
}

void NSocketAcceptor::setBacklog(int backlog) {
	backlog_ = ES_MAX(backlog, backlog_);
}

void NSocketAcceptor::setSoTimeout(int timeout) {
	timeout_ = timeout;
}

int NSocketAcceptor::getSoTimeout() {
	return timeout_;
}

void NSocketAcceptor::setReceiveBufferSize (int size) {
	bufsize_ = size;
}

int NSocketAcceptor::getReceiveBufferSize () {
	return bufsize_;
}

void NSocketAcceptor::setMaxConnections(int connections) {
	maxConns_ = connections;
}

int NSocketAcceptor::getMaxConnections() {
	return maxConns_.value();
}

void NSocketAcceptor::setSessionIdleTime(NIdleStatus status, int seconds) {
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

int NSocketAcceptor::getSessionIdleTime(NIdleStatus status) {
	if ((status & READER_IDLE) == READER_IDLE) {
		return idleTimeForRead_.value();
	}
	if ((status & WRITER_IDLE) == WRITER_IDLE) {
		return idleTimeForWrite_.value();
	}
	throw EIllegalArgumentException(__FILE__, __LINE__, EString::formatOf("Unknown idle status: %d", status).c_str());
}

int NSocketAcceptor::getWorkThreads() {
	return workThreads_;
}

int NSocketAcceptor::getManagedSessionCount() {
	return managedSessions_->getManagedSessionCount();
}

NIoFilterChainBuilder* NSocketAcceptor::getFilterChainBuilder() {
	return &defaultFilterChain;
}

NIoServiceStatistics* NSocketAcceptor::getStatistics() {
	return &stats_;
}

void NSocketAcceptor::setListeningHandler(std::function<void(NSocketAcceptor* acceptor)> handler) {
	listeningCallback_ = handler;
}

void NSocketAcceptor::setConnectionHandler(std::function<void(NSocketSession* session)> handler) {
	connectionCallback_ = handler;
}

void NSocketAcceptor::bind(int port) {
	boundAddresses_.add(new EInetSocketAddress("127.0.0.1", port));
}

void NSocketAcceptor::bind(const char* hostname, int port) {
	boundAddresses_.add(new EInetSocketAddress(hostname, port));
}

void NSocketAcceptor::bind(EInetSocketAddress* localAddress) {
	if (!localAddress) {
		throw ENullPointerException(__FILE__, __LINE__, "localAddress");
	}
	boundAddresses_.add(new EInetSocketAddress(*localAddress));
}

void NSocketAcceptor::bind(EIterable<EInetSocketAddress*>* localAddresses) {
	if (!localAddresses) {
		throw ENullPointerException(__FILE__, __LINE__, "localAddresses");
	}
	sp < EIterator<EInetSocketAddress*> > iter = localAddresses->iterator();
	while (iter->hasNext()) {
		boundAddresses_.add(new EInetSocketAddress(*iter->next()));
	}
}

void NSocketAcceptor::listen() {
	try {
		EFiberScheduler scheduler;

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

		// create client socket work-thread's leader fiber for clean idle socket.
		for (int i=1; i<workThreads_; i++) {
			this->startClean(scheduler, i);
		}

		// accept loop
		sp<EIterator<EInetSocketAddress*> > iter = boundAddresses_.iterator();
		while (iter->hasNext()) {
			EInetSocketAddress* isa = iter->next();
			this->startAccept(scheduler, *isa);
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

void NSocketAcceptor::dispose() {
	disposing_ = true;
}

boolean NSocketAcceptor::isDisposed() {
	return disposing_;
}

//=============================================================================

void NSocketAcceptor::startAccept(EFiberScheduler& scheduler, EInetSocketAddress& socketAddress) {
	sp<EFiber> acceptFiber = scheduler.schedule([&,this](){
		sp<EServerSocket> ss;
		if (ssl_ != null) {
			ESSLServerSocket* sss = new ESSLServerSocket();
			boolean r = sss->setSSLParameters(ssl_->dh_file.c_str(),
						ssl_->cert_file.c_str(),
						ssl_->private_key_file.c_str(),
						ssl_->passwd.c_str(),
						ssl_->CAfile.c_str());
			if (!r) {
				throw EIOException(__FILE__, __LINE__, "SSL error.");
			}
			ss = sss;
		} else {
			ss = new EServerSocket();
		}
		ss->setReuseAddress(reuseAddress_);
		if (timeout_ > 0) {
			ss->setSoTimeout(timeout_);
		}
		if (bufsize_ > 0) {
			ss->setReceiveBufferSize(bufsize_);
		}
		ss->bind(&socketAddress, backlog_);

		while (!disposing_) {
			try {
				// accept
				sp<ESocket> socket = ss->accept();
				if (socket != null) {
					try {
						sp<NSocketSession> session(new NSocketSession(this, socket));

						// reach the max connections.
						int maxconns = maxConns_.value();
						if (disposing_ || (maxconns > 0 && connections_.value() >= maxconns)) {
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

						scheduler.schedule([session,this](){
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
									connectionCallback_(session.get());
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
			} catch (ESocketTimeoutException& e) {
				// nothing to do.
			} catch (EThrowable& t) {
				logger->error__(__FILE__, __LINE__, t.toString().c_str());
			}
		}
	});
	acceptFiber->setTag(0); //tag: 0
}

void NSocketAcceptor::startClean(EFiberScheduler& scheduler, int tag) {
	sp<EFiber> leadFiber = scheduler.schedule([&,this](){
		logger->debug__(__FILE__, __LINE__, "I'm clean fiber, thread id=%ld", EThread::currentThread()->getId());

		try {
			EHashMap<int, NIoSession*>* threadManagedSessions = managedSessions_->getCurrentThreadManagedSessions();

			while (!disposing_) {
				int maxsecs = EInteger::MAX_VALUE; // sleep for 1s-10s second.
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
				int seconds = ES_MAX(maxsecs/2, 1);

				sleep(seconds); //!

				llong currTime = ESystem::currentTimeMillis();
				sp<EIterator<NIoSession*> > iter = threadManagedSessions->values()->iterator();
				while (iter->hasNext()) {
					NSocketSession* session = dynamic_cast<NSocketSession*>(iter->next());

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
		} catch (EThrowable& t) {
			logger->error__(__FILE__, __LINE__, t.toString().c_str());
		}
	});
	leadFiber->setTag(tag);  //tag: 1-N
}

void NSocketAcceptor::startStatistics(EFiberScheduler& scheduler) {
	sp<EFiber> staticsticsFiber = scheduler.schedule([this](){
		try {
			llong currentTime = ESystem::currentTimeMillis();
			int count = 0;

			while (!disposing_) {
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
		} catch (EThrowable& t) {
			logger->error__(__FILE__, __LINE__, t.toString().c_str());
		}
	});
	staticsticsFiber->setTag(0); //tag: 0
}

} /* namespace naf */
} /* namespace efc */
