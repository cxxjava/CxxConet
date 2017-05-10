/*
 * NSocketAcceptor.hh
 *
 *  Created on: 2013-8-19
 *      Author: cxxjava@163.com
 */

#ifndef NSOCKETACCEPTOR_HH_
#define NSOCKETACCEPTOR_HH_

#include "NIoService.hh"
#include "NSocketSession.hh"
#include "NIoFilterChainBuilder.hh"

namespace efc {
namespace naf {

class NManagedSession;

/**
 * {@link IoAcceptor} for socket transport (TCP/IP).  This class
 * handles incoming TCP/IP based socket connections.
 *
 */

class NSocketAcceptor: virtual public NIoService {
public:
	virtual ~NSocketAcceptor();

	NSocketAcceptor();

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
	virtual void setSessionIdleTime(NIdleStatus status, int seconds);

	/**
	 *
	 */
	virtual int getSessionIdleTime(NIdleStatus status);

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
	virtual NIoFilterChainBuilder* getFilterChainBuilder();

	/**
	 *
	 */
	virtual NIoServiceStatistics* getStatistics();

	/**
	 *
	 */
	virtual void setListeningHandler(std::function<void(NSocketAcceptor* acceptor)> handler);

	/**
	 *
	 */
	virtual void setConnectionHandler(std::function<void(NSocketSession* session)> handler);

	/**
	 * Binds the <code>ServerSocket</code> to a specific address
     * (IP address and port number).
	 */
	virtual void bind(int port) THROWS(EIOException);
	virtual void bind(const char* hostname, int port) THROWS(EIOException);
	virtual void bind(EInetSocketAddress *localAddress) THROWS(EIOException);
	virtual void bind(EIterable<EInetSocketAddress*> *localAddresses) THROWS(EIOException);

	/**
	 * Listens for connections and loop run for network io events.
	 */
	virtual void listen() THROWS(EIOException);

	/**
	 * Signal acceptor to stop.
	 */
	virtual void dispose();

	/**
	 * Returns <tt>true</tt> if and if only all resources of this processor
	 * have been disposed.
	 */
	virtual boolean isDisposed();

private:
	static sp<ELogger> logger;

	typedef struct SSL {
		EString dh_file;
		EString cert_file;
		EString private_key_file;
		EString passwd;
		EString CAfile;
	} SSL;
	sp<SSL> ssl_;

	boolean disposing_;// = false;
	boolean reuseAddress_;// = false;
	int backlog_;
	int timeout_;
	int bufsize_;
	EHashSet<EInetSocketAddress*> boundAddresses_;
	EAtomicCounter maxConns_;// = -1;
	EAtomicCounter connections_;
	EAtomicCounter idleTimeForRead_;
	EAtomicCounter idleTimeForWrite_;

	int workThreads_;
	NManagedSession* managedSessions_;

	NIoFilterChainBuilder defaultFilterChain;

	NIoServiceStatistics stats_;

	std::function<void(NSocketAcceptor* acceptor)> listeningCallback_;
	std::function<void(NSocketSession* session)> connectionCallback_;

	void startAccept(EFiberScheduler& scheduler, EInetSocketAddress& socketAddress) THROWS(EIOException);
	void startClean(EFiberScheduler& scheduler, int tag) THROWS(EIOException);
	void startStatistics(EFiberScheduler& scheduler);
};

} /* namespace naf */
} /* namespace efc */

#endif /* NSOCKETACCEPTOR_HH_ */
