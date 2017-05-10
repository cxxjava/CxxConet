/*
 * NSocketSession.hh
 *
 *  Created on: 2016-1-19
 *      Author: cxxjava@163.com
 */

#ifndef NSOCKETSESSION_HH_
#define NSOCKETSESSION_HH_

#include "NIoSession.hh"
#include "NIoBuffer.hh"
#include "NIoService.hh"
#include "NIoFilterChain.hh"

namespace efc {
namespace naf {

class NSocketSession: public NIoSession {
public:
	virtual ~NSocketSession();

	/**
	 *
	 * Creates a new instance of NSocketSession.
	 *
	 * @param service the associated IoService
	 * @param processor the associated IoProcessor
	 * @param ch the used channel
	 */
	NSocketSession(NIoService* service, sp<ESocket>& socket);

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

private:
	sp<ESocket> socket_;
	boolean closed_;

	sp<NIoBuffer> ioBuffer;
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
 * using {@link NSocketAcceptor#setIdleTime(IdleStatus,int)}.
 *
 */
enum NIdleStatus {
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
#endif /* NSOCKETSESSION_HH_ */
