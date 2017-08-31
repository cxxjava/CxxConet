/*
 * EHttpResponse.hh
 *
 *  Created on: 2017-4-21
 *      Author: cxxjava@163.com
 */

#ifndef EHTTPRESPONSE_HH_
#define EHTTPRESPONSE_HH_

#include "Efc.hh"

namespace efc {
namespace naf {

class EHttpResponse: public EObject {
public:
	EHttpResponse(void* body, int size, const char* head=NULL) {
		m_BodyLen = size;
		if (!head) {
			m_HttpData.appendFormat("HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n", size);
		} else {
			m_HttpData.append(head);
		}
		m_HeadLen = m_HttpData.size();
		m_HttpData.append(body, size);
	}
	EHttpResponse(EString body, const char* head=NULL) {
		m_BodyLen = body.length();
		if (!head) {
			m_HttpData.appendFormat("HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n", m_BodyLen);
		} else {
			m_HttpData.append(head);
		}
		m_HeadLen = m_HttpData.size();
		m_HttpData.append(body);
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

} /* namespace naf */
} /* namespace efc */
#endif /* EHTTPRESPONSE_HH_ */
