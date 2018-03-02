/*
 * EHttpCodecFilter.cpp
 *
 *  Created on: 2017-4-7
 *      Author: cxxjava@163.com
 */

#include "./EHttpCodecFilter.hh"
#include "./EHttpRequest.hh"
#include "./EHttpResponse.hh"

namespace efc {
namespace naf {

static const char* SESSION_CACHE_DATA = "session_cache_data";

boolean EHttpCodecFilter::sessionCreated(EIoFilter::NextFilter* nextFilter,
		EIoSession* session) {
	return nextFilter->sessionCreated(session);
}

void EHttpCodecFilter::sessionClosed(EIoFilter::NextFilter* nextFilter,
		EIoSession* session) {
	nextFilter->sessionClosed(session);
}

sp<EObject> EHttpCodecFilter::messageReceived(EIoFilter::NextFilter* nextFilter,
		EIoSession* session, sp<EObject> message) {

	sp<EIoBuffer> buf = dynamic_pointer_cast<EIoBuffer>(message);

	llong key = (llong)SESSION_CACHE_DATA;
	sp<EByteBuffer> cache = dynamic_pointer_cast<EByteBuffer>(session->attributes.get(key));

	if (buf == null && (cache == null || cache->size() == 0)) {
		return nextFilter->messageReceived(session, null);
	}

	if (cache == null) {
		cache = new EByteBuffer(buf->limit(), 64);
		session->attributes.put(key, cache);
	}

	if (buf != null) {
		if (cache->size() + buf->limit() > MAX_HTTP_SIZE) {
			throw EOUTOFMEMORYERROR;
		} else {
			cache->append(buf->current(), buf->limit());
		}
	}

	sp<EHttpRequest> out;

	int hlen, blen, httplen;
	httplen = hlen = blen = 0;

	char* prnrn = eso_strnstr((char*)cache->data(), cache->size(), "\r\n\r\n");
	if (prnrn > 0) {
		hlen = prnrn + 4 - (char*)cache->data();

		char* plen = eso_strncasestr((char*)cache->data(), hlen, "Content-Length:");
		if (plen) {
			plen += 15;
			char* plen2 = eso_strstr(plen, "\r\n");
			EString lenstr(plen, 0, plen2-plen);
			blen = EInteger::parseInt(lenstr.trim().c_str());

			httplen = hlen + blen;
		} else {
			httplen = hlen;
		}

		if (cache->size() >= httplen) {
			out = new EHttpRequest(cache->data(), hlen, blen);
			cache->erase(0, httplen);
		}
	}

	return nextFilter->messageReceived(session, out);
}

sp<EObject> EHttpCodecFilter::messageSend(EIoFilter::NextFilter* nextFilter,
		EIoSession* session, sp<EObject> message) {
	sp<EHttpResponse> response = dynamic_pointer_cast<EHttpResponse>(message);
	if (response != null) {
		sp<EIoBuffer> out = EIoBuffer::allocate(response->getHttpDataLen());
		out->put(response->getHttpData(), response->getHttpDataLen());
		out->flip();
		message = out;
	}
	return nextFilter->messageSend(session, message);
}

EString EHttpCodecFilter::toString() {
	return "EHttpCodecFilter";
}

} /* namespace naf */
} /* namespace efc */
