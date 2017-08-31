/*
 * EBlacklistFilter.cpp
 *
 *  Created on: 2016-5-7
 *      Author: cxxjava@163.com
 */

#include "../inc/EBlacklistFilter.hh"

namespace efc {
namespace naf {

sp<ELogger> EBlacklistFilter::LOGGER = ELoggerManager::getLogger("EBlacklistFilter");

EBlacklistFilter* EBlacklistFilter::setBlacklist(EA<EInetAddress*>* addresses) {
	if (addresses == null) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "addresses");
	}

	blacklist.clear();

	for (int i = 0; i < addresses->length(); i++) {
		EInetAddress* addr = (*addresses)[i];
		block(addr);
	}

	return this;
}

EBlacklistFilter* EBlacklistFilter::setSubnetBlacklist(EA<ESubnet*>* subnets) {
	if (subnets == null) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "Subnets must not be null");
	}

	blacklist.clear();

	for (int i = 0; i < subnets->length(); i++) {
		ESubnet* subnet = (*subnets)[i];
		block(subnet);
	}

	return this;
}

EBlacklistFilter* EBlacklistFilter::setBlacklist(EIterable<EInetAddress*>* addresses) {
	if (addresses == null) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "addresses");
	}

	blacklist.clear();

	sp<EIterator<EInetAddress*> > iter = addresses->iterator();
	while (iter->hasNext()) {
		block(iter->next());
	}

	return this;
}

EBlacklistFilter* EBlacklistFilter::setSubnetBlacklist(EIterable<ESubnet*>* subnets) {
	if (subnets == null) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "Subnets must not be null");
	}

	blacklist.clear();

	sp<EIterator<ESubnet*> > iter = subnets->iterator();
	while (iter->hasNext()) {
		block(iter->next());
	}

	return this;
}

EBlacklistFilter* EBlacklistFilter::block(EInetAddress* address) {
	if (address == null) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "Adress to block can not be null");
	}

	blacklist.add(new ESubnet(address, 32));

	return this;
}

EBlacklistFilter* EBlacklistFilter::block(const char* hostname) {
	EInetAddress address = EInetAddress::getByName(hostname);
	return this->block(&address);
}

EBlacklistFilter* EBlacklistFilter::block(ESubnet* subnet) {
	if (subnet == null) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "Subnet can not be null");
	}

	blacklist.add(new ESubnet(*subnet));

	return this;
}

EBlacklistFilter* EBlacklistFilter::unblock(EInetAddress* address) {
	if (address == null) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "Adress to unblock can not be null");
	}

	ESubnet subnet(address, 32);
	blacklist.remove(&subnet);

	return this;
}

EBlacklistFilter* EBlacklistFilter::unblock(const char* hostname) {
	EInetAddress address = EInetAddress::getByName(hostname);
	return this->unblock(&address);
}

EBlacklistFilter* EBlacklistFilter::unblock(ESubnet* subnet) {
	if (subnet == null) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "Subnet can not be null");
	}

	blacklist.remove(subnet);

	return this;
}

boolean EBlacklistFilter::sessionCreated(EIoFilter::NextFilter* nextFilter, EIoSession* session) {
	if (!isBlocked(session)) {
		// forward if not blocked
		return nextFilter->sessionCreated(session);
	} else {
		EString msg = EString::formatOf(
				"Remote address: %s not in the blacklist; closing.",
				session->getRemoteAddress()->toString().c_str());
		LOGGER->warn(__FILE__, __LINE__, msg.c_str());
		return false;
	}
}

boolean EBlacklistFilter::isBlocked(EIoSession* session) {
	EInetSocketAddress* remoteAddress = session->getRemoteAddress();

	//if (remoteAddress instanceof InetSocketAddress)
	{
		EInetAddress* address = remoteAddress->getAddress();

		// check all subnets
		sp<EConcurrentIterator<ESubnet> > iter = blacklist.iterator();
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
