/*
 * EHttpSession.hh
 *
 *  Created on: 2018-6-21
 *      Author: cxxjava@163.com
 */

#ifndef EHTTPSESSION_HH_
#define EHTTPSESSION_HH_

#include "./ESocketSession.hh"
#include "./EHttpRequest.hh"
#include "./EHttpResponse.hh"

#include "../http/source/http1/codec_impl.h"
#include "../http/source/http2/codec_impl.h"

namespace efc {
namespace naf {

using namespace Http;

class ActiveStream: public EObject,
		public Http::StreamDecoder,
		public Http::StreamEncoder {
public:
	ActiveStream(EHttpSession* session, Http::StreamEncoder& encoder): session(session), response_encoder(&encoder) {}
	virtual ~ActiveStream() {}

	sp<EHttpSession> getHttpSession();

	// Http::StreamDecoder
	virtual void decode100ContinueHeaders(HeaderMapPtr&& headers);
	virtual void decodeHeaders(HeaderMapPtr&& headers, bool end_stream);
	virtual void decodeData(Buffer::Instance& data, bool end_stream);
	virtual void decodeTrailers(HeaderMapPtr&& trailers);

	// Http::StreamEncoder
	virtual void encode100ContinueHeaders(const HeaderMap& headers);
	virtual void encodeHeaders(const HeaderMap& headers, bool end_stream);
	virtual void encodeData(Buffer::Instance& data, bool end_stream);
	virtual void encodeTrailers(const HeaderMap& trailers);

private:
	EHttpSession* session;
	Http::StreamEncoder* response_encoder;

	sp<EHttpRequest> request;
	sp<EHttpResponse> response;
};

//=============================================================================


/**
 *
 */

class EHttpAcceptor;

class EHttpSession: public ESocketSession, public Http::ServerConnectionCallbacks {
public:
	virtual ~EHttpSession();

	/**
	 *
	 * Creates a new instance of EHttpSession.
	 *
	 * @param service the associated IoService
	 * @param socket the associated socket
	 */
	EHttpSession(EIoService* service, sp<ESocket>& socket);

	EHttpAcceptor* getHttpAcceptor();

	virtual void init();
	virtual sp<EObject> read();
	virtual boolean write(sp<EObject> message);
	virtual void close();

protected:
	friend class EHttpAcceptor;

	EFiberChannel<EHttpResponse> responseChannel;

protected:
	virtual Http::StreamDecoder& newStream(Http::StreamEncoder& response_encoder);

	virtual void onGoAway();

	EHttpAcceptor* acceptor;

	ELinkedList<ActiveStream*> streams_;


	boolean isFirstRequest;
	sp<EByteBuffer> firstRequestBuffer;

	Http::ServerConnectionPtr codec_;

	Http::Http1Settings hs1;
	Http::Http2Settings hs2;
};

} /* namespace naf */
} /* namespace efc */
#endif /* EHTTPSESSION_HH_ */
