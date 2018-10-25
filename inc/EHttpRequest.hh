/*
 * EHttpRequest.hh
 *
 *  Created on: 2018-7-4
 *      Author: cxxjava@163.com
 */

#ifndef EHTTPREQUEST_HH_
#define EHTTPREQUEST_HH_

#include "../http/include/buffer.h"
#include "../http/include/header_map.h"
#include "./EIoBuffer.hh"

namespace efc {
namespace naf {

class ActiveStream;
class HttpInputStream;

class EHttpRequest: public efc::EObject {
public:
	EHttpRequest(ActiveStream* stream, Http::HeaderMapPtr headerMap);

	ActiveStream* getHttpStream();

	Http::HeaderMapPtr getHeaderMap() { return headerMap; }
	Http::Buffer::LinkedBuffer getBodyData() { return bodyData; }

private:
	friend class ActiveStream;

	ActiveStream* stream;
	Http::HeaderMapPtr headerMap;
	Http::Buffer::LinkedBuffer bodyData;
};

} /* namespace naf */
} /* namespace efc */
#endif /* EHTTPREQUEST_HH_ */
