/*
 * EHttpResponse.cpp
 *
 *  Created on: 2018-7-4
 *      Author: cxxjava@163.com
 */

#include "../inc/EHttpResponse.hh"

#include "../http/include/codes.h"
#include "../http/source/headers.h"
#include "../http/source/buffer_impl.h"

namespace efc {
namespace naf {

EHttpResponse::EHttpResponse(ActiveStream* as) :
		stream(as), headerMap{{Http::Headers::get().Status, std::to_string((int)Http::Code::OK)}} {
	//
}

ActiveStream* EHttpResponse::getHttpStream() {
	return stream;
}

Http::HeaderMap& EHttpResponse::getHeaderMap() {
	return headerMap;
}

void EHttpResponse::write(const void* data, int len) {
	if (bodyData == null) {
		bodyData = EIoBuffer::allocate(256);
	}
	bodyData->put(data, len);
}

} /* namespace naf */
} /* namespace efc */
