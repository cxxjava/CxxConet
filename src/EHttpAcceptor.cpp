/*
 * EHttpAcceptor.cpp
 *
 *  Created on: 2018-6-21
 *      Author: cxxjava@163.com
 */

#include "../inc/EHttpAcceptor.hh"
#include "../inc/EHttpSession.hh"

namespace efc {
namespace naf {

EHttpAcceptor::~EHttpAcceptor() {

}

EHttpAcceptor::EHttpAcceptor(boolean rwIoDetached, boolean workerDetached) :
		rwIoDetached_(rwIoDetached), workerDetached_(workerDetached), handler_(null), requestChannel(100) {
	if (workerDetached) {
		rwIoDetached_ = true; //! unsupport (false, true) mode.
	}
}

void EHttpAcceptor::listen() {
	if (!workerDetached_) {
		for (int i=0; i<WORKERS; i++) {
			this->getFiberScheduler().schedule([this](){
				while (true) {
					try {
						processRequest(this->requestChannel.read());
					} catch (...) {
						//...
					}
				}
			});
		}
	} else {
		for (int i=0; i<WORKERS; i++) {
			sp<EThread> worker = new EEThreadTarget([this](){
				while (true) {
					try {
						processRequest(this->requestChannel.read());
					} catch (...) {
						//...
					}
				}
			});
			EThread::setDaemon(worker, true);
			worker->start();
		}
	}

	ESocketAcceptor::listen(); //!!!
}

sp<ESocketSession> EHttpAcceptor::newSession(EIoService *service, sp<ESocket>& socket) {
	return dynamic_cast<ESocketSession*>(new EHttpSession(service, socket));
}

void EHttpAcceptor::onConnectionHandle(sp<ESocketSession>& session, ESocketAcceptor::Service* service) {
	// supper call
	ESocketAcceptor::onConnectionHandle(session, service);

	sp<EHttpSession> hs = dynamic_pointer_cast<EHttpSession>(session);

	// let write message in another fiber.
	if (rwIoDetached_) {
		detachWriteRoutine(hs);
	}

	if (handler_) {
		// on opened.
		handler_->sessionOpened(hs);
	}

	// let read message in current fiber.
	sp<EIoBuffer> request;
	while ((request = dynamic_pointer_cast<EIoBuffer>(session->read())) != null) {
		//
	}

	if (handler_) {
		// on closed.
		handler_->sessionClosed(hs);
	}
}

void EHttpAcceptor::setHttpHandler(EHttpHandler* handler) {
	handler_ = handler;
}

boolean EHttpAcceptor::isRWIoDetached() {
	return rwIoDetached_;
}

boolean EHttpAcceptor::isWorkerDetached() {
	return workerDetached_;
}

void EHttpAcceptor::processRequest(sp<EHttpRequest> request) {
	HeaderMapPtr headers = request->getHeaderMap();
	HeaderEntry* he = headers->Method();
	EString method = he->value().c_str();

	ActiveStream* stream = request->getHttpStream();
	sp<EHttpSession> session = stream->getHttpSession();
	sp<EHttpResponse> response(new EHttpResponse(stream));

	// service
	handler_->service(session, request, response);

	if (method.equalsIgnoreCase("GET")) {
		handler_->doGet(session, request, response);
	} else if (method.equalsIgnoreCase("HEAD")) {
		handler_->doHead(session, request, response);
	} else if (method.equalsIgnoreCase("POST")) {
		handler_->doPost(session, request, response);
	} else if (method.equalsIgnoreCase("PUT")) {
		handler_->doPut(session, request, response);
	} else if (method.equalsIgnoreCase("DELETE")) {
		handler_->doDelete(session, request, response);
	} else if (method.equalsIgnoreCase("OPTIONS")) {
		handler_->doOptions(session, request, response);
	} else if (method.equalsIgnoreCase("TRACE")) {
		handler_->doTrace(session, request, response);
	} else if (method.equalsIgnoreCase("PATCH")) {
		handler_->doPatch(session, request, response);
	}

	// response.
	if (isRWIoDetached()) {
		session->responseChannel.write(response);
	} else {
		stream->encodeHeaders(response->getHeaderMap(), (response->bodyData == null));

		if (response->bodyData != null) {
			Http::Buffer::OwnedImpl buffer;
			buffer.add(response->bodyData);
			stream->encodeData(buffer, false);
			buffer.clear();
			stream->encodeData(buffer, true);
		}
	}
}

void EHttpAcceptor::detachWriteRoutine(sp<EHttpSession>& session) {
	this->getFiberScheduler().scheduleInheritThread([session](){
		while (!session->isClosed()) {
			try {
				auto response = session->responseChannel.read();
				if (dynamic_pointer_cast<EndHttpResponse>(response) != null) {
					break;
				}

				ActiveStream* stream = response->stream;

				stream->encodeHeaders(response->headerMap, (response->bodyData == null));

				if (response->bodyData != null) {
					Http::Buffer::OwnedImpl buffer;
					buffer.add(response->bodyData);
					stream->encodeData(buffer, false);
					buffer.clear();
					stream->encodeData(buffer, true);
				}
			} catch (EInterruptedException& e) {
				break;
			}
		}
	});
}

} /* namespace naf */
} /* namespace efc */
