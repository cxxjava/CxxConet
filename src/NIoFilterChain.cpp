/*
 * NIoFilterChain.cpp
 *
 *  Created on: 2017-3-16
 *      Author: cxxjava@163.com
 */

#include "NIoFilterChain.hh"
#include "NIoFilterAdapter.hh"
#include "NIoBuffer.hh"

namespace efc {
namespace naf {

class NIoFilterChain::EntryImpl: public NIoFilterChain::Entry {
public:
	~EntryImpl() {
		delete nextFilter;
	}

	EntryImpl(EntryImpl* prevEntry, EntryImpl* nextEntry, const char* name,
			NIoFilter* filter, NIoFilterChain* difc) {
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

		class _NextFilter: public NIoFilter::NextFilter {
		private:
			EntryImpl* ei;
			NIoFilterChain* ifc;
		public:
			_NextFilter(EntryImpl* e, NIoFilterChain* f): ei(e), ifc(f) {
			}

			virtual ~_NextFilter() {
			}

			virtual boolean sessionCreated(NIoSession* session) {
				Entry* nextEntry = ei->nextEntry;
				return ifc->callNextSessionCreated(nextEntry, session);
			}

//			virtual void sessionOpened(NIoSession* session) {
//				Entry* nextEntry = ei->nextEntry;
//				ifc->callNextSessionOpened(nextEntry, session);
//			}

			virtual void sessionClosed(NIoSession* session) {
				Entry* nextEntry = ei->nextEntry;
				ifc->callNextSessionClosed(nextEntry, session);
			}

//			virtual void sessionIdle(NIoSession* session, EIdleStatus status) {
//				Entry* nextEntry = ei->nextEntry;
//				ifc->callNextSessionIdle(nextEntry, session, status);
//			}
//
//			virtual void exceptionCaught(NIoSession* session, sp<EThrowableType>& cause) {
//				Entry* nextEntry = ei->nextEntry;
//				ifc->callNextExceptionCaught(nextEntry, session, cause);
//			}
//
//			virtual void inputClosed(NIoSession* session) {
//				Entry* nextEntry = ei->nextEntry;
//				ifc->callNextInputClosed(nextEntry, session);
//			}

			virtual sp<EObject> messageReceived(NIoSession* session, sp<EObject> message) {
				Entry* nextEntry = ei->nextEntry;
				return ifc->callNextMessageReceived(nextEntry, session, message);
			}

			sp<EObject> messageSend(NIoSession* session, sp<EObject> message) {
				Entry* nextEntry = ei->nextEntry;
				return ifc->callNextMessageSend(nextEntry, session, message);
			}

//			virtual void filterWrite(NIoSession* session, sp<EWriteRequest>& writeRequest) {
//				Entry* nextEntry = ei->prevEntry;
//				ifc->callPreviousFilterWrite(nextEntry, session, writeRequest);
//			}
//
//			virtual void filterClose(NIoSession* session) {
//				Entry* nextEntry = ei->prevEntry;
//				ifc->callPreviousFilterClose(nextEntry, session);
//			}

			virtual EStringBase toString() {
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
	NIoFilter* getFilter() {
		return filter;
	}

	/**
	 * @return The {@link NextFilter} of the filter.
	 */
	NIoFilter::NextFilter* getNextFilter() {
		return nextFilter;
	}

	/**
	 * Adds the specified filter with the specified name just before this entry.
	 *
	 * @param name The Filter's name
	 * @param filter The added Filter
	 */
	void addBefore(const char* name, NIoFilter* filter) {
		owner->addBefore(getName(), name, filter);
	}

	/**
	 * Adds the specified filter with the specified name just after this entry.
	 *
	 * @param name The Filter's name
	 * @param filter The added Filter
	 */
	void addAfter(const char* name, NIoFilter* filter) {
		owner->addAfter(getName(), name, filter);
	}

	/**
	 * Replace the filter of this entry with the specified new filter.
	 *
	 * @param newFilter The new filter that will be put in the chain
	 */
//	void replace(NIoFilter* newFilter) {
//
//	}

	/**
	 * Removes this entry from the chain it belongs to.
	 */
	void remove() {
		owner->remove(getName());
	}

	virtual EStringBase toString() {
		EStringBase sb;

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
	NIoFilter* filter;
	NIoFilter::NextFilter* nextFilter;
	NIoFilterChain* owner;
};

NIoFilterChain::~NIoFilterChain() {
	delete head->getFilter(); //!
	delete head;
	delete tail->getFilter(); //!
	delete tail;
	delete name2entry;
}

NIoFilterChain::NIoFilterChain(NIoSession* session) {
	if (session == null) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "session");
	}

	this->session = session;
	head = new EntryImpl(null, null, "head", new NIoFilterAdapter(), this);
	tail = new EntryImpl(head, null, "tail", new NIoFilterAdapter(), this);
	head->nextEntry = tail;

	name2entry = new EHashMap<EString*, EntryImpl*>();
}

NIoSession* NIoFilterChain::getSession() {
	return session;
}

NIoFilterChain::Entry* NIoFilterChain::getEntry(
		const char* name) {
	EString ns(name);
	return name2entry->get(&ns);
}

NIoFilterChain::Entry* NIoFilterChain::getEntry(
		NIoFilter* filter) {
	EntryImpl* e = head->nextEntry;

	while (e != tail) {
		if (e->getFilter() == filter) {
			return e;
		}

		e = e->nextEntry;
	}

	return null;
}

NIoFilter* NIoFilterChain::get(const char* name) {
	NIoFilterChain::Entry* e = getEntry(name);

	if (e == null) {
		return null;
	}

	return e->getFilter();
}

boolean NIoFilterChain::contains(const char* name) {
	return getEntry(name) != null;
}

boolean NIoFilterChain::contains(NIoFilter* filter) {
	return getEntry(filter) != null;
}

NIoFilter::NextFilter* NIoFilterChain::getNextFilter(const char* name) {
	NIoFilterChain::Entry* e = getEntry(name);

	if (e == null) {
		return null;
	}

	return e->getNextFilter();
}

NIoFilter::NextFilter* NIoFilterChain::getNextFilter(NIoFilter* filter) {
	NIoFilterChain::Entry* e = getEntry(filter);
	if (e == null) {
		return null;
	}
	return e->getNextFilter();
}

void NIoFilterChain::addFirst(const char* name, NIoFilter* filter) {
	checkAddable(name);
	register_(head, name, filter);
}

void NIoFilterChain::addLast(const char* name, NIoFilter* filter) {
	checkAddable(name);
	register_(tail->prevEntry, name, filter);
}

void NIoFilterChain::addBefore(const char* baseName, const char* name,
		NIoFilter* filter) {
	EntryImpl* baseEntry = checkOldName(baseName);
	checkAddable(name);
	register_(baseEntry->prevEntry, name, filter);
}

void NIoFilterChain::addAfter(const char* baseName, const char* name,
		NIoFilter* filter) {
	EntryImpl* baseEntry = checkOldName(baseName);
	checkAddable(name);
	register_(baseEntry, name, filter);
}

NIoFilter* NIoFilterChain::remove(const char* name) {
	EntryImpl* entry = checkOldName(name);
	deregister(entry);
	NIoFilter* filter = entry->getFilter();
	delete entry; //!
	return filter;
}

NIoFilter* NIoFilterChain::remove(NIoFilter* filter) {
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

void NIoFilterChain::clear() {
	sp<EIterator<EMapEntry<EString*, EntryImpl*>*> > iter = name2entry->entrySet()->iterator();
	while (iter->hasNext()) {
		EMapEntry<EString*, EntryImpl*>* entry = iter->next();
		EntryImpl* e = entry->getValue();
		deregister(e);
		delete e; //!
	}
}

EStringBase NIoFilterChain::toString() {
	EStringBase buf("{ ");

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

void NIoFilterChain::checkAddable(const char* name) {
    EString ns(name);
	if (name2entry->containsKey(&ns)) {
		EString msg("Other filter is using the same name '");
		msg += name;
		msg += "'";
		throw EIllegalArgumentException(__FILE__, __LINE__, msg.c_str());
	}
}

void NIoFilterChain::register_(EntryImpl* prevEntry, const char* name, NIoFilter* filter) {
	EntryImpl* newEntry = new EntryImpl(prevEntry, prevEntry->nextEntry, name, filter, this);

//	try {
//		filter->onPreAdd(this, name, newEntry->getNextFilter());
//	} catch (EException& e) {
//		delete newEntry; //!
//
//		EString msg("onPreAdd(): ");
//		msg += name;
//		msg += ":";
//		msg += filter->toString();
//		msg += " in ";
//		msg += getSession()->toString();
//		throw NIoFilterLifeCycleException(__FILE__, __LINE__, msg.c_str());
//	}

	prevEntry->nextEntry->prevEntry = newEntry;
	prevEntry->nextEntry = newEntry;
	delete name2entry->put(new EString(name), newEntry);

//	try {
//		filter->onPostAdd(this, name, newEntry->getNextFilter());
//	} catch (EException& e) {
//		deregister0(newEntry);
//		delete newEntry; //!
//
//		EString msg("onPostAdd(): ");
//		msg += name;
//		msg += ":";
//		msg += filter->toString();
//		msg += " in ";
//		msg += getSession()->toString();
//		throw NIoFilterLifeCycleException(__FILE__, __LINE__, msg.c_str());
//	}
}

void NIoFilterChain::deregister(EntryImpl* entry) {
	EntryImpl* prevEntry = entry->prevEntry;
	EntryImpl* nextEntry = entry->nextEntry;
	prevEntry->nextEntry = nextEntry;
	nextEntry->prevEntry = prevEntry;

	EString ns(entry->name);
	name2entry->remove(&ns); //delay to free the entry!
}

NIoFilterChain::EntryImpl* NIoFilterChain::checkOldName(const char* baseName) {
	EString ns(baseName);
	EntryImpl* e = dynamic_cast<EntryImpl*>(name2entry->get(&ns));

	if (e == null) {
		EString msg("Filter not found:");
		msg += baseName;
		throw EIllegalArgumentException(__FILE__, __LINE__, msg.c_str());
	}

	return e;
}

boolean NIoFilterChain::fireSessionCreated() {
	return callNextSessionCreated(head, session);
}

boolean NIoFilterChain::callNextSessionCreated(NIoFilterChain::Entry* entry, NIoSession* session) {
	if (!entry) return true;
	NIoFilter* filter = entry->getFilter();
	NIoFilter::NextFilter* nextFilter = entry->getNextFilter();
	return filter->sessionCreated(nextFilter, session);
}

void NIoFilterChain::fireSessionClosed() {
	callNextSessionClosed(head, session);
}

void NIoFilterChain::callNextSessionClosed(Entry* entry, NIoSession* session) {
	if (!entry) return;
	NIoFilter* filter = entry->getFilter();
	NIoFilter::NextFilter* nextFilter = entry->getNextFilter();
	filter->sessionClosed(nextFilter, session);
}

sp<EObject> NIoFilterChain::fireMessageReceived(sp<EObject> message) {
	llong currTime = 0;
	NIoBuffer* buf = dynamic_cast<NIoBuffer*>(message.get());
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

sp<EObject> NIoFilterChain::callNextMessageReceived(Entry* entry, NIoSession* session, sp<EObject> message) {
	if (!entry) return message;
	NIoFilter* filter = entry->getFilter();
	NIoFilter::NextFilter* nextFilter = entry->getNextFilter();
	return filter->messageReceived(nextFilter, session, message);
}

sp<EObject> NIoFilterChain::fireMessageSend(sp<EObject> message) {
	sp<EObject> o = callNextMessageSend(head, session, message);

	llong currTime = 0;
	NIoBuffer* buf = dynamic_cast<NIoBuffer*>(o.get());
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

sp<EObject> NIoFilterChain::callNextMessageSend(Entry* entry, NIoSession* session, sp<EObject> message) {
	if (!entry) return message;
	NIoFilter* filter = entry->getFilter();
	NIoFilter::NextFilter* nextFilter = entry->getNextFilter();
	return filter->messageSend(nextFilter, session, message);
}

} /* namespace naf */
} /* namespace efc */
