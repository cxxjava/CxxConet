/*
 * NIoFilterAdapter.hh
 *
 *  Created on: 2016-1-22
 *      Author: cxxjava@163.com
 */

#ifndef NIOFILTERADAPTER_HH_
#define NIOFILTERADAPTER_HH_

#include "NIoFilter.hh"
#include "Efc.hh"

namespace efc {
namespace naf {

/**
 * An adapter class for {@link IoFilter}.  You can extend
 * this class and selectively override required event filter methods only.  All
 * methods forwards events to the next filter by default.
 *
 */

class NIoFilterAdapter: virtual public NIoFilter {
public:
//	virtual ~NIoFilterAdapter();

	/**
	 * {@inheritDoc}
	 */
//	virtual void init() THROWS(EException);

	/**
	 * {@inheritDoc}
	 */
//	virtual void destroy() THROWS(EException);

	/**
	 * {@inheritDoc}
	 */
//	virtual void onPreAdd(NIoFilterChain* parent, const char* name, NIoFilter::NextFilter* nextFilter) THROWS(EException);

	/**
	 * {@inheritDoc}
	 */
//	virtual void onPostAdd(NIoFilterChain* parent, const char* name, NIoFilter::NextFilter* nextFilter) THROWS(EException);

	/**
	 * {@inheritDoc}
	 */
//	virtual void onPreRemove(NIoFilterChain* parent, const char* name, NIoFilter::NextFilter* nextFilter) THROWS(EException);

	/**
	 * {@inheritDoc}
	 */
//	virtual void onPostRemove(NIoFilterChain* parent, const char* name, NIoFilter::NextFilter* nextFilter) THROWS(EException);

	/**
	 * {@inheritDoc}
	 */
	virtual boolean sessionCreated(NIoFilter::NextFilter* nextFilter, NIoSession* session) THROWS(EException);
//
//	/**
//	 * {@inheritDoc}
//	 */
//	virtual void sessionOpened(NIoFilter::NextFilter* nextFilter, NIoSession* session) THROWS(EException);

	/**
	 * {@inheritDoc}
	 */
	virtual void sessionClosed(NIoFilter::NextFilter* nextFilter, NIoSession* session) THROWS(EException);

//	/**
//	 * {@inheritDoc}
//	 */
//	virtual void sessionIdle(NIoFilter::NextFilter* nextFilter, NIoSession* session, EIdleStatus status) THROWS(EException);
//
//	/**
//	 * {@inheritDoc}
//	 */
//	virtual void exceptionCaught(NIoFilter::NextFilter* nextFilter, NIoSession* session, EThrowable* cause) THROWS(EException);
//
//	/**
//	 * {@inheritDoc}
//	 */
//	virtual void inputClosed(NIoFilter::NextFilter* nextFilter, NIoSession* session) THROWS(EException);

	/**
	 * {@inheritDoc}
	 */
	virtual sp<EObject> messageReceived(NIoFilter::NextFilter* nextFilter, NIoSession* session, sp<EObject> message) THROWS(EException);

	/**
	 * {@inheritDoc}
	 */
	virtual sp<EObject> messageSend(NIoFilter::NextFilter* nextFilter, NIoSession* session, sp<EObject> message) THROWS(EException);

//	/**
//	 * {@inheritDoc}
//	 */
//	virtual void filterClose(NIoFilter::NextFilter* nextFilter, NIoSession* session) THROWS(EException);
//
//	/**
//	 * {@inheritDoc}
//	 */
//	virtual void filterWrite(NIoFilter::NextFilter* nextFilter, NIoSession* session, EObject* writeRequest) THROWS(EException);

	/**
	 *
	 */
	virtual EStringBase toString();
};

} /* namespace naf */
} /* namespace efc */
#endif /* NIOFILTERADAPTER_HH_ */
