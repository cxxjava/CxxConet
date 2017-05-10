/*
 * NHttpCodecFilter.hh
 *
 *  Created on: 2017-4-7
 *      Author: cxxjava@163.com
 */

#ifndef NHTTPCODECFILTER_HH_
#define NHTTPCODECFILTER_HH_

#include "NIoFilterAdapter.hh"
#include "NIoBuffer.hh"

namespace efc {
namespace naf {

class NHttpCodecFilter: public NIoFilterAdapter {
public:
	static const int MAX_HTTP_SIZE = 4096;

public:
	/**
	 * {@inheritDoc}
	 */
	virtual boolean sessionCreated(NIoFilter::NextFilter* nextFilter, NIoSession* session) THROWS(EException);

	/**
	 * {@inheritDoc}
	 */
	virtual void sessionClosed(NIoFilter::NextFilter* nextFilter, NIoSession* session) THROWS(EException);

	/**
	 * {@inheritDoc}
	 */
	virtual sp<EObject> messageReceived(NIoFilter::NextFilter* nextFilter, NIoSession* session, sp<EObject> message) THROWS(EException);

	/**
	 * {@inheritDoc}
	 */
	virtual sp<EObject> messageSend(NIoFilter::NextFilter* nextFilter, NIoSession* session, sp<EObject> message) THROWS(EException);

	/**
	 *
	 */
	virtual EStringBase toString();
};

} /* namespace naf */
} /* namespace efc */
#endif /* NHTTPCODECFILTER_HH_ */
