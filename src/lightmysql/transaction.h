/*
 * transaction.h
 *
 *  Created on: Sep 30, 2012
 *      Author: ondra
 */

#ifndef LIGHTMYSL_TRANSACTION_H_
#define LIGHTMYSL_TRANSACTION_H_
#include "query.h"


#pragma once

namespace LightMySQL {

using namespace LightSpeed;



class Transaction {
public:


	Transaction(Query& queryObj);
	~Transaction();

	Query &operator()(ConstStrA queryText);
	Query &operator()();
	operator Query &();


	///Starts exception
	/**
	 * @param isolationLevel specify isolation level
	 * @retval true transaction has been started
	 * @retval false transaction has been already started
	 *
	 * @code
	 * for (LightMySQL::Transaction t(queryObj);t.start();t.commit()) try {
	 * 		// work with transaction
	 *
	 * } catch (LightMySQL::ServerError &e) {
	 *     t.except(e,THISLOCATION)
	 * }
 	 * @endcode
	 *
	 * @note every transaction object should be started once only. Function start() returns false
	 * until reset() is called or except() with deadlock error is called.
	 *
	 * @note commit() in for cycle is called until transaction section exits with return. In this
	 * case, you have to call commit() manually, otherwise transaction is rollbacked
	 *
	 */

	bool start(IConnection::Level isolationLevel = IConnection::defaultLevel);

	void commit();

	void rollback();

	void reset();

	Query &INSERT(ConstStrA pattern) {return (*this)().end().INSERT(pattern);}
	Query &INSERT_IGNORE(ConstStrA pattern) {return (*this)().end().INSERT_IGNORE(pattern);}
	Query &UPDATE(ConstStrA pattern) {return (*this)().end().UPDATE(pattern);}
	Query &REPLACE(ConstStrA pattern) {return (*this)().end().REPLACE(pattern);}
	Query &DELETE(ConstStrA pattern) {return (*this)().end().DELETE(pattern);}
	Query &DELETE() {return (*this)().end().DELETE();}
	Query &SELECT(ConstStrA pattern, bool calcfoundrows = false) {return (*this)().end().SELECT(pattern,calcfoundrows);}
	Query &SELECT_DISTINCT(ConstStrA pattern, bool calcfoundrows = false) {return (*this)().end().SELECT_DISTINCT(pattern,calcfoundrows);}
	Query &SELECT_DISTINCTROW(ConstStrA pattern, bool calcfoundrows = false) {return (*this)().end().SELECT_DISTINCTROW(pattern,calcfoundrows);}
	Query &SET(ConstStrA field, ConstStrA pattern) {return (*this)().end().SET(field,pattern);}
	Query &SET(ConstStrA field) {return (*this)().end().SET(field);}
	CreateTableDef CREATE_TEMPORARY_TABLE(ConstStrA tableName) {return (*this)().end().CREATE_TEMPORARY_TABLE(tableName);}





	void except(const ServerError_t &e, const ProgramLocation &loc);

	template<typename Ret>
	Ret exec(const ProgramLocation &loc, Ret (*fn)(Transaction &trn));
	template<typename Ret, typename Arg1>
	Ret exec(const ProgramLocation &loc, Ret (*fn)(Transaction &trn, Arg1 ), Arg1 arg);
	template<typename Ret, typename Arg1, typename Arg2>
	Ret exec(const ProgramLocation &loc, Ret (*fn)(Transaction &trn,  Arg1 , Arg2 ), Arg1 arg1, Arg2 arg2);
	template<typename Ret, typename Obj>
	Ret exec(const ProgramLocation &loc, Obj *obj, Ret (Obj::*fn)(Transaction &trn));
	template<typename Ret, typename Obj>
	Ret exec(const ProgramLocation &loc, const Obj *obj, Ret (Obj::*fn)(Transaction &trn));
	template<typename Ret, typename Obj, typename Arg1>
	Ret exec(const ProgramLocation &loc, Obj *obj, Ret (Obj::*fn)(Transaction &trn, Arg1 ), Arg1 arg);
	template<typename Ret, typename Obj, typename Arg1>
	Ret exec(const ProgramLocation &loc, const Obj *obj, Ret (Obj::*fn)(Transaction &trn, Arg1 ), Arg1 arg);

	///Executes prepared query
	Result exec();


	class IsolationLevelRef {
	public:
		Transaction &trn;
		IConnection::Level level;

