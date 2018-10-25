/*
 * EHttpHandler.hh
 *
 *  Created on: 2018-7-4
 *      Author: cxxjava@163.com
 */

#ifndef EHTTPHANDLER_HH_
#define EHTTPHANDLER_HH_

#include "./EHttpSession.hh"
#include "./EHttpRequest.hh"
#include "./EHttpResponse.hh"

namespace efc {
namespace naf {

abstract class EHttpHandler: public efc::EObject {
public:
	virtual void sessionOpened(sp<EHttpSession>& session) THROWS(EException);
	virtual void sessionClosed(sp<EHttpSession>& session) THROWS(EException);

	virtual void service(sp<EHttpSession>& session, sp<EHttpRequest>& request, sp<EHttpResponse>& response) THROWS(EException);

	virtual void doGet(sp<EHttpSession>& session, sp<EHttpRequest>& request, sp<EHttpResponse>& response) THROWS(EException);
	virtual void doHead(sp<EHttpSession>& session, sp<EHttpRequest>& request, sp<EHttpResponse>& response) THROWS(EException);
	virtual void doPost(sp<EHttpSession>& session, sp<EHttpRequest>& request, sp<EHttpResponse>& response) THROWS(EException);
	virtual void doPut(sp<EHttpSession>& session, sp<EHttpRequest>& request, sp<EHttpResponse>& response) THROWS(EException);
	virtual void doDelete(sp<EHttpSession>& session, sp<EHttpRequest>& request, sp<EHttpResponse>& response) THROWS(EException);
	virtual void doOptions(sp<EHttpSession>& session, sp<EHttpRequest>& request, sp<EHttpResponse>& response) THROWS(EException);
	virtual void doTrace(sp<EHttpSession>& session, sp<EHttpRequest>& request, sp<EHttpResponse>& response) THROWS(EException);
	virtual void doPatch(sp<EHttpSession>& session, sp<EHttpRequest>& request, sp<EHttpResponse>& response) THROWS(EException);
};

} /* namespace naf */
} /* namespace efc */
#endif /* EHTTPHANDLER_HH_ */
