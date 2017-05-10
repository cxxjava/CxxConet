/*
 * NSubnet.cpp
 *
 *  Created on: 2016-5-7
 *      Author: cxxjava@163.com
 */

#include "NSubnet.hh"

namespace efc {
namespace naf {

// Warning: Now only support IPV4 address.

NSubnet::~NSubnet() {

}

NSubnet::NSubnet(EInetAddress* subnet, int mask) {
	/* @see:
	if (subnet == null) {
		throw new IllegalArgumentException("Subnet address can not be null");
	}

	if (!(subnet instanceof Inet4Address) && !(subnet instanceof Inet6Address)) {
		throw new IllegalArgumentException("Only IPv4 and IPV6 supported");
	}

	if (subnet instanceof Inet4Address) {
		// IPV4 address
		if ((mask < 0) || (mask > 32)) {
			throw new IllegalArgumentException("Mask has to be an integer between 0 and 32 for an IPV4 address");
		} else {
			this.subnet = subnet;
			subnetInt = toInt(subnet);
			this.suffix = mask;

			// binary mask for this subnet
			this.subnetMask = IP_MASK_V4 >> (mask - 1);
		}
	} else {
		// IPV6 address
		if ((mask < 0) || (mask > 128)) {
			throw new IllegalArgumentException("Mask has to be an integer between 0 and 128 for an IPV6 address");
		} else {
			this.subnet = subnet;
			subnetLong = toLong(subnet);
			this.suffix = mask;

			// binary mask for this subnet
			this.subnetMask = IP_MASK_V6 >> (mask - 1);
		}
	}
	*/

	if (subnet == null) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "Subnet address can not be null");
	}

	// IPV4 address
	if ((mask < 0) || (mask > 32)) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "Mask has to be an integer between 0 and 32 for an IPV4 address");
	} else {
		this->subnet = *subnet;
		subnetInt = toInt(subnet);
		this->suffix = mask;

		// binary mask for this subnet
		this->subnetMask = IP_MASK_V4 >> (mask - 1);
	}
}

boolean NSubnet::inSubnet(EInetAddress* address) {
	/* @see:
	if (address.isAnyLocalAddress()) {
		return true;
	}

	if (address instanceof Inet4Address) {
		return (int) toSubnet(address) == subnetInt;
	} else {
		return toSubnet(address) == subnetLong;
	}
	*/

	if (!address) return false;

	if (address->isAnyLocalAddress()) {
		return true;
	}

	return (int) toSubnet(address) == subnetInt;
}

EStringBase NSubnet::toString() {
	return subnet.getHostAddress() + "/" + suffix;
}

boolean NSubnet::equals(NSubnet* other) {
	if (!other) return false;

	return other->subnetInt == subnetInt && other->suffix == suffix;
}

int NSubnet::toInt(EInetAddress* inetAddress) {
	/* @see:
	byte[] address = inetAddress.getAddress();
	int result = 0;

	for (int i = 0; i < address.length; i++) {
		result <<= 8;
		result |= address[i] & BYTE_MASK;
	}

	return result;
	*/
	return inetAddress->getAddress();
}

long NSubnet::toLong(EInetAddress* inetAddress) {
	return inetAddress->getAddress();
}

long NSubnet::toSubnet(EInetAddress* address) {
	/* @see:
	if (address instanceof Inet4Address) {
		return toInt(address) & (int) subnetMask;
	} else {
		return toLong(address) & subnetMask;
	}
	*/

	return address->getAddress() & (int) subnetMask;
}

} /* namespace naf */
} /* namespace efc */
