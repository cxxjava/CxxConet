/*
 * NIoServiceStatistics.cpp
 *
 *  Created on: 2014-9-15
 *      Author: cxxjava@163.com
 */

#include "NIoServiceStatistics.hh"
#include "NIoService.hh"
#include "EFiber.hh"

namespace efc {
namespace naf {

NIoServiceStatistics::NIoServiceStatistics(NIoService* service) :
		service(service),
		workThreads(service->getWorkThreads()),
		threadThroughput(workThreads),
		throughputCalculationInterval(3),
		lastThroughputCalculationTime(0L),
		lastReadBytes(0L),
		lastWrittenBytes(0L),
		lastReadMessages(0L),
		lastWrittenMessages(0L) {
	for (int i=0; i<workThreads; i++) {
		threadThroughput[i] = new ThreadThroughput();
	}
}

int NIoServiceStatistics::getLargestManagedSessionCount() {
	return largestManagedSessionCount.get();
}

llong NIoServiceStatistics::getCumulativeManagedSessionCount() {
	return cumulativeManagedSessionCount.get();
}

llong NIoServiceStatistics::getLastIoTime() {
	return ES_MAX(lastReadTime.get(), lastWriteTime.get());
}

llong NIoServiceStatistics::getLastReadTime() {
	return lastReadTime.get();
}

llong NIoServiceStatistics::getLastWriteTime() {
	return lastWriteTime.get();
}

llong NIoServiceStatistics::getReadBytes() {
	return readBytes.get();
}

llong NIoServiceStatistics::getWrittenBytes() {
	return writtenBytes.get();
}

llong NIoServiceStatistics::getReadMessages() {
	return readMessages.get();
}

llong NIoServiceStatistics::getWrittenMessages() {
	return writtenMessages.get();
}

double NIoServiceStatistics::getReadBytesThroughput() {
	return readBytesThroughput.get();
}

double NIoServiceStatistics::getWrittenBytesThroughput() {
	return writtenBytesThroughput.get();
}

double NIoServiceStatistics::getReadMessagesThroughput() {
	return readMessagesThroughput.get();
}

double NIoServiceStatistics::getWrittenMessagesThroughput() {
	return writtenMessagesThroughput.get();
}

double NIoServiceStatistics::getLargestReadBytesThroughput() {
	return largestReadBytesThroughput.get();
}

double NIoServiceStatistics::getLargestWrittenBytesThroughput() {
	return largestWrittenBytesThroughput.get();
}

double NIoServiceStatistics::getLargestReadMessagesThroughput() {
	return largestReadMessagesThroughput.get();
}

double NIoServiceStatistics::getLargestWrittenMessagesThroughput() {
	return largestWrittenMessagesThroughput.get();
}

int NIoServiceStatistics::getThroughputCalculationInterval() {
	return throughputCalculationInterval.get();
}

void NIoServiceStatistics::setThroughputCalculationInterval(
		int throughputCalculationInterval) {
	if (throughputCalculationInterval < 0) {
		EString msg("throughputCalculationInterval: ");
		msg += throughputCalculationInterval;
		throw EIllegalArgumentException(__FILE__, __LINE__, msg.c_str());
	}

	this->throughputCalculationInterval.set(throughputCalculationInterval);
}

void NIoServiceStatistics::updateThroughput(llong currentTime) {
	// readBytes, writtenBytes, readMessages, writtenMessages, lastReadTime, lastWriteTime
	llong readBytes, writtenBytes, readMessages, writtenMessages;
	llong lastReadTime0, lastReadTime;
	llong lastWriteTime0, lastWriteTime;
	readBytes = writtenBytes = readMessages = writtenMessages = 0L;
	lastReadTime0 = lastReadTime = this->lastReadTime.get();
	lastWriteTime0 = lastWriteTime = this->lastWriteTime.get();
	for (int i=0; i<threadThroughput.length(); i++) {
		ThreadThroughput* tt = threadThroughput[i];
		llong t;

		readBytes += tt->readBytes.get();
		writtenBytes += tt->writtenBytes.get();
		readMessages += tt->readMessages.get();
		writtenMessages += tt->writtenMessages.get();

		t = tt->lastReadTime.get();
		if (t > lastReadTime) {
			lastReadTime = t;
		}

		t = tt->lastWriteTime.get();
		if (t > lastWriteTime) {
			lastWriteTime = t;
		}
	}
	this->readBytes.set(readBytes);
	this->writtenBytes.set(writtenBytes);
	this->readMessages.set(readMessages);
	this->writtenMessages.set(writtenMessages);
	if (lastReadTime >  lastReadTime0) {
		this->lastReadTime.set(lastReadTime);
	}
	if (lastWriteTime >  lastWriteTime0) {
		this->lastWriteTime.set(lastWriteTime);
	}

	// throughput calculate
	int interval = (int) (currentTime - lastThroughputCalculationTime);

	double rbt = (readBytes - lastReadBytes) * 1000.0 / interval;
	readBytesThroughput.set(rbt);
	double wbt = (writtenBytes - lastWrittenBytes) * 1000.0 / interval;
	writtenBytesThroughput.set(wbt);
	double rmt = (readMessages - lastReadMessages) * 1000.0 / interval;
	readMessagesThroughput.set(rmt);
	double wmt = (writtenMessages - lastWrittenMessages) * 1000.0 / interval;
	writtenMessagesThroughput.set(wmt);

	if (rbt > largestReadBytesThroughput.get()) {
		largestReadBytesThroughput.set(rbt);
	}

	if (wbt > largestWrittenBytesThroughput.get()) {
		largestWrittenBytesThroughput.set(wbt);
	}

	if (rmt > largestReadMessagesThroughput.get()) {
		largestReadMessagesThroughput.set(rmt);
	}

	if (wmt > largestWrittenMessagesThroughput.get()) {
		largestWrittenMessagesThroughput.set(wmt);
	}

	lastReadBytes = readBytes;
	lastWrittenBytes = writtenBytes;
	lastReadMessages = readMessages;
	lastWrittenMessages = writtenMessages;

	lastThroughputCalculationTime = currentTime;
}

void NIoServiceStatistics::increaseReadBytes(long increment, llong currentTime) {
	EFiber* fiber = EFiber::currentFiber();
	if (!fiber) {
		throw ENullPointerException(__FILE__, __LINE__, "Out of fiber schedule.");
	}
	ThreadThroughput* tt = threadThroughput[fiber->getThreadIndex()];
	tt->readBytes.addAndGet(increment);
	tt->lastReadTime.set(currentTime);
}

void NIoServiceStatistics::increaseReadMessages(llong currentTime) {
	EFiber* fiber = EFiber::currentFiber();
	if (!fiber) {
		throw ENullPointerException(__FILE__, __LINE__, "Out of fiber schedule.");
	}
	ThreadThroughput* tt = threadThroughput[fiber->getThreadIndex()];
	tt->readMessages.addAndGet(1);
	tt->lastReadTime.set(currentTime);
}

void NIoServiceStatistics::increaseWrittenBytes(int increment, llong currentTime) {
	EFiber* fiber = EFiber::currentFiber();
	if (!fiber) {
		throw ENullPointerException(__FILE__, __LINE__, "Out of fiber schedule.");
	}
	ThreadThroughput* tt = threadThroughput[fiber->getThreadIndex()];
	tt->writtenBytes.addAndGet(increment);
	tt->lastWriteTime.set(currentTime);
}

void NIoServiceStatistics::increaseWrittenMessages(llong currentTime) {
	EFiber* fiber = EFiber::currentFiber();
	if (!fiber) {
		throw ENullPointerException(__FILE__, __LINE__, "Out of fiber schedule.");
	}
	ThreadThroughput* tt = threadThroughput[fiber->getThreadIndex()];
	tt->writtenMessages.addAndGet(1);
	tt->lastWriteTime.set(currentTime);
}
//
//void NIoServiceStatistics::setLastReadTime(llong lastReadTime) {
//	SYNCBLOCK(&throughputCalculationLock) {
//		this->lastReadTime = lastReadTime;
//    }}
//}
//
//void NIoServiceStatistics::setLastWriteTime(llong lastWriteTime) {
//	SYNCBLOCK(&throughputCalculationLock) {
//		this->lastWriteTime = lastWriteTime;
//    }}
//}
//
//void NIoServiceStatistics::setLastThroughputCalculationTime(
//		llong lastThroughputCalculationTime) {
//	SYNCBLOCK(&throughputCalculationLock) {
//		this->lastThroughputCalculationTime = lastThroughputCalculationTime;
//    }}
//}

} /* namespace naf */
} /* namespace efc */
