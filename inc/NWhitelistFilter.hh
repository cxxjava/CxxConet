/*
 * NWhitelistFilter.hh
 *
 *  Created on: 2016-5-7
 *      Author: cxxjava@163.com
 */

#ifndef NWHITELISTFILTER_HH_
#define NWHITELISTFILTER_HH_

#include "NIoFilterAdapter.hh"
#include "NSubnet.hh"
#include "ELog.hh"

namespace efc {
namespace naf {

/**
 * A {@link IoFilter} which allows connections from whitelisted remote
 * address.
 *
 * @see: BlacklistFilter.java
 */

class NWhitelistFilter: public NIoFilterAdapter {
public:
	/**
	 * Sets the addresses to be whitelisted.
	 *
	 * NOTE: this call will remove any previously whitelisted addresses.
	 *
	 * @param addresses an array of addresses to be whitelisted.
	 */
	NWhitelistFilter* setWhitelist(EA<EInetAddress*>* addresses);

	/**
	 * Sets the subnets to be whitelisted.
	 *
	 * NOTE: this call will remove any previously whitelisted subnets.
	 *
	 * @param subnets an array of subnets to be whitelisted.
	 */
	NWhitelistFilter* setSubnetWhitelist(EA<NSubnet*>* subnets);

	/**
	 * Sets the addresses to be whitelisted.
	 *
	 * NOTE: this call will remove any previously whitelisted addresses.
	 *
	 * @param addresses a collection of InetAddress objects representing the
	 *        addresses to be whitelisted.
	 * @throws IllegalArgumentException if the specified collections contains
	 *         non-{@link InetAddress} objects.
	 */
	NWhitelistFilter* setWhitelist(EIterable<EInetAddress*>* addresses);

	/**
	 * Sets the subnets to be whitelisted.
	 *
	 * NOTE: this call will remove any previously whitelisted subnets.
	 *
	 * @param subnets an array of subnets to be whitelisted.
	 */
	NWhitelistFilter* setSubnetWhitelist(EIterable<NSubnet*>* subnets);

	/**
	 * Allows the specified endpoint.
	 *
	 * @param address The address to allow
	 */
	NWhitelistFilter* allow(EInetAddress* address);
	NWhitelistFilter* allow(const char* hostname);

	/**
	 * Allows the specified subnet.
	 *
	 * @param subnet The subnet to allow
	 */
	NWhitelistFilter* allow(NSubnet* subnet);

	/**
	 * Disallows the specified endpoint.
	 *
	 * @param address The address to disallow
	 */
	NWhitelistFilter* disallow(EInetAddress* address);
	NWhitelistFilter* disallow(const char* hostname);

	/**
	 * Disallows the specified subnet.
	 *
	 * @param subnet The subnet to disallow
	 */
	NWhitelistFilter* disallow(NSubnet* subnet);

	/**
	 * {@inheritDoc}
	 */
	virtual boolean sessionCreated(NIoFilter::NextFilter* nextFilter, NIoSession* session) THROWS(EException);

private:
	static sp<ELogger> LOGGER;// = LoggerFactory.getLogger(WhitelistFilter.class);

	/** The list of allowed addresses */
	ECopyOnWriteArrayList<NSubnet> whitelist;

	boolean isAllowed(NIoSession* session);
};

} /* namespace naf */
} /* namespace efc */
#endif /* NWHITELISTFILTER_HH_ */
