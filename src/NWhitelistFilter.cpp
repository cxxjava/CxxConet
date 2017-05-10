/*
 * NWhitelistFilter.cpp
 *
 *  Created on: 2016-5-7
 *      Author: cxxjava@163.com
 */

#include "NWhitelistFilter.hh"

namespace efc {
namespace naf {

sp<ELogger> NWhitelistFilter::LOGGER = ELoggerManager::getLogger("NWhitelistFilter");

NWhitelistFilter* NWhitelistFilter::setWhitelist(EA<EInetAddress*>* addresses) {
	if (addresses == null) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "addresses");
	}

	whitelist.clear();

	for (int i = 0; i < addresses->length(); i++) {
		EInetAddress* addr = (*addresses)[i];
		allow(addr);
	}

	return this;
}

NWhitelistFilter* NWhitelistFilter::setSubnetWhitelist(EA<NSubnet*>* subnets) {
	if (subnets == null) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "Subnets must not be null");
	}

	whitelist.clear();

	for (int i = 0; i < subnets->length(); i++) {
		NSubnet* subnet = (*subnets)[i];
		allow(subnet);
	}

	return this;
}

NWhitelistFilter* NWhitelistFilter::setWhitelist(EIterable<EInetAddress*>* addresses) {
	if (addresses == null) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "addresses");
	}

	whitelist.clear();

	sp<EIterator<EInetAddress*> > iter = addresses->iterator();
	while (iter->hasNext()) {
		allow(iter->next());
	}

	return this;
}

NWhitelistFilter* NWhitelistFilter::setSubnetWhitelist(EIterable<NSubnet*>* subnets) {
	if (subnets == null) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "Subnets must not be null");
	}

	whitelist.clear();

	sp<EIterator<NSubnet*> > iter = subnets->iterator();
	while (iter->hasNext()) {
		allow(iter->next());
	}

	return this;
}

NWhitelistFilter* NWhitelistFilter::allow(EInetAddress* address) {
	if (address == null) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "Adress to allow can not be null");
	}

	whitelist.add(new NSubnet(address, 32));

	return this;
}

NWhitelistFilter* NWhitelistFilter::allow(const char* hostname) {
	EInetAddress address = EInetAddress::getByName(hostname);
	return this->allow(&address);
}

NWhitelistFilter* NWhitelistFilter::allow(NSubnet* subnet) {
	if (subnet == null) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "Subnet can not be null");
	}

	whitelist.add(new NSubnet(*subnet));

	return this;
}

NWhitelistFilter* NWhitelistFilter::disallow(EInetAddress* address) {
	if (address == null) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "Adress to disallow can not be null");
	}

	NSubnet subnet(address, 32);
	whitelist.remove(&subnet);

	return this;
}

NWhitelistFilter* NWhitelistFilter::disallow(const char* hostname) {
	EInetAddress address = EInetAddress::getByName(hostname);
	return this->disallow(&address);
}

NWhitelistFilter* NWhitelistFilter::disallow(NSubnet* subnet) {
	if (subnet == null) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "Subnet can not be null");
	}

	whitelist.remove(subnet);

	return this;
}

boolean NWhitelistFilter::sessionCreated(NIoFilter::NextFilter* nextFilter,
		NIoSession* session) {
	if (isAllowed(session)) {
		// forward if allowed
		return nextFilter->sessionCreated(session);
	} else {
		LOGGER->warn(__FILE__, __LINE__, "Remote address not in the whitelist; closing.");
		return false;
	}
}

boolean NWhitelistFilter::isAllowed(NIoSession* session) {
	EInetSocketAddress* remoteAddress = session->getRemoteAddress();

	//if (remoteAddress instanceof InetSocketAddress)
	{
		EInetAddress* address = remoteAddress->getAddress();

		// check all subnets
		sp<EConcurrentIterator<NSubnet> > iter = whitelist.iterator();
		while (iter->hasNext()) {
			sp<NSubnet> subnet = iter->next();
			if (subnet->inSubnet(address)) {
				return true;
			}
		}
	}

	return false;
}

} /* namespace naf */
} /* namespace efc */
