/*
 * ESocketAcceptor.hh
 *
 *  Created on: 2013-8-19
 *      Author: cxxjava@163.com
 */

#ifndef ESOCKETACCEPTOR_HH_
#define ESOCKETACCEPTOR_HH_

#include "./EIoService.hh"
#include "./ESocketSession.hh"
#include "./EIoFilterChainBuilder.hh"

namespace efc {
namespace naf {

class EManagedSession;

/**
 * {@link IoAcceptor} for socket transport (TCP/IP).  This class
 * handles incoming TCP/IP based socket connections.
 *
 */

class ESocketAcceptor: virtual public EIoService {
public:
	enum Status {
		INITED,
		RUNNING,
		DISPOSING,
		DISPOSED
	};

	class Service: public EObject {
	public:
		sp<EServerSocket> ss;
		boolean sslActive;
		EString serviceName;
		EInetSocketAddress boundAddress;
		virtual EString toString() {
			return boundAddress.toString() + ", ssl=" + sslActive + ", service=" + serviceName;
		}
	private:
		friend class ESocketAcceptor;
		Service(const char* name, boolean ssl, const char* hostname, int port): sslActive(ssl), serviceName(name), boundAddress(hostname, port) {}
		Service(const char* name, boolean ssl, EInetSocketAddress* localAddress): sslActive(ssl), serviceName(name), boundAddress(*localAddress) {}
	};

public:
	virtual ~ESocketAcceptor();

	ESocketAcceptor();

	/**
	 * SSL support.
	 */
	void setSSLParameters(const char* dh_file, const char* cert_file,
			const char* private_key_file, const char* passwd,
			const char* CAfile);

	/**
	 * @see ServerSocket#getReuseAddress()
	 */
	virtual boolean isReuseAddress();

	/**
	 * @see ServerSocket#setReuseAddress(boolean)
	 */
	virtual void setReuseAddress(boolean on);

	/**
	 * Returns the size of the backlog.
	 */
	virtual int getBacklog();

	/**
	 * Sets the size of the backlog.  This can only be done when this
	 * class is not bound
	 */
	virtual void setBacklog(int backlog);

	/**
	 * @see ServerSocket#setSoTimeout()
	 */
	virtual void setSoTimeout(int timeout) THROWS(ESocketException);

	/**
	 * @see ServerSocket#getSoTimeout()
	 */
	virtual int getSoTimeout() THROWS(EIOException);

	/**
	 * @see ServerSocket#setReceiveBufferSize()
	 */
	virtual void setReceiveBufferSize (int size) THROWS(ESocketException);

	/**
	 * @see ServerSocket#getReceiveBufferSize()
	 */
	virtual int getReceiveBufferSize() THROWS(ESocketException);

	/**
	 *
	 */
	virtual void setMaxConnections(int connections);

	/**
	 *
	 */
	virtual int getMaxConnections();

	/**
	 *
	 */
	virtual void setSessionIdleTime(EIdleStatus status, int seconds);

	/**
	 *
	 */
	virtual int getSessionIdleTime(EIdleStatus status);

	/**
	 *
	 */
	virtual int getWorkThreads();

	/**
	 *
	 */
	virtual int getManagedSessionCount();

	/**
	 *
	 */
	virtual EIoFilterChainBuilder* getFilterChainBuilder();

	/**
	 *
	 */
	virtual EIoServiceStatistics* getStatistics();

	/**
	 *
	 */
	virtual void setListeningHandler(std::function<void(ESocketAcceptor* acceptor)> handler);

	/**
	 *
	 */
	virtual void setConnectionHandler(std::function<void(ESocketSession* session, Service* service)> handler);

	/**
	 * Binds the <code>ServerSocket</code> to a specific address
     * (IP address and port number).
	 */
	virtual void bind(int port, boolean ssl=false, const char* name=null) THROWS(EIOException);
	virtual void bind(const char* hostname, int port, boolean ssl=false, const char* name=null) THROWS(EIOException);
	virtual void bind(EInetSocketAddress *localAddress, boolean ssl=false, const char* name=null) THROWS(EIOException);
	virtual void bind(EIterable<EInetSocketAddress*> *localAddresses, boolean ssl=false, const char* name=null) THROWS(EIOException);

	/**
	 * Listens for connections and loop run for network io events.
	 */
	virtual void listen() THROWS(EIOException);

	/**
	 * Only close accept socket for gracefully stop.
	 */
	virtual void shutdown();

	/**
	 * Signal acceptor to stop immediately.
	 */
	virtual void dispose();

	/**
	 * Returns <tt>true</tt> if and if only all resources of this processor
	 * have been disposed.
	 */
	virtual boolean isDisposed();

private:
	static sp<ELogger> logger;

	EFiberScheduler scheduler;

	typedef struct SSL {
		EString dh_file;
		EString cert_file;
		EString private_key_file;
		EString passwd;
		EString CAfile;
	} SSL;
	sp<SSL> ssl_;

	EHashSet<Service*> Services_;

	volatile Status status_;;
	boolean reuseAddress_;// = false;
	int backlog_;
	int timeout_;
	int bufsize_;
	EAtomicCounter maxConns_;// = -1;
	EAtomicCounter connections_;
	EAtomicCounter idleTimeForRead_;
	EAtomicCounter idleTimeForWrite_;

	int workThreads_;
	EManagedSession* managedSessions_;

	EIoFilterChainBuilder defaultFilterChain;

	EIoServiceStatistics stats_;

	std::function<void(ESocketAcceptor* acceptor)> listeningCallback_;
	std::function<void(ESocketSession* session, Service* service)> connectionCallback_;

	void startAccept(EFiberScheduler& scheduler, Service* service) THROWS(EIOException);
	void startClean(EFiberScheduler& scheduler, int tag) THROWS(EIOException);
	void startStatistics(EFiberScheduler& scheduler);
};

} /* namespace naf */
} /* namespace efc */

#endif /* ESOCKETACCEPTOR_HH_ */
