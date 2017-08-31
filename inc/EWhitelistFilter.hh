/*
 * EWhitelistFilter.hh
 *
 *  Created on: 2016-5-7
 *      Author: cxxjava@163.com
 */

#ifndef EWHITELISTFILTER_HH_
#define EWHITELISTFILTER_HH_

#include "ELog.hh"

#include "./EIoFilterAdapter.hh"
#include "./ESubnet.hh"

namespace efc {
namespace naf {

/**
 * A {@link IoFilter} which allows connections from whitelisted remote
 * address.
 *
 * @see: BlacklistFilter.java
 */

class EWhitelistFilter: public EIoFilterAdapter {
public:
	/**
	 * Sets the addresses to be whitelisted.
	 *
	 * NOTE: this call will remove any previously whitelisted addresses.
	 *
	 * @param addresses an array of addresses to be whitelisted.
	 */
	EWhitelistFilter* setWhitelist(EA<EInetAddress*>* addresses);

	/**
	 * Sets the subnets to be whitelisted.
	 *
	 * NOTE: this call will remove any previously whitelisted subnets.
	 *
	 * @param subnets an array of subnets to be whitelisted.
	 */
	EWhitelistFilter* setSubnetWhitelist(EA<ESubnet*>* subnets);

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
	EWhitelistFilter* setWhitelist(EIterable<EInetAddress*>* addresses);

	/**
	 * Sets the subnets to be whitelisted.
	 *
	 * NOTE: this call will remove any previously whitelisted subnets.
	 *
	 * @param subnets an array of subnets to be whitelisted.
	 */
	EWhitelistFilter* setSubnetWhitelist(EIterable<ESubnet*>* subnets);

	/**
	 * Allows the specified endpoint.
	 *
	 * @param address The address to allow
	 */
	EWhitelistFilter* allow(EInetAddress* address);
	EWhitelistFilter* allow(const char* hostname);

	/**
	 * Allows the specified subnet.
	 *
	 * @param subnet The subnet to allow
	 */
	EWhitelistFilter* allow(ESubnet* subnet);

	/**
	 * Disallows the specified endpoint.
	 *
	 * @param address The address to disallow
	 */
	EWhitelistFilter* disallow(EInetAddress* address);
	EWhitelistFilter* disallow(const char* hostname);

	/**
	 * Disallows the specified subnet.
	 *
	 * @param subnet The subnet to disallow
	 */
	EWhitelistFilter* disallow(ESubnet* subnet);

	/**
	 * {@inheritDoc}
	 */
	virtual boolean sessionCreated(EIoFilter::NextFilter* nextFilter, EIoSession* session) THROWS(EException);

private:
	static sp<ELogger> LOGGER;// = LoggerFactory.getLogger(WhitelistFilter.class);

	/** The list of allowed addresses */
	ECopyOnWriteArrayList<ESubnet> whitelist;

	boolean isAllowed(EIoSession* session);
};

} /* namespace naf */
} /* namespace efc */
#endif /* EWHITELISTFILTER_HH_ */
