/*
 * NManagedSession.hh
 *
 *  Created on: 2017-3-31
 *      Author: cxxjava@163.com
 */

#ifndef NMANAGEDSESSION_HH_
#define NMANAGEDSESSION_HH_

#include "NIoService.hh"

namespace efc {
namespace naf {

/**
 * Only thread local safe!
 */

class NManagedSession: public EObject {
public:
	NManagedSession(NIoService* service) :
		service(service),
		workThreads(service->getWorkThreads()),
		threadSessions(workThreads) {
		for (int i=0; i<workThreads; i++) {
			threadSessions[i] = new ThreadSessions();
		}
	}

	void addSession(int fd, NIoSession* session) {
		EFiber* fiber = EFiber::currentFiber();
		if (!fiber) {
			throw ENullPointerException(__FILE__, __LINE__, "Out of fiber schedule.");
		}
		ThreadSessions* ts = threadSessions[fiber->getThreadIndex()];
		ts->managedSessions->put(fd, session);
		ts->sessionsCounter++;
	}

	void removeSession(int fd) {
		EFiber* fiber = EFiber::currentFiber();
		if (!fiber) {
			throw ENullPointerException(__FILE__, __LINE__, "Out of fiber schedule.");
		}
		ThreadSessions* ts = threadSessions[fiber->getThreadIndex()];
		ts->managedSessions->remove(fd);
		ts->sessionsCounter--;
	}

	EHashMap<int, NIoSession*>* getCurrentThreadManagedSessions() {
		EFiber* fiber = EFiber::currentFiber();
		if (!fiber) {
			throw ENullPointerException(__FILE__, __LINE__, "Out of fiber schedule.");
		}
		ThreadSessions* ts = threadSessions[fiber->getThreadIndex()];
		return ts->managedSessions;
	}

	int getManagedSessionCount() {
		int count = 0;
		for (int i=0; i<threadSessions.length(); i++) {
			count += threadSessions[i]->sessionsCounter.value();
		}
		return count;
	}

private:
	struct ThreadSessions: public EObject {
		EHashMap<int, NIoSession*>* managedSessions;
		EAtomicCounter sessionsCounter;
		ThreadSessions(): managedSessions(new EHashMap<int, NIoSession*>(8192, false)) {
		}
		~ThreadSessions() {
			delete managedSessions;
		}
	};

	NIoService* service;
	int workThreads;
	EA<ThreadSessions*> threadSessions;
};

} /* namespace naf */
} /* namespace efc */
#endif /* NMANAGEDSESSION_HH_ */
