/*
 * NIoFilterAdapter.cpp
 *
 *  Created on: 2016-1-22
 *      Author: cxxjava@163.com
 */

#include "NIoFilterAdapter.hh"
#include "NIoFilterChain.hh"

namespace efc {
namespace naf {

boolean NIoFilterAdapter::sessionCreated(NIoFilter::NextFilter* nextFilter,
		NIoSession* session) {
	return nextFilter->sessionCreated(session);
}

void NIoFilterAdapter::sessionClosed(NIoFilter::NextFilter* nextFilter,
		NIoSession* session) {
	nextFilter->sessionClosed(session);
}

sp<EObject> NIoFilterAdapter::messageReceived(NIoFilter::NextFilter* nextFilter,
		NIoSession* session, sp<EObject> message) {
	return nextFilter->messageReceived(session, message);
}

sp<EObject> NIoFilterAdapter::messageSend(NIoFilter::NextFilter* nextFilter,
		NIoSession* session, sp<EObject> message) {
	return nextFilter->messageSend(session, message);
}

EStringBase NIoFilterAdapter::toString() {
	return "NIoFilterAdapter";
}

} /* namespace naf */
} /* namespace efc */
