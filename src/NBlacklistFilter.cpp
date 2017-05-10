/*
 * NBlacklistFilter.cpp
 *
 *  Created on: 2016-5-7
 *      Author: cxxjava@163.com
 */

#include "NBlacklistFilter.hh"

namespace efc {
namespace naf {

sp<ELogger> NBlacklistFilter::LOGGER = ELoggerManager::getLogger("NBlacklistFilter");

NBlacklistFilter* NBlacklistFilter::setBlacklist(EA<EInetAddress*>* addresses) {
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

NBlacklistFilter* NBlacklistFilter::setSubnetBlacklist(EA<NSubnet*>* subnets) {
	if (subnets == null) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "Subnets must not be null");
	}

	blacklist.clear();

	for (int i = 0; i < subnets->length(); i++) {
		NSubnet* subnet = (*subnets)[i];
		block(subnet);
	}

	return this;
}

NBlacklistFilter* NBlacklistFilter::setBlacklist(EIterable<EInetAddress*>* addresses) {
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

NBlacklistFilter* NBlacklistFilter::setSubnetBlacklist(EIterable<NSubnet*>* subnets) {
	if (subnets == null) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "Subnets must not be null");
	}

	blacklist.clear();

	sp<EIterator<NSubnet*> > iter = subnets->iterator();
	while (iter->hasNext()) {
		block(iter->next());
	}

	return this;
}

NBlacklistFilter* NBlacklistFilter::block(EInetAddress* address) {
	if (address == null) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "Adress to block can not be null");
	}

	blacklist.add(new NSubnet(address, 32));

	return this;
}

NBlacklistFilter* NBlacklistFilter::block(const char* hostname) {
	EInetAddress address = EInetAddress::getByName(hostname);
	return this->block(&address);
}

NBlacklistFilter* NBlacklistFilter::block(NSubnet* subnet) {
	if (subnet == null) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "Subnet can not be null");
	}

	blacklist.add(new NSubnet(*subnet));

	return this;
}

NBlacklistFilter* NBlacklistFilter::unblock(EInetAddress* address) {
	if (address == null) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "Adress to unblock can not be null");
	}

	NSubnet subnet(address, 32);
	blacklist.remove(&subnet);

	return this;
}

NBlacklistFilter* NBlacklistFilter::unblock(const char* hostname) {
	EInetAddress address = EInetAddress::getByName(hostname);
	return this->unblock(&address);
}

NBlacklistFilter* NBlacklistFilter::unblock(NSubnet* subnet) {
	if (subnet == null) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "Subnet can not be null");
	}

	blacklist.remove(subnet);

	return this;
}

boolean NBlacklistFilter::sessionCreated(NIoFilter::NextFilter* nextFilter, NIoSession* session) {
	if (!isBlocked(session)) {
		// forward if not blocked
		return nextFilter->sessionCreated(session);
	} else {
		LOGGER->warn(__FILE__, __LINE__, "Remote address in the blacklist; closing.");
		return false;
	}
}

boolean NBlacklistFilter::isBlocked(NIoSession* session) {
	EInetSocketAddress* remoteAddress = session->getRemoteAddress();

	//if (remoteAddress instanceof InetSocketAddress)
	{
		EInetAddress* address = remoteAddress->getAddress();

		// check all subnets
		sp<EConcurrentIterator<NSubnet> > iter = blacklist.iterator();
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
