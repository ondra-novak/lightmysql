/*
 * transaction.cpp
 *
 *  Created on: Sep 30, 2012
 *      Author: ondra
 */

#include "transaction.h"
#include "exception.h"
#include "connection.h"
#include "result.h"
#include <mysql/mysqld_error.h>

/*
 * query.select.expr(
 */

namespace LightMySQL {


Transaction::Transaction(Query& queryObj):queryObj(queryObj),retryCount(0),state(stReady) {
}

Query& Transaction::operator ()(ConstStrA queryText) {
	if (state != stStarted) throw UnopenedTransactionException_t(THISLOCATION);
	return queryObj(queryText);

}

Query& Transaction::operator ()() {
	if (state != stStarted) throw UnopenedTransactionException_t(THISLOCATION);
	return queryObj;

}

Transaction::operator Query&() {
	if (state != stStarted) throw UnopenedTransactionException_t(THISLOCATION);
	return queryObj;
}

bool Transaction::start(IConnection::Level isolationLevel) {
	if (state == stReady || state == stRecovered) {

		queryObj.getConnection().startTransaction(isolationLevel);
		state = stStarted;

		/* NOTE - bylo smazano z konstruktoru, neni jasne proc. Kazdopadne
		 * transakce nesmi zacinat s neprazdnym querybufferem, takze odted
		 * se zavolanim start() vzdy querybuffer smaze a nastavi do vychoziho stavu
		 *
		 * Mozne vysvetleni - vnorovani transakci..??
		 */
		queryObj.clear();

		return true;
	} else {
		return false;
	}
}

void Transaction::commit() {
	if (state == stStarted) {
		queryObj.getConnection().commitTransaction();
		state = stCommited;
		retryCount = 0;
	} else if (state == stReady) {
		throw UnopenedTransactionException_t(THISLOCATION);
	}
}

void Transaction::rollback() {
	if (state == stStarted) {
		queryObj.getConnection().rollbackTransaction();
		state = stRollbacked;
	}
}

void Transaction::reset() {
	rollback();
	state = stReady;
}

void Transaction::except(const ServerError_t& e,
		const ProgramLocation& loc) {
	IConnection &conn = queryObj.getConnection();
	rollback();
	unsigned int err = e.getErrno();
	if (err == ER_LOCK_DEADLOCK || err == ER_LOCK_WAIT_TIMEOUT) {
		if (retryCount == 20) {
			conn.logString("Too many retries to solve deadlock state, giving up",true);
			throw e;
		}
		state = stRecovered;
		retryCount++;
		natural delay = retryCount *100;
		if (conn.isLogEnabled()) {

			char buff[200];
			sprintf(buff,"Sleeping for %lu miliseconds to recover from a deadlock",delay);
			conn.logString(buff,false);
		}
		Thread::deepSleep(delay);
	} else {
		LightSpeed::Exception::rethrow(loc);
		throw;
	}

}


Transaction::~Transaction() {
	try {
		rollback();
	} catch (...) {
		//catch exception if we are in exception
		if (!std::uncaught_exception()) throw;
	}
}


void UnopenedTransactionException_t::message(LightSpeed::ExceptionMsg &msg) const {
	msg("Function cannot be used outside the transaction braces.");
}

Result Transaction::exec() {
	 return queryObj.exec();
}

}

/* namespace LightMySQL */

