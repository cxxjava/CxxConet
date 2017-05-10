/*
 * NIoService.hh
 *
 *  Created on: 2017-3-19
 *      Author: cxxjava@163.com
 */

#ifndef NIOSERVICE_HH_
#define NIOSERVICE_HH_

#include "Efc.hh"
#include "Eco.hh"
#include "ELog.hh"

#include "NIoServiceStatistics.hh"
#include "NIoFilterChainBuilder.hh"

namespace efc {
namespace naf {

/**
 * Base interface for all {@link IoAcceptor}s and {@link IoConnector}s
 * that provide I/O service and manage {@link IoSession}s.
 *
 */

interface NIoService : virtual public EObject {
	virtual ~NIoService() {
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
	virtual NIoFilterChainBuilder* getFilterChainBuilder() = 0;

	/**
	 * Returns the IoServiceStatistics object for this service.
	 *
	 * @return The statistics object for this service.
	 */
	virtual NIoServiceStatistics* getStatistics() = 0;

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

#endif /* NIOSERVICE_HH_ */
