/*
 * EIoServiceStatistics.hh
 *
 *  Created on: 2017-4-15
 *      Author: cxxjava@163.com
 */

#ifndef EIOSERVICESTATISTICS_HH_
#define EIOSERVICESTATISTICS_HH_

#include "Efc.hh"

namespace efc {
namespace naf {

class EIoService;

/**
 * Provides usage statistics for an {@link EIoService} instance.
 *
 */
class EIoServiceStatistics : public EObject {
public:
	EIoServiceStatistics(EIoService* service);

	/**
	 * Returns the maximum number of sessions which were being managed at the
	 * same time.
	 */
	int getLargestManagedSessionCount();

	/**
	 * Returns the cumulative number of sessions which were managed (or are
	 * being managed) by this service, which means 'currently managed session
	 * count + closed session count'.
	 */
	llong getCumulativeManagedSessionCount();

	/**
	 * Returns the time in millis when I/O occurred lastly.
	 */
	llong getLastIoTime();

	/**
	 * Returns the time in millis when read operation occurred lastly.
	 */
	llong getLastReadTime();

	/**
	 * Returns the time in millis when write operation occurred lastly.
	 */
	llong getLastWriteTime();

	/**
	 * Returns the number of bytes read by this service
	 *
	 * @return
	 *     The number of bytes this service has read
	 */
	llong getReadBytes();

	/**
	 * Returns the number of bytes written out by this service
	 *
	 * @return
	 *     The number of bytes this service has written
	 */
	llong getWrittenBytes();

	/**
	 * Returns the number of messages this services has read
	 *
	 * @return
	 *     The number of messages this services has read
	 */
	llong getReadMessages();

	/**
	 * Returns the number of messages this service has written
	 *
	 * @return
	 *     The number of messages this service has written
	 */
	llong getWrittenMessages();

	/**
	 * Returns the number of read bytes per second.
	 */
	double getReadBytesThroughput();

	/**
	 * Returns the number of written bytes per second.
	 */
	double getWrittenBytesThroughput();

	/**
	 * Returns the number of read messages per second.
	 */
	double getReadMessagesThroughput();

	/**
	 * Returns the number of written messages per second.
	 */
	double getWrittenMessagesThroughput();

	/**
	 * Returns the maximum of the {@link #getReadBytesThroughput() readBytesThroughput}.
	 */
	double getLargestReadBytesThroughput();

	/**
	 * Returns the maximum of the {@link #getWrittenBytesThroughput() writtenBytesThroughput}.
	 */
	double getLargestWrittenBytesThroughput();

	/**
	 * Returns the maximum of the {@link #getReadMessagesThroughput() readMessagesThroughput}.
	 */
	double getLargestReadMessagesThroughput();

	/**
	 * Returns the maximum of the {@link #getWrittenMessagesThroughput() writtenMessagesThroughput}.
	 */
	double getLargestWrittenMessagesThroughput();

	/**
	 * Returns the interval (seconds) between each throughput calculation.
	 * The default value is <tt>3</tt> seconds.
	 */
	int getThroughputCalculationInterval();

	/**
	 * Sets the interval (seconds) between each throughput calculation.  The
	 * default value is <tt>3</tt> seconds.
	 *
	 * @param throughputCalculationInterval The interval between two calculation
	 */
	void setThroughputCalculationInterval(int throughputCalculationInterval);

protected:
	friend class ESocketAcceptor;
	friend class EIoSession;

	EAtomicDouble readBytesThroughput;
	EAtomicDouble writtenBytesThroughput;
	EAtomicDouble readMessagesThroughput;
	EAtomicDouble writtenMessagesThroughput;
	EAtomicDouble largestReadBytesThroughput;
	EAtomicDouble largestWrittenBytesThroughput;
	EAtomicDouble largestReadMessagesThroughput;
	EAtomicDouble largestWrittenMessagesThroughput;

	EAtomicLLong readBytes;
	EAtomicLLong writtenBytes;
	EAtomicLLong readMessages;
	EAtomicLLong writtenMessages;
	EAtomicLLong lastReadTime;
	EAtomicLLong lastWriteTime;

	/** The time (in second) between the computation of the service's statistics */
	EAtomicInteger throughputCalculationInterval;// = new AtomicInteger(3);

	/** A counter used to store the maximum sessions we managed since the listenerSupport has been activated */
	EAtomicInteger largestManagedSessionCount;// = 0;

	/** A global counter to count the number of sessions managed since the start */
	EAtomicLLong cumulativeManagedSessionCount;// = 0;

	/**
	 * Increases the count of read bytes by <code>increment</code> and sets
	 * the last read time to <code>currentTime</code>.
	 */
	void increaseReadBytes(long increment, llong currentTime);

	/**
	 * Increases the count of read messages by 1 and sets the last read time to
	 * <code>currentTime</code>.
	 */
	void increaseReadMessages(llong currentTime);

	/**
	 * Increases the count of written bytes by <code>increment</code> and sets
	 * the last write time to <code>currentTime</code>.
	 */
	void increaseWrittenBytes(int increment, llong currentTime);

	/**
	 * Increases the count of written messages by 1 and sets the last write time to
	 * <code>currentTime</code>.
	 */
	void increaseWrittenMessages(llong currentTime);

	/**
	 * Updates the throughput counters.
	 *
	 * @param currentTime The current time
	 */
	void updateThroughput(llong currentTime);

private:
	struct ThreadThroughput: public EObject {
		/** The number of bytes read per second */
		EAtomicDouble readBytesThroughput;

		/** The number of bytes written per second */
		EAtomicDouble writtenBytesThroughput;

		/** The number of messages read per second */
		EAtomicDouble readMessagesThroughput;

		/** The number of messages written per second */
		EAtomicDouble writtenMessagesThroughput;

		/** The biggest number of bytes read per second */
		EAtomicDouble largestReadBytesThroughput;

		/** The biggest number of bytes written per second */
		EAtomicDouble largestWrittenBytesThroughput;

		/** The biggest number of messages read per second */
		EAtomicDouble largestReadMessagesThroughput;

		/** The biggest number of messages written per second */
		EAtomicDouble largestWrittenMessagesThroughput;

		/** The number of read bytes since the service has been started */
		EAtomicLLong readBytes;

		/** The number of written bytes since the service has been started */
		EAtomicLLong writtenBytes;

		/** The number of read messages since the service has been started */
		EAtomicLLong readMessages;

		/** The number of written messages since the service has been started */
		EAtomicLLong writtenMessages;

		/** The time the last read operation occurred */
		EAtomicLLong lastReadTime;

		/** The time the last write operation occurred */
		EAtomicLLong lastWriteTime;
	};

	EIoService* service;
	int workThreads;
	EA<ThreadThroughput*> threadThroughput;

	llong lastThroughputCalculationTime;
	llong lastReadBytes;
	llong lastWrittenBytes;
	llong lastReadMessages;
	llong lastWrittenMessages;
};

} /* namespace naf */
} /* namespace efc */
#endif /* EIOSERVICESTATISTICS_HH_ */
