/*
 * NBlacklistFilter.hh
 *
 *  Created on: 2016-5-7
 *      Author: cxxjava@163.com
 */

#ifndef NBLACKLISTFILTER_HH_
#define NBLACKLISTFILTER_HH_

#include "NIoFilterAdapter.hh"
#include "NSubnet.hh"
#include "ELog.hh"

namespace efc {
namespace naf {

/**
 * A {@link IoFilter} which blocks connections from blacklisted remote
 * address.
 *
 */

class NBlacklistFilter: public NIoFilterAdapter {
public:
	/**
	 * Sets the addresses to be blacklisted.
	 *
	 * NOTE: this call will remove any previously blacklisted addresses.
	 *
	 * @param addresses an array of addresses to be blacklisted.
	 */
	NBlacklistFilter* setBlacklist(EA<EInetAddress*>* addresses);

	/**
	 * Sets the subnets to be blacklisted.
	 *
	 * NOTE: this call will remove any previously blacklisted subnets.
	 *
	 * @param subnets an array of subnets to be blacklisted.
	 */
	NBlacklistFilter* setSubnetBlacklist(EA<NSubnet*>* subnets);

	/**
	 * Sets the addresses to be blacklisted.
	 *
	 * NOTE: this call will remove any previously blacklisted addresses.
	 *
	 * @param addresses a collection of InetAddress objects representing the
	 *        addresses to be blacklisted.
	 * @throws IllegalArgumentException if the specified collections contains
	 *         non-{@link InetAddress} objects.
	 */
	NBlacklistFilter* setBlacklist(EIterable<EInetAddress*>* addresses);

	/**
	 * Sets the subnets to be blacklisted.
	 *
	 * NOTE: this call will remove any previously blacklisted subnets.
	 *
	 * @param subnets an array of subnets to be blacklisted.
	 */
	NBlacklistFilter* setSubnetBlacklist(EIterable<NSubnet*>* subnets);

	/**
	 * Blocks the specified endpoint.
	 *
	 * @param address The address to block
	 */
	NBlacklistFilter* block(EInetAddress* address);
	NBlacklistFilter* block(const char* hostname);

	/**
	 * Blocks the specified subnet.
	 *
	 * @param subnet The subnet to block
	 */
	NBlacklistFilter* block(NSubnet* subnet);

	/**
	 * Unblocks the specified endpoint.
	 *
	 * @param address The address to unblock
	 */
	NBlacklistFilter* unblock(EInetAddress* address);
	NBlacklistFilter* unblock(const char* hostname);

	/**
	 * Unblocks the specified subnet.
	 *
	 * @param subnet The subnet to unblock
	 */
	NBlacklistFilter* unblock(NSubnet* subnet);

	/**
	 * {@inheritDoc}
	 */
	virtual boolean sessionCreated(NIoFilter::NextFilter* nextFilter, NIoSession* session) THROWS(EException);

private:
	static sp<ELogger> LOGGER;// = LoggerFactory.getLogger(BlacklistFilter.class);

	/** The list of blocked addresses */
	ECopyOnWriteArrayList<NSubnet> blacklist;

	boolean isBlocked(NIoSession* session);
};

} /* namespace naf */
} /* namespace efc */
#endif /* NBLACKLISTFILTER_HH_ */
