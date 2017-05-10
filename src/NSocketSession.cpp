/*
 * NSocketSession.cpp
 *
 *  Created on: 2017-3-16
 *      Author: cxxjava@163.com
 */

#include "NSocketSession.hh"

namespace efc {
namespace naf {

NSocketSession::~NSocketSession() {
	//
}

NSocketSession::NSocketSession(NIoService* service, sp<ESocket>& socket):
		NIoSession(service),
		socket_(socket), closed_(false) {
}

sp<EObject> NSocketSession::read() {
	if (ioBuffer == null) {
		ioBuffer = NIoBuffer::allocate(ES_MAX(socket_->getReceiveBufferSize(), 512));
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

boolean NSocketSession::write(sp<EObject> message) {
	EOutputStream* os = socket_->getOutputStream();

	// on session message send.
	sp<EObject> out = filterChain->fireMessageSend(message);

	sp<NIoBuffer> ib = dynamic_pointer_cast<NIoBuffer>(out);
	if (ib != null) {
		os->write(ib->current(), ib->remaining());
		return true;
	}

	sp<EFile> file = dynamic_pointer_cast<EFile>(out);
	if (file != null) {
		ERandomAccessFile raf(file.get(), "r");
		off_t offset = 0;
		socket_->sendfile(eso_fileno(raf.getFD()), &offset, file->length());
		return true;
	}

	return false;
}

void NSocketSession::close() {
	if (!closed_) {
		filterChain->fireSessionClosed();
		socket_->close();
		closed_ = true;
	}
}

EInetSocketAddress* NSocketSession::getRemoteAddress() {
	return socket_->getRemoteSocketAddress();
}

EInetSocketAddress* NSocketSession::getLocalAddress() {
	return socket_->getLocalSocketAddress();
}

boolean NSocketSession::isSecured() {
	return dynamic_pointer_cast<ESSLSocket>(socket_) != null;
}

boolean NSocketSession::isClosed() {
	return closed_;
}

sp<ESocket> NSocketSession::getSocket() {
	return socket_;
}

} /* namespace naf */
} /* namespace efc */
