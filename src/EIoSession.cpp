/*
 * EIoSession.cpp
 *
 *  Created on: 2017-3-16
 *      Author: cxxjava@163.com
 */

#include "../inc/EIoSession.hh"
#include "../inc/EIoService.hh"
#include "../inc/EIoFilterChainBuilder.hh"

namespace efc {
namespace naf {

long EIoSession::idGenerator = 0;

EIoSession::~EIoSession() {
	delete filterChain;
}

EIoSession::EIoSession(EIoService* service):
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

	filterChain = new EIoFilterChain(this);

	// Build the filter chain of this session.
	EIoFilterChainBuilder* chainBuilder = getService()->getFilterChainBuilder();
	chainBuilder->buildFilterChain(filterChain);
}

long EIoSession::getId() {
	return sessionId;
}

EIoService* EIoSession::getService() {
	return service;
}

EIoFilterChain* EIoSession::getFilterChain() {
	return filterChain;
}

llong EIoSession::getCreationTime() {
	return creationTime;
}

llong EIoSession::getLastIoTime() {
	return ES_MAX(lastReadTime, lastWriteTime);
}

llong EIoSession::getLastReadTime() {
	return lastReadTime;
}

llong EIoSession::getLastWriteTime() {
	return lastWriteTime;
}

void EIoSession::increaseReadBytes(long increment, llong currentTime) {
	if (increment <= 0) {
		return;
	}

	readBytes += increment;
	lastReadTime = currentTime;

	service->getStatistics()->increaseReadBytes(increment, currentTime);
}

void EIoSession::increaseReadMessages(llong currentTime) {
	readMessages++;
	lastReadTime = currentTime;

	service->getStatistics()->increaseReadMessages(currentTime);
}

void EIoSession::increaseWrittenBytes(int increment, llong currentTime) {
	if (increment <= 0) {
		return;
	}

	writtenBytes += increment;
	lastWriteTime = currentTime;

	service->getStatistics()->increaseWrittenBytes(increment, currentTime);
}

void EIoSession::increaseWrittenMessages(llong currentTime) {
	writtenMessages++;
	lastWriteTime = currentTime;

	service->getStatistics()->increaseWrittenMessages(currentTime);
}

void* EIoSession::attach(void* ob) {
	return attachment_.getAndSet(ob);
}

void* EIoSession::attachment() {
	return attachment_.get();
}

} /* namespace naf */
} /* namespace efc */
