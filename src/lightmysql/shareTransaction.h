/*
 * shareTransaction.h
 *
 *  Created on: 25. 6. 2015
 *      Author: ondra
 */

#ifndef LIGHTMYSQL_BREDY_SHARETRANSACTION_H_
#define LIGHTMYSQL_BREDY_SHARETRANSACTION_H_

#include <lightspeed/base/actions/promise.h>
#include <lightspeed/base/containers/autoArray.h>
#include <lightspeed/base/memory/poolalloc.h>

#include "query.h"
#include "resourcepool.h"
namespace LightMySQL {

using namespace LightSpeed;


///Transaction shared accros many threads
class SharedTransaction;

///synchronization point to synchronize threads to benefit from shared transaction speedup
/**
 * MySQL has one bottleneck. Every COMMIT is very expensive operation especially when there where
 * INSERTs or UPDATEs. This class allows to synchronize multiple threads writting some similar data at
 * once. Threads can create SharedTransaction and collect all writting operation into single transaction
 *
 * To use SharedTransaction, first declare global object ShareTrnSyncPoint. There can be multiple such objects,
 * each for different type of event. In function which is heavely visited by the many threads and when the
 * function performs many writes to the database, retrieve the SharedTransaction instance using the function
 * ShareTrnSyncPoint::get. You have to specify a connection pool to use.
 *
 * Now, with the SharedTransaction, you can call start() to begin transaction and commit to close connection.
 * Note that rollback operation can be used, but it rollbacks all commands in the shared transaction of the all
 * thread that currently have an opened shared transaction. So it is necessery to catch exception and also
 * build a cycle to allow to repeat transactions that have been rollbacked by an error in other thread
 *
 * How does it work: The first thread calling the start() posses the lock of the sync.point and requests
 * for the connection from the pool. Then a transaction is opened on the connection. Now the thread
 * can execute any INSERT or UPDATE operation and commit the transaction. During the commit, thread explores
 * list of other threads waiting on the sync point and borrows them the transaction which it posses. During
 * this, the thread waits until other thread finish their operations. Once all thread finish, transaction is commited,
 * connection returned back to the pool and all threads are released.
 *
 *
 */
class ShareTrnSyncPoint {
public:

	static unsigned int ERR_SHARED_TRN_ERROR;

	ShareTrnSyncPoint();


	//Retrieve shared transaction from the pool
	/**
	 * @param pool mysql pool - note pool  must remain valid until all shared transactions are released
	 * @return shared transaction.
	 */
	SharedTransaction get(ResourcePool &pool);


protected:

	//status of the operation
	struct Status {
	public:
		//operation resolved
		bool resolved;
		//if != null, operation failed with the exception
		PException e;

		Status():resolved(false) {}
	};

	//custom lock - we need to access addToQueue function to allow reacquire lock before release
	class Lock: public FastLock {
	public:
		//function receives owner - to find out whether current thread own the lock
		const Slot *getOwner() {return FastLock::owner;}
		//we will forge the slot
		using FastLock::Slot;
		//we will put the slot to queue manually
		using FastLock::addToQueue;
	};


	//object is occupied, other threads must wait
	Lock occupied;
	//current status
	AutoArray<Status *> trStatus;
	//current acquired connection
	Optional<ResPtr> curResource;
	//pool for fast allocation of QueryEx objects
	PoolAlloc alloc;

	//when transaction is committed
	void onCommit();
	//when transaction is rollbacked
	void onRollback();
	//when transaction is started
	/*
	 * @param pool pool object to pick transaction up
	 * @param level isolation level
	 * @return current connection, helps to caller access connection directly
	 */
	IConnection *onStart(ResourcePool& pool, IConnection::Level level);
	//rejects whole transaction with an exception
	void rejectAll(const Exception &e);

	friend class SharedTransaction;
	class QueryEx: public IConnection, public Query, public RefCntObj {
	public:

		QueryEx(ResourcePool &pool, ShareTrnSyncPoint &owner);


		virtual Result executeQuery(ConstStrA query);
		virtual StringA escapeString(ConstStrA str);
		virtual void startTransaction(Level isolationLevel);
		virtual void commitTransaction();
		virtual void rollbackTransaction();
		virtual void logString(ConstStrA str, bool error);
		virtual bool isLogEnabled() const;
		virtual bool isConnected() const;

		ResourcePool &pool;
		ShareTrnSyncPoint &owner;
		Pointer<IConnection> nextHop;




	};
};



class SharedTransaction: public Transaction {
public:

	SharedTransaction(ShareTrnSyncPoint::QueryEx *qex);
	~SharedTransaction();

protected:
	RefCntPtr<ShareTrnSyncPoint::QueryEx> ptr;
};

} /* namespace LightMySQL */

#endif /* LIGHTMYSQL_BREDY_SHARETRANSACTION_H_ */
