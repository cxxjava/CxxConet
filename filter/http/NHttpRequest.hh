/*
 * NHttpRequest.hh
 *
 *  Created on: 2017-4-21
 *      Author: cxxjava@163.com
 */

#ifndef NHTTPREQUEST_HH_
#define NHTTPREQUEST_HH_

#include "Efc.hh"

namespace efc {
namespace naf {

class NHttpRequest: public EObject {
public:
	es_data_t* m_HttpData;
	int m_HeadLen;
	int m_BodyLen;

public:
	~NHttpRequest() {
		eso_mfree(m_HttpData);
	}

	NHttpRequest(const char* httpData, int headLen, int bodyLen) :
		m_HttpData(NULL),
		m_HeadLen(headLen),
		m_BodyLen(bodyLen) {
		eso_mmemcpy(&m_HttpData, 0, httpData, headLen+bodyLen);
	}
};

} /* namespace naf */
} /* namespace efc */
#endif /* NHTTPREQUEST_HH_ */
