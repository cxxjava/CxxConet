/*
 * EIoService.hh
 *
 *  Created on: 2017-3-19
 *      Author: cxxjava@163.com
 */

#ifndef EIOSERVICE_HH_
#define EIOSERVICE_HH_

#include "Efc.hh"
#include "Eco.hh"
#include "ELog.hh"

#include "./EIoServiceStatistics.hh"
#include "./EIoFilterChainBuilder.hh"

namespace efc {
namespace naf {

/**
 * Base interface for all {@link IoAcceptor}s and {@link IoConnector}s
 * that provide I/O service and manage {@link IoSession}s.
 *
 */

interface EIoService : virtual public EObject {
	virtual ~EIoService() {
	}

	/**
	 * Returns the number of all threads which are currently working by this
	 * service.
	 */
	virtual int getWorkThreads() = 0;

	/**
	 * Returns the number of all sessions which are currently managed by this
	 * service.
	 */
	virtual int getManagedSessionCount() = 0;

	/**
	 * Returns the {@link IoFilterChainBuilder} which will build the
	 * {@link IoFilterChain} of all {@link IoSession}s which is created
	 * by this service.
	 * The default value is an empty {@link DefaultIoFilterChainBuilder}.
	 */
	virtual EIoFilterChainBuilder* getFilterChainBuilder() = 0;

	/**
	 * Returns the IoServiceStatistics object for this service.
	 *
	 * @return The statistics object for this service.
	 */
	virtual EIoServiceStatistics* getStatistics() = 0;

	/**
	 * Releases any resources allocated by this service.  Please note that
	 * this method might block as long as there are any sessions managed by
	 * this service.
	 */
	virtual void dispose() = 0;

	/**
	 * Returns <tt>true</tt> if and if only all resources of this processor
	 * have been disposed.
	 */
	virtual boolean isDisposed() = 0;
};

} // namespace naf
} // namespace efc

#endif /* EIOSERVICE_HH_ */
