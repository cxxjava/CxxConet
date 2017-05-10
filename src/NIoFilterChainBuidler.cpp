/*
 * NIoFilterChainBuilder.cpp
 *
 *  Created on: 2017-3-16
 *      Author: cxxjava@163.com
 */

#include "NIoFilterChainBuilder.hh"

namespace efc {
namespace naf {

class EntryImpl : public NIoFilterChain::Entry {
public:
	EString name;
	NIoFilter* volatile filter;
	NIoFilterChainBuilder* owner;

	virtual ~EntryImpl() {
		//TODO...
	}

	EntryImpl(const char* name, NIoFilter* filter, NIoFilterChainBuilder* dicb) {
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

	NIoFilter* getFilter() {
		return filter;
	}

	void setFilter(NIoFilter* filter) {
		this->filter = filter;
	}

	NIoFilter::NextFilter* getNextFilter() {
		throw EIllegalStateException(__FILE__, __LINE__);
	}

	virtual EStringBase toString() {
		return EStringBase::formatOf("(%s:%s)", name.c_str(), filter->toString().c_str());
	}

	void addAfter(const char* name, NIoFilter* filter) {
		owner->addAfter(getName(), name, filter);
	}

	void addBefore(const char* name, NIoFilter* filter) {
		owner->addBefore(getName(), name, filter);
	}

	void remove() {
		owner->remove(getName());
	}

//	void replace(NIoFilter* newFilter) {
//		owner->replace(getName(), newFilter);
//	}
};

NIoFilterChainBuilder::~NIoFilterChainBuilder() {

}

NIoFilterChainBuilder::NIoFilterChainBuilder() {

}

NIoFilterChain::Entry* NIoFilterChainBuilder::getEntry(
		const char* name) {
	sp<EConcurrentIterator<NIoFilterChain::Entry> > citer = entries.iterator();
	while (citer->hasNext()) {
		sp<NIoFilterChain::Entry> e = citer->next();
        EString n(e->getName());
		if (n.equals(name)) {
			return e.get();
		}
	}

	return null;
}

NIoFilterChain::Entry* NIoFilterChainBuilder::getEntry(
		NIoFilter* filter) {
	sp<EConcurrentIterator<NIoFilterChain::Entry> > citer = entries.iterator();
	while (citer->hasNext()) {
		sp<NIoFilterChain::Entry> e = citer->next();
		if (e->getFilter() == filter) {
			return e.get();
		}
	}

	return null;
}

NIoFilter* NIoFilterChainBuilder::get(const char* name) {
	sp<NIoFilterChain::Entry> e = getEntry(name);
	if (e == null) {
		return null;
	}

	return e->getFilter();
}

boolean NIoFilterChainBuilder::contains(const char* name) {
	return getEntry(name) != null;
}

boolean NIoFilterChainBuilder::contains(NIoFilter* filter) {
	return getEntry(filter) != null;
}

void NIoFilterChainBuilder::addFirst(const char* name, NIoFilter* filter) {
	register_(0, new EntryImpl(name, filter, this));
}

void NIoFilterChainBuilder::addLast(const char* name, NIoFilter* filter) {
	register_(entries.size(), new EntryImpl(name, filter, this));
}

void NIoFilterChainBuilder::addBefore(const char* baseName, const char* name,
		NIoFilter* filter) {
	checkBaseName(baseName);

	for (sp<EConcurrentListIterator<NIoFilterChain::Entry> > i = entries.listIterator(); i->hasNext();) {
		sp<NIoFilterChain::Entry> base = i->next();
		if (strcmp(base->getName(), baseName) == 0) {
			register_(i->previousIndex(), new EntryImpl(name, filter, this));
			break;
		}
	}
}

void NIoFilterChainBuilder::addAfter(const char* baseName, const char* name,
		NIoFilter* filter) {
	checkBaseName(baseName);

	for (sp<EConcurrentListIterator<NIoFilterChain::Entry> > i = entries.listIterator(); i->hasNext();) {
		sp<NIoFilterChain::Entry> base = i->next();
		if (strcmp(base->getName(), baseName) == 0) {
			register_(i->nextIndex(), new EntryImpl(name, filter, this));
			break;
		}
	}
}

NIoFilter* NIoFilterChainBuilder::remove(const char* name) {
	if (name == null) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "name");
	}

	for (sp<EConcurrentListIterator<NIoFilterChain::Entry> > i = entries.listIterator(); i->hasNext();) {
		sp<NIoFilterChain::Entry> e = i->next();
		if (strcmp(e->getName(), name) == 0) {
			entries.removeAt(i->previousIndex());
			return e->getFilter();
		}
	}

	EString msg("Unknown filter name: ");
	msg += name;
	throw EIllegalArgumentException(__FILE__, __LINE__, msg.c_str());
}

NIoFilter* NIoFilterChainBuilder::remove(NIoFilter* filter) {
	if (filter == null) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "filter");
	}

	for (sp<EConcurrentListIterator<NIoFilterChain::Entry> > i = entries.listIterator(); i->hasNext();) {
		sp<NIoFilterChain::Entry> e = i->next();
		if (e->getFilter() == filter) {
			entries.removeAt(i->previousIndex());
			return e->getFilter();
		}
	}

	EString msg("Filter not found: ");
	msg += filter->toString();
	throw EIllegalArgumentException(__FILE__, __LINE__, msg.c_str());
}

void NIoFilterChainBuilder::clear() {
	entries.clear();
}

void NIoFilterChainBuilder::buildFilterChain(NIoFilterChain* chain) {
	sp<EConcurrentIterator<NIoFilterChain::Entry> > citer = entries.iterator();
	while (citer->hasNext()) {
		sp<NIoFilterChain::Entry> e = citer->next();
		chain->addLast(e->getName(), e->getFilter());
	}
}

EStringBase NIoFilterChainBuilder::toString() {
	EStringBase buf("{ ");

	boolean empty = true;

	sp<EConcurrentIterator<NIoFilterChain::Entry> > citer = entries.iterator();
	while (citer->hasNext()) {
		sp<NIoFilterChain::Entry> e = citer->next();
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

void NIoFilterChainBuilder::register_(int index, NIoFilterChain::Entry* e) {
	if (contains(e->getName())) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "Other filter is using the same name.");
	}

	entries.addAt(index, e);
}

void NIoFilterChainBuilder::checkBaseName(const char* baseName) {
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
