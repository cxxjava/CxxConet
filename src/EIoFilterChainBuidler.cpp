/*
 * EIoFilterChainBuilder.cpp
 *
 *  Created on: 2017-3-16
 *      Author: cxxjava@163.com
 */

#include "../inc/EIoFilterChainBuilder.hh"

namespace efc {
namespace naf {

class EntryImpl : public EIoFilterChain::Entry {
public:
	EString name;
	EIoFilter* volatile filter;
	EIoFilterChainBuilder* owner;

	virtual ~EntryImpl() {
		//TODO...
	}

	EntryImpl(const char* name, EIoFilter* filter, EIoFilterChainBuilder* dicb) {
		if (name == null) {
			throw EIllegalArgumentException(__FILE__, __LINE__, "name");
		}
		if (filter == null) {
			throw EIllegalArgumentException(__FILE__, __LINE__, "filter");
		}

		this->name = name;
		this->filter = filter;
		this->owner = dicb;
	}

	const char* getName() {
		return name.c_str();
	}

	EIoFilter* getFilter() {
		return filter;
	}

	void setFilter(EIoFilter* filter) {
		this->filter = filter;
	}

	EIoFilter::NextFilter* getNextFilter() {
		throw EIllegalStateException(__FILE__, __LINE__);
	}

	virtual EStringBase toString() {
		return EStringBase::formatOf("(%s:%s)", name.c_str(), filter->toString().c_str());
	}

	void addAfter(const char* name, EIoFilter* filter) {
		owner->addAfter(getName(), name, filter);
	}

	void addBefore(const char* name, EIoFilter* filter) {
		owner->addBefore(getName(), name, filter);
	}

	void remove() {
		owner->remove(getName());
	}

//	void replace(EIoFilter* newFilter) {
//		owner->replace(getName(), newFilter);
//	}
};

EIoFilterChainBuilder::~EIoFilterChainBuilder() {

}

EIoFilterChainBuilder::EIoFilterChainBuilder() {

}

EIoFilterChain::Entry* EIoFilterChainBuilder::getEntry(
		const char* name) {
	sp<EConcurrentIterator<EIoFilterChain::Entry> > citer = entries.iterator();
	while (citer->hasNext()) {
		sp<EIoFilterChain::Entry> e = citer->next();
        EString n(e->getName());
		if (n.equals(name)) {
			return e.get();
		}
	}

	return null;
}

EIoFilterChain::Entry* EIoFilterChainBuilder::getEntry(
		EIoFilter* filter) {
	sp<EConcurrentIterator<EIoFilterChain::Entry> > citer = entries.iterator();
	while (citer->hasNext()) {
		sp<EIoFilterChain::Entry> e = citer->next();
		if (e->getFilter() == filter) {
			return e.get();
		}
	}

	return null;
}

EIoFilter* EIoFilterChainBuilder::get(const char* name) {
	sp<EIoFilterChain::Entry> e = getEntry(name);
	if (e == null) {
		return null;
	}

	return e->getFilter();
}

boolean EIoFilterChainBuilder::contains(const char* name) {
	return getEntry(name) != null;
}

boolean EIoFilterChainBuilder::contains(EIoFilter* filter) {
	return getEntry(filter) != null;
}

void EIoFilterChainBuilder::addFirst(const char* name, EIoFilter* filter) {
	register_(0, new EntryImpl(name, filter, this));
}

void EIoFilterChainBuilder::addLast(const char* name, EIoFilter* filter) {
	register_(entries.size(), new EntryImpl(name, filter, this));
}

void EIoFilterChainBuilder::addBefore(const char* baseName, const char* name,
		EIoFilter* filter) {
	checkBaseName(baseName);

	for (sp<EConcurrentListIterator<EIoFilterChain::Entry> > i = entries.listIterator(); i->hasNext();) {
		sp<EIoFilterChain::Entry> base = i->next();
		if (strcmp(base->getName(), baseName) == 0) {
			register_(i->previousIndex(), new EntryImpl(name, filter, this));
			break;
		}
	}
}

void EIoFilterChainBuilder::addAfter(const char* baseName, const char* name,
		EIoFilter* filter) {
	checkBaseName(baseName);

	for (sp<EConcurrentListIterator<EIoFilterChain::Entry> > i = entries.listIterator(); i->hasNext();) {
		sp<EIoFilterChain::Entry> base = i->next();
		if (strcmp(base->getName(), baseName) == 0) {
			register_(i->nextIndex(), new EntryImpl(name, filter, this));
			break;
		}
	}
}

EIoFilter* EIoFilterChainBuilder::remove(const char* name) {
	if (name == null) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "name");
	}

	for (sp<EConcurrentListIterator<EIoFilterChain::Entry> > i = entries.listIterator(); i->hasNext();) {
		sp<EIoFilterChain::Entry> e = i->next();
		if (strcmp(e->getName(), name) == 0) {
			entries.removeAt(i->previousIndex());
			return e->getFilter();
		}
	}

	EString msg("Unknown filter name: ");
	msg += name;
	throw EIllegalArgumentException(__FILE__, __LINE__, msg.c_str());
}

EIoFilter* EIoFilterChainBuilder::remove(EIoFilter* filter) {
	if (filter == null) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "filter");
	}

	for (sp<EConcurrentListIterator<EIoFilterChain::Entry> > i = entries.listIterator(); i->hasNext();) {
		sp<EIoFilterChain::Entry> e = i->next();
		if (e->getFilter() == filter) {
			entries.removeAt(i->previousIndex());
			return e->getFilter();
		}
	}

	EString msg("Filter not found: ");
	msg += filter->toString();
	throw EIllegalArgumentException(__FILE__, __LINE__, msg.c_str());
}

void EIoFilterChainBuilder::clear() {
	entries.clear();
}

void EIoFilterChainBuilder::buildFilterChain(EIoFilterChain* chain) {
	sp<EConcurrentIterator<EIoFilterChain::Entry> > citer = entries.iterator();
	while (citer->hasNext()) {
		sp<EIoFilterChain::Entry> e = citer->next();
		chain->addLast(e->getName(), e->getFilter());
	}
}

EStringBase EIoFilterChainBuilder::toString() {
	EStringBase buf("{ ");

	boolean empty = true;

	sp<EConcurrentIterator<EIoFilterChain::Entry> > citer = entries.iterator();
	while (citer->hasNext()) {
		sp<EIoFilterChain::Entry> e = citer->next();
		if (!empty) {
			buf.append(", ");
		} else {
			empty = false;
		}

		buf.append('(');
		buf.append(e->getName());
		buf.append(':');
		buf.append(e->getFilter()->toString());
		buf.append(')');
	}

	if (empty) {
		buf.append("empty");
	}

	buf.append(" }");

	return buf;
}

void EIoFilterChainBuilder::register_(int index, EIoFilterChain::Entry* e) {
	if (contains(e->getName())) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "Other filter is using the same name.");
	}

	entries.addAt(index, e);
}

void EIoFilterChainBuilder::checkBaseName(const char* baseName) {
	if (baseName == null) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "baseName");
	}

	if (!contains(baseName)) {
		EString msg("Unknown filter name: ");
		msg += baseName;
		throw EIllegalArgumentException(__FILE__, __LINE__, msg.c_str());
	}
}

} /* namespace naf */
} /* namespace efc */
