/*
 * NIoSession.cpp
 *
 *  Created on: 2017-3-16
 *      Author: cxxjava@163.com
 */

#include "NIoSession.hh"
#include "NIoService.hh"
#include "NIoFilterChainBuilder.hh"

namespace efc {
namespace naf {

long NIoSession::idGenerator = 0;

NIoSession::~NIoSession() {
	delete filterChain;
}

NIoSession::NIoSession(NIoService* service):
		service(service),
		creationTime(0),
		lastReadTime(0),
		lastWriteTime(0),
		sessionId(idGenerator++) {

	// Initialize all the Session counters to the current time
	llong currentTime = ESystem::currentTimeMillis();

	creationTime = currentTime;
//	lastThroughputCalculationTime = currentTime;
	lastReadTime = currentTime;
	lastWriteTime = currentTime;
//	lastIdleTimeForBoth = currentTime;
//	lastIdleTimeForRead = currentTime;
//	lastIdleTimeForWrite = currentTime;

	filterChain = new NIoFilterChain(this);

	// Build the filter chain of this session.
	NIoFilterChainBuilder* chainBuilder = getService()->getFilterChainBuilder();
	chainBuilder->buildFilterChain(filterChain);
}

long NIoSession::getId() {
	return sessionId;
}

NIoService* NIoSession::getService() {
	return service;
}

NIoFilterChain* NIoSession::getFilterChain() {
	return filterChain;
}

llong NIoSession::getCreationTime() {
	return creationTime;
}

llong NIoSession::getLastIoTime() {
	return ES_MAX(lastReadTime, lastWriteTime);
}

llong NIoSession::getLastReadTime() {
	return lastReadTime;
}

llong NIoSession::getLastWriteTime() {
	return lastWriteTime;
}

void NIoSession::increaseReadBytes(long increment, llong currentTime) {
	if (increment <= 0) {
		return;
	}

	readBytes += increment;
	lastReadTime = currentTime;

	service->getStatistics()->increaseReadBytes(increment, currentTime);
}

void NIoSession::increaseReadMessages(llong currentTime) {
	readMessages++;
	lastReadTime = currentTime;

	service->getStatistics()->increaseReadMessages(currentTime);
}

void NIoSession::increaseWrittenBytes(int increment, llong currentTime) {
	if (increment <= 0) {
		return;
	}

	writtenBytes += increment;
	lastWriteTime = currentTime;

	service->getStatistics()->increaseWrittenBytes(increment, currentTime);
}

void NIoSession::increaseWrittenMessages(llong currentTime) {
	writtenMessages++;
	lastWriteTime = currentTime;

	service->getStatistics()->increaseWrittenMessages(currentTime);
}

} /* namespace naf */
} /* namespace efc */
