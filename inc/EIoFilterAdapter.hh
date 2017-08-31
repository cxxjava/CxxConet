/*
 * EIoFilterAdapter.hh
 *
 *  Created on: 2016-1-22
 *      Author: cxxjava@163.com
 */

#ifndef EIOFILTERADAPTER_HH_
#define EIOFILTERADAPTER_HH_

#include "./EIoFilter.hh"

namespace efc {
namespace naf {

/**
 * An adapter class for {@link IoFilter}.  You can extend
 * this class and selectively override required event filter methods only.  All
 * methods forwards events to the next filter by default.
 *
 */

class EIoFilterAdapter: virtual public EIoFilter {
public:
	/**
	 * {@inheritDoc}
	 */
	virtual boolean sessionCreated(EIoFilter::NextFilter* nextFilter, EIoSession* session) THROWS(EException);

	/**
	 * {@inheritDoc}
	 */
	virtual void sessionClosed(EIoFilter::NextFilter* nextFilter, EIoSession* session) THROWS(EException);

	/**
	 * {@inheritDoc}
	 */
	virtual sp<EObject> messageReceived(EIoFilter::NextFilter* nextFilter, EIoSession* session, sp<EObject> message) THROWS(EException);

	/**
	 * {@inheritDoc}
	 */
	virtual sp<EObject> messageSend(EIoFilter::NextFilter* nextFilter, EIoSession* session, sp<EObject> message) THROWS(EException);

	/**
	 *
	 */
	virtual EStringBase toString();
};

} /* namespace naf */
} /* namespace efc */
#endif /* EIOFILTERADAPTER_HH_ */
