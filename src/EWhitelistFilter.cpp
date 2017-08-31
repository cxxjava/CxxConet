/*
 * EWhitelistFilter.cpp
 *
 *  Created on: 2016-5-7
 *      Author: cxxjava@163.com
 */

#include "../inc/EWhitelistFilter.hh"

namespace efc {
namespace naf {

sp<ELogger> EWhitelistFilter::LOGGER = ELoggerManager::getLogger("EWhitelistFilter");

EWhitelistFilter* EWhitelistFilter::setWhitelist(EA<EInetAddress*>* addresses) {
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

EWhitelistFilter* EWhitelistFilter::setSubnetWhitelist(EA<ESubnet*>* subnets) {
	if (subnets == null) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "Subnets must not be null");
	}

	whitelist.clear();

	for (int i = 0; i < subnets->length(); i++) {
		ESubnet* subnet = (*subnets)[i];
		allow(subnet);
	}

	return this;
}

EWhitelistFilter* EWhitelistFilter::setWhitelist(EIterable<EInetAddress*>* addresses) {
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

EWhitelistFilter* EWhitelistFilter::setSubnetWhitelist(EIterable<ESubnet*>* subnets) {
	if (subnets == null) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "Subnets must not be null");
	}

	whitelist.clear();

	sp<EIterator<ESubnet*> > iter = subnets->iterator();
	while (iter->hasNext()) {
		allow(iter->next());
	}

	return this;
}

EWhitelistFilter* EWhitelistFilter::allow(EInetAddress* address) {
	if (address == null) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "Adress to allow can not be null");
	}

	whitelist.add(new ESubnet(address, 32));

	return this;
}

EWhitelistFilter* EWhitelistFilter::allow(const char* hostname) {
	EInetAddress address = EInetAddress::getByName(hostname);
	return this->allow(&address);
}

EWhitelistFilter* EWhitelistFilter::allow(ESubnet* subnet) {
	if (subnet == null) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "Subnet can not be null");
	}

	whitelist.add(new ESubnet(*subnet));

	return this;
}

EWhitelistFilter* EWhitelistFilter::disallow(EInetAddress* address) {
	if (address == null) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "Adress to disallow can not be null");
	}

	ESubnet subnet(address, 32);
	whitelist.remove(&subnet);

	return this;
}

EWhitelistFilter* EWhitelistFilter::disallow(const char* hostname) {
	EInetAddress address = EInetAddress::getByName(hostname);
	return this->disallow(&address);
}

EWhitelistFilter* EWhitelistFilter::disallow(ESubnet* subnet) {
	if (subnet == null) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "Subnet can not be null");
	}

	whitelist.remove(subnet);

	return this;
}

boolean EWhitelistFilter::sessionCreated(EIoFilter::NextFilter* nextFilter,
		EIoSession* session) {
	if (isAllowed(session)) {
		// forward if allowed
		return nextFilter->sessionCreated(session);
	} else {
		EString msg = EString::formatOf(
				"Remote address: %s not in the whitelist; closing.",
				session->getRemoteAddress()->toString().c_str());
		LOGGER->warn(__FILE__, __LINE__, msg.c_str());
		return false;
	}
}

boolean EWhitelistFilter::isAllowed(EIoSession* session) {
	EInetSocketAddress* remoteAddress = session->getRemoteAddress();

	//if (remoteAddress instanceof InetSocketAddress)
	{
		EInetAddress* address = remoteAddress->getAddress();

		// check all subnets
		sp<EConcurrentIterator<ESubnet> > iter = whitelist.iterator();
		while (iter->hasNext()) {
			sp<ESubnet> subnet = iter->next();
			if (subnet->inSubnet(address)) {
				return true;
			}
		}
	}

	return false;
}

} /* namespace naf */
} /* namespace efc */
