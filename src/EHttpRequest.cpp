/*
 * EHttpRequest.cpp
 *
 *  Created on: 2018-7-4
 *      Author: cxxjava@163.com
 */

#include "../inc/EHttpRequest.hh"
#include "../inc/EHttpSession.hh"

namespace efc {
namespace naf {

EHttpRequest::EHttpRequest(ActiveStream* as, Http::HeaderMapPtr hm) : stream(as), headerMap(hm) {
	//
}

ActiveStream* EHttpRequest::getHttpStream() {
	return stream;
}

} /* namespace naf */
} /* namespace efc */
