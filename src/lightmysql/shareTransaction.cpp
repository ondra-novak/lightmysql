/*
 * shareTransaction.cpp
 *
 *  Created on: 25. 6. 2015
 *      Author: ondra
 */

#include "shareTransaction.h"

#include <lightspeed/base/actions/promise.tcc>
#include <lightspeed/base/constructor.h>
#include <lightspeed/base/exceptions/stdexception.h>
#include <mysql/mysqld_error.h>

#include "result.h"
namespace LightMySQL {

unsigned int ShareTrnSyncPoint::ERR_SHARED_TRN_ERROR = ER_ERROR_LAST;

ShareTrnSyncPoint::ShareTrnSyncPoint() {
}


SharedTransaction ShareTrnSyncPoint::get(ResourcePool& pool) {
	class DynQueryEx: public QueryEx, public DynObject {
	public:
		DynQueryEx(ResourcePool &pool, ShareTrnSyncPoint &owner):QueryEx(pool,owner) {}
	};
	return SharedTransaction(new(alloc) DynQueryEx(pool, *this));
}

IConnection *ShareTrnSyncPoint::onStart(ResourcePool& pool, IConnection::Level level) {
	occupied.lock();
	try {
		if (curResource == nil) {
			ResPtr x(pool);
			curResource = x;
			ResPtr &res = curResource;
			res->startTransaction(level);
			return res;
		} else {
			ResPtr &res = curResource;
			return res;
		}
	} catch (...) {
		curResource = nil;
		occupied.unlock();
		throw;
	}
}
void ShareTrnSyncPoint::onCommit() {

	//status object - every thread waiting for finish transaction must register one
	Status st;
	//register status object
	trStatus.add(&st);
	//create slot for lock
	//because the thread currently under lock, it cannot call lock again.
	//But thread need to register itself to the lock's queue again to prevent racing after lock is released
	//any thread arrives later will be queued to executing after this thread finishes
	Lock::Slot slot(getCurThreadSleepingObj());
	//we can call directly addToQueue because the lock still have the owner - us.
	occupied.addToQueue(&slot);
	//now release lock, any thread waiting before us can run and use the opened transaction.
	occupied.unlock();
	//wait for finishing - every thread stops here waiting for ownership of the lock
	while (occupied.getOwner() != &slot) threadHalt();
	//we gained ownership, again under the lock
	//check the status - if status is not yet resolved, current thread must close the transaction
	if (!st.resolved) {
		try {
			ResPtr &res = curResource;
				res->commitTransaction();
				for (natural i = 0; i < trStatus.length(); i++)
					trStatus(i)->resolved = true;
				trStatus.clear();
		} catch (Exception &e) {
			rejectAll(e);
		} catch (std::exception &e) {
			rejectAll(StdException(THISLOCATION,e));
		} catch (...) {
			rejectAll(UnknownException(THISLOCATION));
		}
		//return connection back to pool
		curResource = nil;
	}
	//release the lock
	occupied.unlock();
	if (st.e != nil)
		st.e->throwAgain(THISLOCATION);
}

void ShareTrnSyncPoint::onRollback() {

	//when rollback, whole transaction is lost
	try {
		//rollback is transaction first
		ResPtr &res = curResource;
		res->rollbackTransaction();
	} catch (Exception &e) {
		//if exception during rollback, we have to broadcast the exception to all threads
		rejectAll(e);
		//clear current resource
		curResource = nil;
		//and unlock the object
		occupied.unlock();
		//rethrow exception to the current thread
		throw;
	}
	//transaction rollbacked - now reject all threads by special exception
	//this exception causes, that each thread will repeat its transaction similar to deadlock
	rejectAll(ServerError_t(THISLOCATION,ERR_SHARED_TRN_ERROR,"Shared transaction rollbacked - please repeat"));
	//return connection back to the pool
	curResource = nil;
	//unlock the object to give other threads chance to run
	occupied.unlock();
}

void ShareTrnSyncPoint::rejectAll(const Exception& e) {
	for (natural i = 0; i < trStatus.length(); i++) {
		trStatus(i)->resolved = true;
		trStatus(i)->e = e.clone();
	}
	trStatus.clear();
}


SharedTransaction::SharedTransaction(ShareTrnSyncPoint::QueryEx* qex)
	:Transaction(*qex),ptr(qex)
{
}

SharedTransaction::~SharedTransaction() {
}


ShareTrnSyncPoint::QueryEx::QueryEx(ResourcePool& pool,ShareTrnSyncPoint& owner)
	             :Query(*static_cast<IConnection *>(this)),pool(pool),owner(owner) {}


Result ShareTrnSyncPoint::QueryEx::executeQuery(ConstStrA query) {
			return nextHop->executeQuery(query);
		}

StringA ShareTrnSyncPoint::QueryEx::escapeString(ConstStrA str)  {
			return nextHop->escapeString(str);
		}

void ShareTrnSyncPoint::QueryEx::startTransaction(Level isolationLevel)  {
			nextHop = owner.onStart(pool,isolationLevel);
		}

void ShareTrnSyncPoint::QueryEx::commitTransaction()  {
			owner.onCommit();
		}

void ShareTrnSyncPoint::QueryEx::rollbackTransaction() {
			owner.onRollback();
		}

void ShareTrnSyncPoint::QueryEx::logString(ConstStrA str, bool error)  {
			if (nextHop != nil) nextHop->logString(str,error);
		}

bool ShareTrnSyncPoint::QueryEx::isLogEnabled() const  {
			return nextHop != nil && nextHop->isLogEnabled();
		}

bool ShareTrnSyncPoint::QueryEx::isConnected() const   {
			return nextHop != nil && nextHop->isConnected();
		}

} /* namespace LightMySQL */
