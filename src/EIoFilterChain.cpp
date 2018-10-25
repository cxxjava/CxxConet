/*
 * EIoFilterChain.cpp
 *
 *  Created on: 2017-3-16
 *      Author: cxxjava@163.com
 */

#include "../inc/EIoFilterChain.hh"
#include "../inc/EIoFilterAdapter.hh"
#include "../inc/EIoBuffer.hh"

namespace efc {
namespace naf {

class EIoFilterChain::EntryImpl: public EIoFilterChain::Entry {
public:
	~EntryImpl() {
		delete nextFilter;
	}

	EntryImpl(EntryImpl* prevEntry, EntryImpl* nextEntry, const char* name,
			EIoFilter* filter, EIoFilterChain* difc) {
		if (filter == null) {
			throw EIllegalArgumentException(__FILE__, __LINE__, "filter");
		}

		if (name == null) {
			throw EIllegalArgumentException(__FILE__, __LINE__, "name");
		}

		this->prevEntry = prevEntry;
		this->nextEntry = nextEntry;
		this->name = name;
		this->filter = filter;
		this->owner = difc;

		class _NextFilter: public EIoFilter::NextFilter {
		private:
			EntryImpl* ei;
			EIoFilterChain* ifc;
		public:
			_NextFilter(EntryImpl* e, EIoFilterChain* f): ei(e), ifc(f) {
			}

			virtual ~_NextFilter() {
			}

			virtual boolean sessionCreated(EIoSession* session) {
				Entry* nextEntry = ei->nextEntry;
				return ifc->callNextSessionCreated(nextEntry, session);
			}

			virtual void sessionClosed(EIoSession* session) {
				Entry* nextEntry = ei->nextEntry;
				ifc->callNextSessionClosed(nextEntry, session);
			}

			virtual sp<EObject> messageReceived(EIoSession* session, sp<EObject> message) {
				Entry* nextEntry = ei->nextEntry;
				return ifc->callNextMessageReceived(nextEntry, session, message);
			}

			sp<EObject> messageSend(EIoSession* session, sp<EObject> message) {
				Entry* nextEntry = ei->nextEntry;
				return ifc->callNextMessageSend(nextEntry, session, message);
			}

			virtual EString toString() {
				return ei->nextEntry->name;
			}
		};

		this->nextFilter = new _NextFilter(this, difc);
	}

	/**
	 * @return the name of the filter.
	 */
	const char* getName() {
		return name.c_str();
	}

	/**
	 * @return the filter.
	 */
	EIoFilter* getFilter() {
		return filter;
	}

	/**
	 * @return The {@link NextFilter} of the filter.
	 */
	EIoFilter::NextFilter* getNextFilter() {
		return nextFilter;
	}

	/**
	 * Adds the specified filter with the specified name just before this entry.
	 *
	 * @param name The Filter's name
	 * @param filter The added Filter
	 */
	void addBefore(const char* name, EIoFilter* filter) {
		owner->addBefore(getName(), name, filter);
	}

	/**
	 * Adds the specified filter with the specified name just after this entry.
	 *
	 * @param name The Filter's name
	 * @param filter The added Filter
	 */
	void addAfter(const char* name, EIoFilter* filter) {
		owner->addAfter(getName(), name, filter);
	}

	/**
	 * Removes this entry from the chain it belongs to.
	 */
	void remove() {
		owner->remove(getName());
	}

	virtual EString toString() {
		EString sb;

		// Add the current filter
		sb.append("('").append(getName()).append('\'');

		// Add the previous filter
		sb.append(", prev: '");

		if (prevEntry != null) {
			sb.append(prevEntry->name);
			sb.append(':');
			sb.append(typeid(prevEntry->getFilter()).name());
		} else {
			sb.append("null");
		}

		// Add the next filter
		sb.append("', next: '");

		if (nextEntry != null) {
			sb.append(nextEntry->name);
			sb.append(':');
			sb.append(typeid(nextEntry->getFilter()).name());
		} else {
			sb.append("null");
		}

		sb.append("')");

		return sb;
	}

public:
	EntryImpl* prevEntry;
	EntryImpl* nextEntry;
	EString name;
	EIoFilter* filter;
	EIoFilter::NextFilter* nextFilter;
	EIoFilterChain* owner;
};

EIoFilterChain::~EIoFilterChain() {
	delete head->getFilter(); //!
	delete head;
	delete tail->getFilter(); //!
	delete tail;
	delete name2entry;
}

EIoFilterChain::EIoFilterChain(EIoSession* session) {
	if (session == null) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "session");
	}

