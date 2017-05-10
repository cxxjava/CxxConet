/*
 * NHttpCodecFilter.cpp
 *
 *  Created on: 2017-4-7
 *      Author: cxxjava@163.com
 */

#include "NHttpCodecFilter.hh"
#include "NHttpRequest.hh"
#include "NHttpResponse.hh"

namespace efc {
namespace naf {

static const char* SESSION_CACHE_DATA = "session_cache_data";

boolean NHttpCodecFilter::sessionCreated(NIoFilter::NextFilter* nextFilter,
		NIoSession* session) {
	return nextFilter->sessionCreated(session);
}

void NHttpCodecFilter::sessionClosed(NIoFilter::NextFilter* nextFilter,
		NIoSession* session) {
	nextFilter->sessionClosed(session);
}

sp<EObject> NHttpCodecFilter::messageReceived(NIoFilter::NextFilter* nextFilter,
		NIoSession* session, sp<EObject> message) {

	sp<NIoBuffer> buf = dynamic_pointer_cast<NIoBuffer>(message);

	llong key = (llong)SESSION_CACHE_DATA;
	sp<EByteBuffer> cache = dynamic_pointer_cast<EByteBuffer>(session->attributes.get(key));

	if (buf == null && cache == null) {
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

	sp<NHttpRequest> out;

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
			out = new NHttpRequest((const char*)cache->data(), hlen, blen);
			cache->erase(0, httplen);
		}
	}

	return nextFilter->messageReceived(session, out);
}

sp<EObject> NHttpCodecFilter::messageSend(NIoFilter::NextFilter* nextFilter,
		NIoSession* session, sp<EObject> message) {
	sp<NHttpResponse> response = dynamic_pointer_cast<NHttpResponse>(message);
	if (response != null) {
		sp<NIoBuffer> out = NIoBuffer::allocate(response->getHttpDataLen());
		out->put(response->getHttpData(), response->getHttpDataLen());
		out->flip();
		message = out;
	}
	return nextFilter->messageSend(session, message);
}

EStringBase NHttpCodecFilter::toString() {
	return "NHttpCodecFilter";
}

} /* namespace naf */
} /* namespace efc */