		IsolationLevelRef(Transaction &trn,IConnection::Level level)
			:trn(trn),level(level) {}

		template<typename Fn>
		IsolationLevelRef operator>>(const Fn &fn) {
			if (trn.state == stStarted) {
				fn();
			} else {
				while (trn.start(level)) try {
					fn();
					trn.commit();
				} catch (ServerError_t &e) {
					trn.except(e,THISLOCATION);
				}
			}
			return *this;
		}
	};

	///For C++11, to easily create transaction
	template<typename Fn>
	IsolationLevelRef operator>>(const Fn &fn) {
		return IsolationLevelRef(*this, IConnection::defaultLevel) >> fn;
	}
	///For C++11, to easily create transaction
	/**
	 * If trn is brand new transaction object, then
	 * @code
	 * trn(IConnection::readCommited) >> [&](){...code...};
	 * @endcode
	 * will run 'code' as transaction under 'readCommited' isolation level.
	 *
	 * @param level specify isolation level
	 * @return object able to run transaction
	 */
	template<typename Fn>
	IsolationLevelRef operator()(IConnection::Level level) {
		return IsolationLevelRef(*this, level);
	}

protected:
	Query& queryObj;
	natural retryCount;

	enum State {
		///transaction object is ready to start
		stReady,
		///transaction has been started
		stStarted,
		///transaction has been committed and cannot be started again
		stCommited,
		///transaction has been recovered from deadlock. commit() should have no action until start()
		stRecovered,
		///transaction has been rollbacked
		stRollbacked
	};
	State state;

};


template<typename Ret>
inline Ret Transaction::exec(const ProgramLocation &loc, Ret (*fn)(Transaction& trn)) {
	while (start()) try {
		Ret x = fn(*this);
		commit();
		return x;
	} catch (ServerError_t &e) {
		except(e,loc);
	}
	throw;
}

template<typename Ret, typename Arg1>
inline Ret Transaction::exec(const ProgramLocation &loc,
		Ret (*fn)(Transaction& trn, Arg1), Arg1 arg) {
	while (start()) try {
		Ret x = fn(*this,arg);
		commit();
		return x;
	} catch (ServerError_t &e) {
		except(e,loc);
	}
	throw;
}

template<typename Ret, typename Arg1, typename Arg2>
inline Ret Transaction::exec(const ProgramLocation &loc,
		Ret (*fn)(Transaction& trn, Arg1, Arg2), Arg1 arg1, Arg2 arg2) {
	while (start()) try {
		Ret x = fn(*this,arg1,arg2);
		commit();
		return x;
	} catch (ServerError_t &e) {
		except(e,loc);
	}
	throw;
}

template<typename Ret, typename Obj>
inline Ret Transaction::exec(const ProgramLocation &loc, Obj* obj,
		Ret (Obj::*fn)(Transaction& trn)) {
	while (start()) try {
		Ret x = (obj->*fn)(*this);
		commit();
		return x;
	} catch (ServerError_t &e) {
		except(e,loc);
	}
	throw;
}

template<typename Ret, typename Obj>
inline Ret Transaction::exec(const ProgramLocation &loc, const Obj* obj,
		Ret (Obj::*fn)(Transaction& trn)) {
	while (start()) try {
		Ret x = (obj->*fn)(*this);
		commit();
		return x;
	} catch (ServerError_t &e) {
		except(e,loc);
	}
	throw;
}

template<typename Ret, typename Obj, typename Arg1>
inline Ret Transaction::exec(const ProgramLocation &loc, Obj* obj,
		Ret (Obj::*fn)(Transaction& trn, Arg1), Arg1 arg) {
	while (start()) try {
		Ret x = (obj->*fn)(*this,arg);
		commit();
		return x;
	} catch (ServerError_t &e) {
		except(e,loc);
	}
	throw;
}

template<typename Ret, typename Obj, typename Arg1>
inline Ret Transaction::exec(const ProgramLocation &loc, const Obj* obj,
		Ret (Obj::*fn)(Transaction& trn, Arg1), Arg1 arg) {
	while (start()) try {
		Ret x = (obj->*fn)(*this,arg);
		commit();
		return x;
	} catch (ServerError_t &e) {
		except(e,loc);
	}
	throw;
}



} /* namespace LightMySQL */


#endif /* LIGHTMYSL_TRANSACTION_H_ */