	this->session = session;
	head = new EntryImpl(null, null, "head", new EIoFilterAdapter(), this);
	tail = new EntryImpl(head, null, "tail", new EIoFilterAdapter(), this);
	head->nextEntry = tail;

	name2entry = new EHashMap<EString*, EntryImpl*>();
}

EIoSession* EIoFilterChain::getSession() {
	return session;
}

EIoFilterChain::Entry* EIoFilterChain::getEntry(
		const char* name) {
	EString ns(name);
	return name2entry->get(&ns);
}

EIoFilterChain::Entry* EIoFilterChain::getEntry(
		EIoFilter* filter) {
	EntryImpl* e = head->nextEntry;

	while (e != tail) {
		if (e->getFilter() == filter) {
			return e;
		}

		e = e->nextEntry;
	}

	return null;
}

EIoFilter* EIoFilterChain::get(const char* name) {
	EIoFilterChain::Entry* e = getEntry(name);

	if (e == null) {
		return null;
	}

	return e->getFilter();
}

boolean EIoFilterChain::contains(const char* name) {
	return getEntry(name) != null;
}

boolean EIoFilterChain::contains(EIoFilter* filter) {
	return getEntry(filter) != null;
}

EIoFilter::NextFilter* EIoFilterChain::getNextFilter(const char* name) {
	EIoFilterChain::Entry* e = getEntry(name);

	if (e == null) {
		return null;
	}

	return e->getNextFilter();
}

EIoFilter::NextFilter* EIoFilterChain::getNextFilter(EIoFilter* filter) {
	EIoFilterChain::Entry* e = getEntry(filter);
	if (e == null) {
		return null;
	}
	return e->getNextFilter();
}

void EIoFilterChain::addFirst(const char* name, EIoFilter* filter) {
	checkAddable(name);
	register_(head, name, filter);
}

void EIoFilterChain::addLast(const char* name, EIoFilter* filter) {
	checkAddable(name);
	register_(tail->prevEntry, name, filter);
}

void EIoFilterChain::addBefore(const char* baseName, const char* name,
		EIoFilter* filter) {
	EntryImpl* baseEntry = checkOldName(baseName);
	checkAddable(name);
	register_(baseEntry->prevEntry, name, filter);
}

void EIoFilterChain::addAfter(const char* baseName, const char* name,
		EIoFilter* filter) {
	EntryImpl* baseEntry = checkOldName(baseName);
	checkAddable(name);
	register_(baseEntry, name, filter);
}

EIoFilter* EIoFilterChain::remove(const char* name) {
	EntryImpl* entry = checkOldName(name);
	deregister(entry);
	EIoFilter* filter = entry->getFilter();
	delete entry; //!
	return filter;
}

EIoFilter* EIoFilterChain::remove(EIoFilter* filter) {
	EntryImpl* e = head->nextEntry;

	while (e != tail) {
		if (e->getFilter() == filter) {
			deregister(e);
			delete e; //!

			return filter;
		}

		e = e->nextEntry;
	}

	EString msg("Filter not found: ");
	msg += filter->toString();
	throw EIllegalArgumentException(__FILE__, __LINE__, msg.c_str());
}

void EIoFilterChain::clear() {
	sp<EIterator<EMapEntry<EString*, EntryImpl*>*> > iter = name2entry->entrySet()->iterator();
	while (iter->hasNext()) {
		EMapEntry<EString*, EntryImpl*>* entry = iter->next();
		EntryImpl* e = entry->getValue();
		deregister(e);
		delete e; //!
	}
}

EString EIoFilterChain::toString() {
	EString buf("{ ");

	boolean empty = true;

	EntryImpl* e = head->nextEntry;

	while (e != tail) {
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

		e = e->nextEntry;
	}

	if (empty) {
		buf.append("empty");
	}

	buf.append(" }");

	return buf;
}

void EIoFilterChain::checkAddable(const char* name) {
    EString ns(name);
	if (name2entry->containsKey(&ns)) {
		EString msg("Other filter is using the same name '");
		msg += name;
		msg += "'";
		throw EIllegalArgumentException(__FILE__, __LINE__, msg.c_str());
	}
}

