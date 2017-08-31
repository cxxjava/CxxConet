/*
 * EIoFilter.hh
 *
 *  Created on: 2017-3-16
 *      Author: cxxjava@163.com
 */

#ifndef EIOFILTER_HH_
#define EIOFILTER_HH_

#include "./EIoSession.hh"

namespace efc {
namespace naf {

interface EIoFilterChain;

interface EIoFilter: virtual public EObject {
	/**
	 * Represents the next {@link IoFilter} in {@link IoFilterChain}.
	 */
	interface NextFilter {
		virtual ~NextFilter() {
		}

		/**
		 * Forwards <tt>sessionCreated</tt> event to next filter.
		 *
		 * @param session The {@link IoSession} which has to process this invocation
		 */
		virtual boolean sessionCreated(EIoSession* session) = 0;

		/**
		 * Forwards <tt>sessionClosed</tt> event to next filter.
		 *
		 * @param session The {@link IoSession} which has to process this invocation
		 */
		virtual void sessionClosed(EIoSession* session) = 0;

		/**
		 * Forwards <tt>messageReceived</tt> event to next filter.
		 *
		 * @param session The {@link IoSession} which has to process this invocation
		 * @param message The received message
		 */
		virtual sp<EObject> messageReceived(EIoSession* session, sp<EObject> message) = 0;

		/**
		 * Forwards <tt>messageSend</tt> event to next filter.
		 *
		 * @param session The {@link IoSession} which has to process this invocation
		 * @param data The message data to write
		 * @param size The message data size
		 */
		virtual sp<EObject> messageSend(EIoSession* session, sp<EObject> message) = 0;
	};

	virtual ~EIoFilter() {
	}

	/**
	 * Filters {@link IoHandler#sessionCreated(IoSession)} event.
	 *
	 * @param nextFilter
	 *            the {@link NextFilter} for this filter. You can reuse this
	 *            object until this filter is removed from the chain.
	 * @param session The {@link IoSession} which has received this event
	 * @throws Exception If an error occurred while processing the event
	 */
	virtual boolean sessionCreated(NextFilter* nextFilter, EIoSession* session)
			THROWS(EException) = 0;

	/**
	 * Filters {@link IoHandler#sessionClosed(IoSession)} event.
	 *
	 * @param nextFilter
	 *            the {@link NextFilter} for this filter. You can reuse this
	 *            object until this filter is removed from the chain.
	 * @param session The {@link IoSession} which has received this event
	 * @throws Exception If an error occurred while processing the event
	 */
	virtual void sessionClosed(NextFilter* nextFilter, EIoSession* session)
			THROWS(EException) = 0;

	/**
	 * Filters {@link IoHandler#messageReceived(IoSession,Object)} event.
	 *
	 * @param nextFilter
	 *            the {@link NextFilter} for this filter. You can reuse this
	 *            object until this filter is removed from the chain.
	 * @param session The {@link IoSession} which has received this event
	 * @param message The received message
	 * @throws Exception If an error occurred while processing the event
	 */
	virtual sp<EObject> messageReceived(NextFilter* nextFilter, EIoSession* session,
			sp<EObject> message) THROWS(EException) = 0;

	/**
	 * Filters {@link IoHandler#messageSend(IoSession,Object)} event.
	 *
	 * @param nextFilter
	 *            the {@link NextFilter} for this filter. You can reuse this
	 *            object until this filter is removed from the chain.
	 * @param session The {@link IoSession} which has received this event
	 * @param writeRequest The {@link WriteRequest} that contains the sent message
	 * @throws Exception If an error occurred while processing the event
	 */
	virtual sp<EObject> messageSend(NextFilter* nextFilter, EIoSession* session,
			sp<EObject> message) THROWS(EException) = 0;
};

} /* namespace naf */
} /* namespace efc */
#endif /* EIOFILTER_HH_ */
