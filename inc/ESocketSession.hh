/*
 * ESocketSession.hh
 *
 *  Created on: 2016-1-19
 *      Author: cxxjava@163.com
 */

#ifndef ESOCKETSESSION_HH_
#define ESOCKETSESSION_HH_

#include "./EIoSession.hh"
#include "./EIoBuffer.hh"
#include "./EIoService.hh"
#include "./EIoFilterChain.hh"

namespace efc {
namespace naf {

class ESocketSession: public EIoSession, public enable_shared_from_this<ESocketSession> {
public:
	virtual ~ESocketSession();

	/**
	 *
	 * Creates a new instance of ESocketSession.
	 *
	 * @param service the associated IoService
	 * @param socket the associated socket
	 */
	ESocketSession(EIoService* service, sp<ESocket>& socket);

	virtual void init();

	virtual sp<EObject> read();
	virtual boolean write(sp<EObject> message);
	virtual void close();

	/**
	 * {@inheritDoc}
	 */
	virtual EInetSocketAddress* getRemoteAddress();

	/**
	 * {@inheritDoc}
	 */
	virtual EInetSocketAddress* getLocalAddress();

	/**
	 * {@inheritDoc}
	 */
	virtual boolean isSecured();

	/**
	 *
	 */
	boolean isClosed();

	/**
	 *
	 */
	sp<ESocket> getSocket();

	/**
	 *
	 */
	int bufferLimit() { return ioBufferLimit; }

private:
	sp<ESocket> socket_;
	boolean closed_;

	sp<EIoBuffer> ioBuffer;
	uint ioBufferLimit;
};

//=============================================================================

/**
 * Represents the type of idleness of {@link IoSession} or
 * {@link IoSession}.  There are three types of idleness:
 * <ul>
 *   <li>{@link #READER_IDLE} - No data is coming from the remote peer.</li>
 *   <li>{@link #WRITER_IDLE} - Session is not writing any data.</li>
 * </ul>
 * <p>
 * Idle time settings are all disabled by default.  You can enable them
 * using {@link ESocketAcceptor#setIdleTime(IdleStatus,int)}.
 *
 */
enum EIdleStatus {
	/**
	 * Represents the session status that no data is coming from the remote
	 * peer.
	 */
	READER_IDLE = 0x01,

	/**
	 * Represents the session status that the session is not writing any data.
	 */
	WRITER_IDLE = 0x02
};

} /* namespace naf */
} /* namespace efc */
#endif /* ESOCKETSESSION_HH_ */
