/*
 * EHttpAcceptor.hh
 *
 *  Created on: 2018-6-21
 *      Author: cxxjava@163.com
 */

#ifndef EHTTPACCEPTOR_HH_
#define EHTTPACCEPTOR_HH_

#include "./ESocketAcceptor.hh"
#include "./EHttpHandler.hh"

namespace efc {
namespace naf {

/**
 * {@link IoAcceptor} for http transport (HTTP&HTTP2).  This class
 * handles incoming http based socket connections.
 *
 */

class EHttpAcceptor: public ESocketAcceptor {
public:
	static const int WORKERS = 10;

public:
	virtual ~EHttpAcceptor();

	EHttpAcceptor(boolean rwIoDetached=false, boolean workerDetached=false);

	virtual void listen() THROWS(EIOException);

	/**
	 *
	 */
	virtual void setHttpHandler(EHttpHandler* handler);

	/**
	 *
	 */
	boolean isRWIoDetached();

	/**
	 *
	 */
	boolean isWorkerDetached();

protected:
	boolean rwIoDetached_;
	boolean workerDetached_;
	EHttpHandler* handler_;

private:
	friend class ActiveStream;

	EFiberChannel<EHttpRequest> requestChannel;

	/**
	 * Override
	 */
	virtual sp<ESocketSession> newSession(EIoService *service, sp<ESocket>& socket);

	/**
	 * Override
	 */
	virtual void onConnectionHandle(sp<ESocketSession>& session, Service* service);

	/**
	 *
	 */
	void detachWriteRoutine(sp<EHttpSession>& session);

	/**
	 *
	 */
	void processRequest(sp<EHttpRequest> request);
};

} /* namespace naf */
} /* namespace efc */
#endif /* EHTTPACCEPTOR_HH_ */
