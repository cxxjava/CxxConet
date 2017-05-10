/*
 * NHttpResponse.hh
 *
 *  Created on: 2017-4-21
 *      Author: cxxjava@163.com
 */

#ifndef NHTTPRESPONSE_HH_
#define NHTTPRESPONSE_HH_

#include "Efc.hh"

namespace efc {
namespace naf {

#define TEST_HTTP_DATA "HTTP/1.1 200 OK\r\nContent-Length: 3\r\n\r\nOK!"

class NHttpResponse: public EObject {
public:
	const char* getHttpData() {
		return TEST_HTTP_DATA;
	}
	int getHttpDataLen() {
		return strlen(TEST_HTTP_DATA);
	}
};

} /* namespace naf */
} /* namespace efc */
#endif /* NHTTPRESPONSE_HH_ */
