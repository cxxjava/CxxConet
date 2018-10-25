/*
 * EHttpRequest.hh
 *
 *  Created on: 2017-4-21
 *      Author: cxxjava@163.com
 */

#ifndef EHTTPREQUEST_HH_
#define EHTTPREQUEST_HH_

#include "Efc.hh"

namespace efc {
namespace naf {
namespace filter {
namespace http {

class EHttpRequest: public EObject {
public:
	EHttpRequest(void* httpData, int headLen, int bodyLen) :
		m_HeadLen(headLen),
		m_BodyLen(bodyLen) {
		m_HttpData.append(httpData, headLen+bodyLen);
	}
	char* getHttpData() {
		return (char*)m_HttpData.data();
	}
	int getHttpDataLen() {
		return m_HttpData.size();
	}
	char* getHeadData() {
		return (char*)m_HttpData.data();
	}
	int getHeadLen() {
		return m_HeadLen;
	}
	char* getBodyData() {
		return (char*)m_HttpData.data() + m_HeadLen;
	}
	int getBodyLen() {
		return m_BodyLen;
	}
private:
	EByteBuffer m_HttpData;
	int m_HeadLen;
	int m_BodyLen;
};

} /* namespace http */
} /* namespace filter */
} /* namespace naf */
} /* namespace efc */
#endif /* EHTTPREQUEST_HH_ */