void EIoFilterChain::register_(EntryImpl* prevEntry, const char* name, EIoFilter* filter) {
	EntryImpl* newEntry = new EntryImpl(prevEntry, prevEntry->nextEntry, name, filter, this);

	prevEntry->nextEntry->prevEntry = newEntry;
	prevEntry->nextEntry = newEntry;
	delete name2entry->put(new EString(name), newEntry);
}

void EIoFilterChain::deregister(EntryImpl* entry) {
	EntryImpl* prevEntry = entry->prevEntry;
	EntryImpl* nextEntry = entry->nextEntry;
	prevEntry->nextEntry = nextEntry;
	nextEntry->prevEntry = prevEntry;

	EString ns(entry->name);
	name2entry->remove(&ns); //delay to free the entry!
}

EIoFilterChain::EntryImpl* EIoFilterChain::checkOldName(const char* baseName) {
	EString ns(baseName);
	EntryImpl* e = dynamic_cast<EntryImpl*>(name2entry->get(&ns));

	if (e == null) {
		EString msg("Filter not found:");
		msg += baseName;
		throw EIllegalArgumentException(__FILE__, __LINE__, msg.c_str());
	}

	return e;
}

boolean EIoFilterChain::fireSessionCreated() {
	return callNextSessionCreated(head, session);
}

boolean EIoFilterChain::callNextSessionCreated(EIoFilterChain::Entry* entry, EIoSession* session) {
	if (!entry) return true;
	EIoFilter* filter = entry->getFilter();
	EIoFilter::NextFilter* nextFilter = entry->getNextFilter();
	return filter->sessionCreated(nextFilter, session);
}

void EIoFilterChain::fireSessionClosed() {
	callNextSessionClosed(head, session);
}

void EIoFilterChain::callNextSessionClosed(Entry* entry, EIoSession* session) {
	if (!entry) return;
	EIoFilter* filter = entry->getFilter();
	EIoFilter::NextFilter* nextFilter = entry->getNextFilter();
	filter->sessionClosed(nextFilter, session);
}

sp<EObject> EIoFilterChain::fireMessageReceived(sp<EObject> message) {
	llong currTime = 0;
	EIoBuffer* buf = dynamic_cast<EIoBuffer*>(message.get());
	if (buf) {
		if (currTime == 0) currTime = ESystem::currentTimeMillis();
		session->increaseReadBytes(buf->remaining(), currTime);
	}
	if (message != null) {
		if (currTime == 0) currTime = ESystem::currentTimeMillis();
		session->increaseReadMessages(currTime);
	}

	return callNextMessageReceived(head, session, message);
}

sp<EObject> EIoFilterChain::callNextMessageReceived(Entry* entry, EIoSession* session, sp<EObject> message) {
	if (!entry) return message;
	EIoFilter* filter = entry->getFilter();
	EIoFilter::NextFilter* nextFilter = entry->getNextFilter();
	return filter->messageReceived(nextFilter, session, message);
}

sp<EObject> EIoFilterChain::fireMessageSend(sp<EObject> message) {
	sp<EObject> o = callNextMessageSend(head, session, message);

	llong currTime = 0;
	EIoBuffer* buf = dynamic_cast<EIoBuffer*>(o.get());
	if (buf) {
		if (currTime == 0) currTime = ESystem::currentTimeMillis();
		session->increaseWrittenBytes(buf->remaining(), currTime);
	} else {
		EFile* file = dynamic_cast<EFile*>(o.get());
		if (file) {
			if (currTime == 0) currTime = ESystem::currentTimeMillis();
			session->increaseWrittenBytes(file->length(), currTime);
		} else {
			throw EIllegalStateException(__FILE__, __LINE__, "Unsupported this message type.");
		}
	}
	if (message != null) {
		if (currTime == 0) currTime = ESystem::currentTimeMillis();
		session->increaseWrittenMessages(currTime);
	}

	return o;
}

sp<EObject> EIoFilterChain::callNextMessageSend(Entry* entry, EIoSession* session, sp<EObject> message) {
	if (!entry) return message;
	EIoFilter* filter = entry->getFilter();
	EIoFilter::NextFilter* nextFilter = entry->getNextFilter();
	return filter->messageSend(nextFilter, session, message);
}

} /* namespace naf */
} /* namespace efc */
