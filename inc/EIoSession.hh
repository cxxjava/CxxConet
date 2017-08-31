/*
 * EIoSession.hh
 *
 *  Created on: 2013-8-19
 *      Author: cxxjava@163.com
 */

#ifndef EIOSESSION_HH_
#define EIOSESSION_HH_

#include "Efc.hh"

namespace efc {
namespace naf {

class EIoService;
class EIoFilterChain;

/**
 * A handle which represents connection between two end-points regardless of
 * transport types.
 * <p/>
 * {@link IoSession} provides user-defined attributes.  User-defined attributes
 * are application-specific data which are associated with a session.
 * It often contains objects that represents the state of a higher-level protocol
 * and becomes a way to exchange data between filters and handlers.
 * <p/>
 * <h3>Adjusting Transport Type Specific Properties</h3>
 * <p/>
 * You can simply downcast the session to an appropriate subclass.
 * </p>
 * <p/>
 */
abstract class EIoSession: virtual public EObject {
public:
	virtual ~EIoSession();

	/**
	 * @return a unique identifier for this session.  Every session has its own
	 * ID which is different from each other.
	 */
	virtual long getId();

	/**
	 * @return the {@link IoService} which provides I/O service to this session.
	 */
	virtual EIoService* getService();

	/**
	 * @return the filter chain that only affects this session.
	 */
	virtual EIoFilterChain* getFilterChain();

	/**
	 * @return a {@link ReadFuture} which is notified when a new message is
	 * received, the connection is closed or an exception is caught.  This
	 * operation is especially useful when you implement a client application.
	 * TODO : Describe here how we enable this feature.
	 * However, please note that this operation is disabled by default and
	 * throw {@link IllegalStateException} because all received events must be
	 * queued somewhere to support this operation, possibly leading to memory
	 * leak.  This means you have to keep calling {@link #read()} once you
	 * enabled this operation.  To enable this operation, please call
	 * {@link IoSessionConfig#setUseReadOperation(boolean)} with <tt>true</tt>.
	 *
	 * @throws IllegalStateException if
	 * {@link IoSessionConfig#setUseReadOperation(boolean) useReadOperation}
	 * option has not been enabled.
	 */
	virtual sp<EObject> read() = 0;

	/**
	 * Writes the specified <code>message</code> to remote peer.  This
	 * operation is asynchronous; {@link IoHandler#messageSend(IoSession,Object)}
	 * will be invoked when the message is actually sent to remote peer.
	 * You can also wait for the returned {@link WriteFuture} if you want
	 * to wait for the message actually written.
	 *
	 * @param data The message data to write
	 * @param size The message data size
     * @return The associated WriteFuture
	 */
	virtual boolean write(sp<EObject> message) = 0;

	/**
	 * Closes this session immediately or after all queued write requests
	 * are flushed.  This operation is asynchronous.  Wait for the returned
	 * {@link CloseFuture} if you want to wait for the session actually closed.
	 *
	 */
	virtual void close() = 0;

	/**
	 * @return <code>true</tt> if the session has started and initialized a SslEngine,
	 * <code>false</code> if the session is not yet secured (the handshake is not completed)
	 * or if SSL is not set for this session, or if SSL is not even an option.
	 */
	virtual boolean isSecured() = 0;

	/**
	 * Returns the socket address of remote peer.
	 */
	virtual EInetSocketAddress* getRemoteAddress() = 0;

	/**
	 * Returns the socket address of local machine which is associated with this
	 * session.
	 */
	virtual EInetSocketAddress* getLocalAddress() = 0;

	/**
	 * @return the session's creation time in milliseconds
	 */
	virtual llong getCreationTime();

	/**
	 * Returns the time in millis when I/O occurred lastly.
	 */
	virtual llong getLastIoTime();

	/**
	 * Returns the time in millis when read operation occurred lastly.
	 */
	virtual llong getLastReadTime();

	/**
	 * Returns the time in millis when write operation occurred lastly.
	 */
	virtual llong getLastWriteTime();

public:
	EHashMap<llong, sp<EObject> > attributes;

	/**
	 *
	 */
	void* attach(void* ob);
	void* attachment();

protected:
	friend class EIoFilterChain;

	/** The session ID */
	long sessionId;

	/* Statistics variables */
	llong creationTime;
	llong lastReadTime;
	llong lastWriteTime;
	long readBytes;
	long writtenBytes;
	long readMessages;
	long writtenMessages;

	EIoService* service;

	EAtomicReference<void> attachment_;

	/** The FilterChain created for this session */
	EIoFilterChain* filterChain;

	/** An id generator guaranteed to generate unique IDs for the session */
	static long idGenerator;

	/**
	 * Create a Session for a service
	 *
	 * @param service the Service for this session
	 */
	EIoSession(EIoService* service);

	/**
	 * Increase the number of read bytes
	 *
	 * @param increment The number of read bytes
	 * @param currentTime The current time
	 */
	void increaseReadBytes(long increment, llong currentTime);

	/**
	 * Increase the number of read messages
	 *
	 * @param currentTime The current time
	 */
	void increaseReadMessages(llong currentTime);

	/**
	 * Increase the number of written bytes
	 *
	 * @param increment The number of written bytes
	 * @param currentTime The current time
	 */
	void increaseWrittenBytes(int increment, llong currentTime);

	/**
	 * Increase the number of written messages
	 *
	 * @param currentTime The current tile
	 */
	void increaseWrittenMessages(llong currentTime);
};

} /* namespace naf */
} /* namespace efc */

#endif /* EIOSESSION_HH_ */
