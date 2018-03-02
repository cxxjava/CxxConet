/*
 * EIoFilterAdapter.cpp
 *
 *  Created on: 2016-1-22
 *      Author: cxxjava@163.com
 */

#include "../inc/EIoFilterAdapter.hh"
#include "../inc/EIoFilterChain.hh"

namespace efc {
namespace naf {

boolean EIoFilterAdapter::sessionCreated(EIoFilter::NextFilter* nextFilter,
		EIoSession* session) {
	return nextFilter->sessionCreated(session);
}

void EIoFilterAdapter::sessionClosed(EIoFilter::NextFilter* nextFilter,
		EIoSession* session) {
	nextFilter->sessionClosed(session);
}

sp<EObject> EIoFilterAdapter::messageReceived(EIoFilter::NextFilter* nextFilter,
		EIoSession* session, sp<EObject> message) {
	return nextFilter->messageReceived(session, message);
}

sp<EObject> EIoFilterAdapter::messageSend(EIoFilter::NextFilter* nextFilter,
		EIoSession* session, sp<EObject> message) {
	return nextFilter->messageSend(session, message);
}

EString EIoFilterAdapter::toString() {
	return "EIoFilterAdapter";
}

} /* namespace naf */
} /* namespace efc */
