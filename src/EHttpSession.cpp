/*
 * EHttpSession.cpp
 *
 *  Created on: 2018-6-21
 *      Author: cxxjava@163.com
 */

#include "../inc/EHttpSession.hh"
#include "../inc/EHttpAcceptor.hh"

#include "../http/source/enum_to_int.h"

namespace efc {
namespace naf {

#define ARRLEN(x) (sizeof(x) / sizeof(x[0]))

#define MAKE_NV(NAME, VALUE)                                                   \
  {                                                                            \
    (uint8_t *)NAME, (uint8_t *)VALUE, sizeof(NAME) - 1, sizeof(VALUE) - 1,    \
        NGHTTP2_NV_FLAG_NONE                                                   \
  }

sp<EHttpSession> ActiveStream::getHttpSession() {
	return dynamic_pointer_cast<EHttpSession>(session->shared_from_this());
}

void ActiveStream::decode100ContinueHeaders(HeaderMapPtr&& headers) {
	// ignore for client request.
	ES_ASSERT(0);
}

//@see: envoy-1.6.0/source/common/http/conn_manager_impl.cc::decodeHeaders(...)
void ActiveStream::decodeHeaders(HeaderMapPtr&& headers, bool end_stream) {
	//	printf("decodeHeaders\n");

	// Check for maximum incoming header size. Both codecs have some amount of checking for maximum
	// header size. For HTTP/1.1 the entire headers data has be less than ~80K (hard coded in
	// http_parser). For HTTP/2 the default allowed header block length is 64k.
	// In order to have generally uniform behavior we also check total header size here and keep it
	// under 60K. Ultimately it would be nice to have a configuration option ranging from the largest
	// header size http_parser and nghttp2 will allow, down to 16k or 8k for
	// envoy users who do not wish to proxy large headers.
	if (headers->byteSize() > (60 * 1024)) {
		HeaderMapImpl headers{
			{Headers::get().Status, std::to_string(enumToInt(Code::RequestHeaderFieldsTooLarge))}};
		response_encoder->encodeHeaders(headers, true);
		return;
	}

	// Currently we only support relative paths at the application layer. We expect the codec to have
	// broken the path into pieces if applicable. NOTE: Currently the HTTP/1.1 codec does not do this
	// so we only support relative paths in all cases. https://tools.ietf.org/html/rfc7230#section-5.3
	// We also need to check for the existence of :path because CONNECT does not have a path, and we
	// don't support that currently.
	if (!headers->Path() || headers->Path()->value().c_str()[0] != '/') {
		HeaderMapImpl headers{{Headers::get().Status, std::to_string(enumToInt(Code::NotFound))}};
		response_encoder->encodeHeaders(headers, true);
		return;
	}

	// Create a new http request.
	request = new EHttpRequest(this, headers);
	if (end_stream) {
		session->getHttpAcceptor()->requestChannel.write(request);
	}
}

void ActiveStream::decodeData(Buffer::Instance& data, bool end_stream) {
//	printf("decodeData(), data.length=%llu\n", data.length());

	ES_ASSERT(request != null && request->getHttpStream() == this);

	Http::Buffer::OwnedImpl& oi = dynamic_cast<Http::Buffer::OwnedImpl&>(data);
	Http::Buffer::LinkedBuffer& lb = oi.buffer();

	for (auto buffer : lb) {
		request->bodyData.add(buffer);
	}

	if (end_stream) {
		session->getHttpAcceptor()->requestChannel.write(request);
	}
}

void ActiveStream::decodeTrailers(HeaderMapPtr&& trailers) {
	// ignore for client request.
	ES_ASSERT(0);
}

void ActiveStream::encode100ContinueHeaders(const HeaderMap& headers) {
	response_encoder->encode100ContinueHeaders(headers);
}

void ActiveStream::encodeHeaders(const HeaderMap& headers, bool end_stream) {
	response_encoder->encodeHeaders(headers, end_stream);
}

void ActiveStream::encodeData(Buffer::Instance& data, bool end_stream) {
	response_encoder->encodeData(data, end_stream);
}

void ActiveStream::encodeTrailers(const HeaderMap& trailers) {
	response_encoder->encodeTrailers(trailers);
}

//=============================================================================

EHttpSession::~EHttpSession() {
	//
}

EHttpSession::EHttpSession(EIoService* service, sp<ESocket>& socket) :
	ESocketSession(service, socket), isFirstRequest(true) {
	acceptor = dynamic_cast<EHttpAcceptor*>(service);
}

EHttpAcceptor* EHttpSession::getHttpAcceptor() {
	return acceptor;
}

void EHttpSession::init() {
	//
}

sp<EObject> EHttpSession::read() {
	sp<EIoBuffer> ioBuffer = dynamic_pointer_cast<EIoBuffer>(ESocketSession::read());
	if (ioBuffer != null) {
		if (isFirstRequest) {
			int magic_len = strlen(NGHTTP2_CLIENT_MAGIC);
			sp<EIoBuffer> buf;
			if (firstRequestBuffer == null) {
				if (ioBuffer->remaining() < magic_len) {
					firstRequestBuffer = new EByteBuffer(32);
					firstRequestBuffer->append(ioBuffer->current(), ioBuffer->remaining());
					buf = EIoBuffer::wrap(firstRequestBuffer->data(), firstRequestBuffer->size());
				} else {
					buf = ioBuffer;
				}
			} else {
				firstRequestBuffer->append(ioBuffer->current(), ioBuffer->remaining());
				buf = EIoBuffer::wrap(firstRequestBuffer->data(), firstRequestBuffer->size());
			}

			if (buf->remaining() >= magic_len) {
				sp<EHttpSession> session = dynamic_pointer_cast<EHttpSession>(shared_from_this());

				if (memcmp(buf->current(), NGHTTP2_CLIENT_MAGIC, magic_len) != 0) {
					codec_.reset(new Http::Http1::ServerConnectionImpl(session, *this, hs1));
				} else {
					codec_.reset(new Http::Http2::ServerConnectionImpl(session, *this, hs2));
				}
				codec_->dispatch(buf);
				isFirstRequest = false;
				firstRequestBuffer = null;
			}
		} else {
			codec_->dispatch(ioBuffer);
		}
	}
	return ioBuffer;
}

boolean EHttpSession::write(sp<EObject> message) {
	return ESocketSession::write(message);
}

void EHttpSession::close() {
	responseChannel.write(new EndHttpResponse());
	ESocketSession::close();
}

Http::StreamDecoder& EHttpSession::newStream(Http::StreamEncoder& response_encoder) {
	ActiveStream* stream = new ActiveStream(this, response_encoder);
	streams_.add(stream);
	return *stream;
}

void EHttpSession::onGoAway() {
	//
}
} /* namespace naf */
} /* namespace efc */
