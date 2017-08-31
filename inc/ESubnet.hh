/*
 * ESubnet.hh
 *
 *  Created on: 2016-5-7
 *      Author: cxxjava@163.com
 */

#ifndef ESUBNET_HH_
#define ESUBNET_HH_

#include "EInetAddress.hh"

namespace efc {
namespace naf {

/**
 * A IP subnet using the CIDR notation. Currently, only IP version 4
 * address are supported.
 *
 */

class ESubnet: public EObject {
public:
	virtual ~ESubnet();

	/**
	 * Creates a subnet from CIDR notation. For example, the subnet
	 * 192.168.0.0/24 would be created using the {@link InetAddress}
	 * 192.168.0.0 and the mask 24.
	 * @param subnet The {@link InetAddress} of the subnet
	 * @param mask The mask
	 */
	ESubnet(EInetAddress* subnet, int mask);

	/**
	 * Checks if the {@link InetAddress} is within this subnet
	 * @param address The {@link InetAddress} to check
	 * @return True if the address is within this subnet, false otherwise
	 */
	boolean inSubnet(EInetAddress* address);

	/**
	 * {@inheritDoc}
	 */
	virtual EStringBase toString();

	/**
	 * {@inheritDoc}
	 */
	virtual boolean equals(ESubnet* obj);

private:
	static const int IP_MASK_V4 = 0x80000000;

	static const llong IP_MASK_V6 = 0x8000000000000000L;

	static const int BYTE_MASK = 0xFF;

	EInetAddress subnet;

	/** An int representation of a subnet for IPV4 addresses */
	int subnetInt;

	/** An long representation of a subnet for IPV6 addresses */
	long subnetLong;

	long subnetMask;

	int suffix;

	/**
	 * Converts an IP address into an integer
	 */
	int toInt(EInetAddress* inetAddress);

	/**
	 * Converts an IP address into a long
	 */
	long toLong(EInetAddress* inetAddress);

	/**
	 * Converts an IP address to a subnet using the provided mask
	 *
	 * @param address
	 *            The address to convert into a subnet
	 * @return The subnet as an integer
	 */
	long toSubnet(EInetAddress* address);
};

} /* namespace naf */
} /* namespace efc */
#endif /* ESUBNET_HH_ */
