/*
 * EIoFilterChain.hh
 *
 *  Created on: 2017-3-16
 *      Author: cxxjava@163.com
 */

#ifndef NFILTERCHAIN_HH_
#define NFILTERCHAIN_HH_

#include "./EIoFilter.hh"

namespace efc {
namespace naf {

/**
 * A default implementation of {@link IoFilterChain} that provides
 * all operations for developers who want to implement their own
 * transport layer once used with {@link EIoSession}.
 *
 */

class EIoFilterChain: public EObject {
public:
	/**
	 * Represents a name-filter pair that an {@link IoFilterChain} contains.
	 *
	 * @author <a href="http://mina.apache.org">Apache MINA Project</a>
	 */
	interface Entry: virtual public EObject {
		/**
		 * @return the name of the filter.
		 */
		virtual const char* getName() = 0;

		/**
		 * @return the filter.
		 */
		virtual EIoFilter* getFilter() = 0;

		/**
		 * @return The {@link NextFilter} of the filter.
		 */
		virtual EIoFilter::NextFilter* getNextFilter() = 0;

		/**
		 * Adds the specified filter with the specified name just before this entry.
		 *
		 * @param name The Filter's name
		 * @param filter The added Filter
		 */
		virtual void addBefore(const char* name, EIoFilter* filter) = 0;

		/**
		 * Adds the specified filter with the specified name just after this entry.
		 *
		 * @param name The Filter's name
		 * @param filter The added Filter
		 */
		virtual void addAfter(const char* name, EIoFilter* filter) = 0;

		/**
		 * Removes this entry from the chain it belongs to.
		 */
		virtual void remove() = 0;
	};

public:
	virtual ~EIoFilterChain();

	/**
	 * Creates a new instance with an empty filter list.
	 */
	EIoFilterChain(EIoSession* session);

	/**
	 * Get the associated session
	 */
	EIoSession* getSession();

	/**
	 * Returns the {@link Entry} with the specified <tt>name</tt> in this chain.
	 *
	 * @param name The filter's name we are looking for
	 * @return <tt>null</tt> if there's no such name in this chain
	 */
	Entry* getEntry(const char* name);

	/**
	 * @see IoFilterChain#getEntry(IoFilter)
	 *
	 * @param filter The Filter we are looking for
	 * @return The found Entry
	 */
	Entry* getEntry(EIoFilter* filter);

	/**
	 * @see IoFilterChain#get(String)
	 *
	 * @param name The Filter's name we are looking for
	 * @return The found Filter, or null
	 */
	EIoFilter* get(const char* name);

	/**
	 * @see IoFilterChain#contains(String)
	 *
	 * @param name The Filter's name we want to check if it's in the chain
	 * @return <tt>true</tt> if the chain contains the given filter name
	 */
	boolean contains(const char* name);

	/**
	 * @see IoFilterChain#contains(IoFilter)
	 *
	 * @param filter The Filter we want to check if it's in the chain
	 * @return <tt>true</tt> if the chain contains the given filter
	 */
	boolean contains(EIoFilter* filter);

	/**
	 *
	 */
	EIoFilter::NextFilter* getNextFilter(const char* name);
	EIoFilter::NextFilter* getNextFilter(EIoFilter* filter);

	/**
	 * @see IoFilterChain#addFirst(String, IoFilter)
	 *
	 * @param name The filter's name
	 * @param filter The filter to add
	 */
	void addFirst(const char* name, EIoFilter* filter);

	/**
	 * @see IoFilterChain#addLast(String, IoFilter)
	 *
	 * @param name The filter's name
	 * @param filter The filter to add
	 */
	void addLast(const char* name, EIoFilter* filter);

	/**
	 * @see IoFilterChain#addBefore(String, String, IoFilter)
	 *
	 * @param baseName The filter baseName
	 * @param name The filter's name
	 * @param filter The filter to add
	 */
	void addBefore(const char* baseName, const char* name, EIoFilter* filter);

	/**
	 * @see IoFilterChain#addAfter(String, String, IoFilter)
	 *
	 * @param baseName The filter baseName
	 * @param name The filter's name
	 * @param filter The filter to add
	 */
	void addAfter(const char* baseName, const char* name, EIoFilter* filter);

	/**
	 * @see IoFilterChain#remove(String)
	 *
	 * @param name The Filter's name to remove from the list of Filters
	 * @return The removed IoFilter
	 */
	EIoFilter* remove(const char* name);

	/**
	 * @see IoFilterChain#remove(IoFilter)
	 *
	 * @param filter The Filter we want to remove from the list of Filters
	 * @return The removed IoFilter
	 */
	EIoFilter* remove(EIoFilter* filter);

	/**
	 * @see IoFilterChain#clear()
	 */
	void clear();

	/**
	 * Fires a {@link IoHandler#sessionCreated(IoSession)} event. Most users don't need to
	 * call this method at all. Please use this method only when you implement a new transport
	 * or fire a virtual event.
	 */
	virtual boolean fireSessionCreated();

	/**
	 * Fires a {@link IoHandler#sessionClosed(IoSession)} event. Most users don't need to call
	 * this method at all. Please use this method only when you implement a new transport or
	 * fire a virtual event.
	 */
	virtual void fireSessionClosed();

	/**
	 * Fires a {@link IoHandler#messageReceived(Object)} event. Most users don't need to
	 * call this method at all. Please use this method only when you implement a new transport
	 * or fire a virtual event.
	 *
	 * @param message The received message
	 */
	virtual sp<EObject> fireMessageReceived(sp<EObject> message);

	/**
	 * Fires a {@link IoHandler#messageSend(IoSession)} event. Most users don't need to call
	 * this method at all. Please use this method only when you implement a new transport or
	 * fire a virtual event.
	 *
	 * @param request The sent request
	 */
	virtual sp<EObject> fireMessageSend(sp<EObject> message);

	/**
	 *
	 */
	virtual EString toString();

private:
	class EntryImpl;

	/** The associated session */
	EIoSession* session;

	/** The mapping between the filters and their associated name */
	//cxxjava: name2entry use HashMap at mina-2.0.0-RC1 and use ConcurrentHashMap at later versions.
	EMap<EString*, EntryImpl*>* name2entry;

	/** The chain head */
	EntryImpl* head;

	/** The chain tail */
	EntryImpl* tail;

	/**
	 * Checks the specified filter name is already taken and throws an exception if already taken.
	 */
	void checkAddable(const char* name);

	/**
	 * Register the newly added filter, inserting it between the previous and
	 * the next filter in the filter's chain. We also call the preAdd and
	 * postAdd methods.
	 */
	void register_(EntryImpl* prevEntry, const char* name, EIoFilter* filter);

	void deregister(EntryImpl* entry);

	/**
	 * Throws an exception when the specified filter name is not registered in this chain.
	 *
	 * @return An filter entry with the specified name.
	 */
	EntryImpl* checkOldName(const char* baseName);

	boolean callNextSessionCreated(Entry* entry, EIoSession* session);
	void callNextSessionClosed(Entry* entry, EIoSession* session);
	sp<EObject> callNextMessageReceived(Entry* entry, EIoSession* session, sp<EObject> message);
	sp<EObject> callNextMessageSend(Entry* entry, EIoSession* session, sp<EObject> message);
};

} /* namespace naf */
} /* namespace efc */
#endif /* NFILTERCHAIN_HH_ */
