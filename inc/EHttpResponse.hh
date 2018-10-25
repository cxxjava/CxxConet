/*
 * EHttpResponse.hh
 *
 *  Created on: 2018-7-4
 *      Author: cxxjava@163.com
 */

#ifndef EHTTPRESPONSE_HH_
#define EHTTPRESPONSE_HH_

#include "../inc/EIoBuffer.hh"
#include "../http/include/header_map.h"
#include "../http/source/header_map_impl.h"

namespace efc {
namespace naf {

class ActiveStream;
class EHttpSession;

class EHttpResponse: public efc::EObject {
public:
	EHttpResponse(ActiveStream* stream);

	ActiveStream* getHttpStream();

	Http::HeaderMap& getHeaderMap();

	void write(const void* data, int len);

private:
	friend class EHttpAcceptor;

	ActiveStream* stream;
	Http::HeaderMapImpl headerMap;
	sp<EIoBuffer> bodyData;
};

class EndHttpResponse : public EHttpResponse {
public:
	EndHttpResponse(): EHttpResponse(null) {}
};

} /* namespace naf */
} /* namespace efc */
#endif /* EHTTPRESPONSE_HH_ */
