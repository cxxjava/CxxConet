/*
 * ESocketSession.cpp
 *
 *  Created on: 2017-3-16
 *      Author: cxxjava@163.com
 */

#include "../inc/ESocketSession.hh"

namespace efc {
namespace naf {

ESocketSession::~ESocketSession() {
	//
}

ESocketSession::ESocketSession(EIoService* service, sp<ESocket>& socket):
		EIoSession(service),
		socket_(socket), closed_(false) {
}

sp<EObject> ESocketSession::read() {
	if (ioBuffer == null) {
		ioBuffer = EIoBuffer::allocate(ES_MAX(socket_->getReceiveBufferSize(), 512));
		ioBuffer->setAutoExpand(true);
	}

	EInputStream* is = socket_->getInputStream();

	// try it for packet splicing.
	sp<EObject> out = filterChain->fireMessageReceived(null);
	if (out != null) {
		return out;
	}

	// else read next.

RESUME:
	ioBuffer->clear();
	int n = is->read(ioBuffer->current(), ioBuffer->limit());
	if (n > 0) {
		ioBuffer->position(n);
		ioBuffer->flip();

		// on session message received.
		out = filterChain->fireMessageReceived(ioBuffer);
		if (out == null) {
			goto RESUME;
		}
		return out;
	}

	if (n == -1) { // EOF
		return null;
	}

	throw EIOException(__FILE__, __LINE__, "socket session read.");
}

boolean ESocketSession::write(sp<EObject> message) {
	EOutputStream* os = socket_->getOutputStream();

	// on session message send.
	sp<EObject> out = filterChain->fireMessageSend(message);

	sp<EIoBuffer> ib = dynamic_pointer_cast<EIoBuffer>(out);
	if (ib != null) {
		os->write(ib->current(), ib->remaining());
		return true;
	}

	sp<EFile> file = dynamic_pointer_cast<EFile>(out);
	if (file != null) {
		socket_->sendfile(file.get());
		return true;
	}

	return false;
}

void ESocketSession::close() {
	if (!closed_) {
		filterChain->fireSessionClosed();
		socket_->close();
		closed_ = true;
	}
}

EInetSocketAddress* ESocketSession::getRemoteAddress() {
	return socket_->getRemoteSocketAddress();
}

EInetSocketAddress* ESocketSession::getLocalAddress() {
	return socket_->getLocalSocketAddress();
}

boolean ESocketSession::isSecured() {
	return dynamic_pointer_cast<ESSLSocket>(socket_) != null;
}

boolean ESocketSession::isClosed() {
	return closed_;
}

sp<ESocket> ESocketSession::getSocket() {
	return socket_;
}

} /* namespace naf */
} /* namespace efc */
