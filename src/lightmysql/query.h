/** @file
 * Copyright (c) 2006, Seznam.cz, a.s.
 * All Rights Reserved.
 * 
 * $Id: query.h 1620 2011-02-15 14:05:42Z ondrej.novak $
 *
 * DESCRIPTION
 * Short description
 * 
 * AUTHOR
 * Ondrej Novak <ondrej.novak@firma.seznam.cz>
 *
 */

#ifndef MYSQL_QUERY_T_H_
#define MYSQL_QUERY_T_H_

#pragma once

#include "iconnection.h"
#include <vector>
#include "exception.h"
#include <lightspeed/base/containers/autoArray.h>
#include "lightspeed/base/containers/constStr.h"


namespace LightMySQL {

using namespace LightSpeed;

class Result;
class SubQuery;
class CreateTableDef;


///Builds and executes query
/**
 * Object created using this class help to build and execute query.
 *
 * Query is defined as text, where variable parts are refered
 * in form "%N" (without quotes). Object automatically adds
 * quote marks to strings, perform mysql_escape to prevent SQL
 * injecting. N in "%N" is one-based index to parameter array.
 *
 * Arguments are assigned using operator <<.
 *
 * You can use object repeatedly. First use operator() to define
 * query pattern and then feed arguments using << operator. When
 * query is prepared, call exec() method to execute query.
 *
 * To repeat same query, you can start to feed arguments without
 * need to reset the query pattern
 */
class Query {
public:
	///Construct query object
	/**
	 * @param conn opened database connection
	 */
	Query(IConnection &conn);

	///Copy constructor
	/** Creates new object of query builder sharing the same connection
	 */
	Query(const Query &other);

	///Sets new query pattern
	/**
	 * @param queryText new query pattern
	 * @return reference to this object allowing to create chains
	 * @note setting query pattern removes arguments
	 */
	Query &operator()(ConstStrA queryText);


	///Executes the query
	/**
	 * @return result of query
	 */
	virtual Result exec();

	void clear();

	///feed by argument
	/** function escapes and adds quotes */
	Query &arg(ConstStrA str);
	///feed by argument
	/** function escapes and adds quotes */
	Query &arg(ConstStrW str);
	///feed by argument
	/** function escapes, but will no add quotes */
	Query &escaped(ConstStrA str);
	///feed by argument
	/** function escapes, but will no add quotes */
	Query &escaped(ConstStrW str);
	///feed by argument
	Query &arg(int i);
	///feed by argument
	Query &arg(unsigned int i);
	///feed by argument
	Query &arg(long i);
	///feed by argument
	Query &arg(unsigned long i);
	///feed by argument
	Query &arg(long long i);
	///feed by argument
	Query &arg(unsigned long long i);
	///feed by argument
	Query &arg(double val);
	///feed by name of field
	Query &field(ConstStrA field);
	Query &field(ConstStrA table, ConstStrA val);
	Query &field(ConstStrA database, ConstStrA table, ConstStrA val);
	Query &field(ConstStrW field);
	Query &field(ConstStrW table, ConstStrW val);
	Query &field(ConstStrW database, ConstStrW table, ConstStrA val);
	Query &null();
	Query &now();
	Query &curDate();
	///feed by array separated by ,
	/** Useful to construct x in [...] condition */
	template<typename T>
	Query &arg(ConstStringT<T> arr);

	///inserts inline table created as series simple selects joined with union.
	/** do not forget to alias such table. */
	template<typename T>
	Query &inline_table(ConstStringT<T> arr,ConstStrA fieldName, ConstStrA colation);

	template<typename T>
	Query &arg(ConstStringT<T> arr, ConstStrA separator) {
		return arg(arr,separator,&Query::arg);
	}

	///feed by array separated by specified separator
	template<typename T>
	Query &arg(ConstStringT<T> arr, ConstStrA separator, Query & (Query::*op)(T));


	///feed by raw string.
	/**
	 * @param text raw string
	 * @note DON'T USE, IF YOU DON'T REALLY KNOW, WHAT ARE YOU DOING
	 */
	Query &raw(ConstStrA text);

	///feed by result of sub-query
	Query &arg(const Query &other) {return raw(other.build());}

	///feed by operator <<
	template<typename T>
	Query &operator << (const T &v) {return arg(v);}

	///append part of query
	/**
	 * Allows to dynamically append parts of queries. The separator must
	 * be included in queryText. Function invokes build() command to
	 * build current query.
	 *
	 * @param queryText query pattern of part
	 * @return this object to allow chaining
	 * @note to start new query, use operator() to reset append state.
	 */
	Query &append(ConstStrA queryText);


	///Builds query, but will not execute it
	/**
	 * @return built query
	 */
	ConstStrA build() const;

	///Retrieves executed query
	/**
	 * @return returns query which has been executed. Function also
	 *  returns query that has been built by last build() function
	 */
	ConstStrA getExecutedQuery() const {return queryBuffer;}

	///Returns query pattern
	/**
	 * @return pattern
	 */
	ConstStrA  getQueryText() const {return queryText;}


	virtual ~Query() {}

	IConnection &getConnection() {return conn;}
	const IConnection &getConnection() const {return conn;}

	//builder commands

	Query &INSERT(ConstStrA pattern);
	Query &INSERT_IGNORE(ConstStrA pattern);

	Query &SET(ConstStrA field, ConstStrA expr);
	Query &SET(ConstStrA field);


	Query &UPDATE(ConstStrA pattern);

	Query &REPLACE(ConstStrA pattern);

	Query &DELETE(ConstStrA pattern);
	Query &DELETE();


	Query &SELECT(ConstStrA pattern, bool calcfoundrows = false);
	Query &SELECT_DISTINCT(ConstStrA pattern, bool calcfoundrows = false);
	Query &SELECT_DISTINCTROW(ConstStrA pattern, bool calcfoundrows = false);

	Query &FROM(ConstStrA tableName);
	Query &FROM(ConstStrA tableName, ConstStrA asTable);
	Query &FROM_pat(ConstStrA pattern);
	///FROM subquery
	SubQuery FROM_SUB(ConstStrA subqueryName);
	///FROM subquery
	SubQuery JOIN_SUB(ConstStrA subqueryName);
	///FROM subquery
	SubQuery LEFT_JOIN_SUB(ConstStrA subqueryName);

	Query &JOIN(ConstStrA table);
	Query &JOIN(ConstStrA table, ConstStrA asTable);
	Query &JOIN_pat(ConstStrA pattern);

	Query &LEFT_JOIN(ConstStrA table);
	Query &LEFT_JOIN(ConstStrA table, ConstStrA asTable);
	Query &LEFT_JOIN_pat(ConstStrA pattern);

	Query &ON(ConstStrA pattern);
	Query &ON(ConstStrA table1, ConstStrA field1,ConstStrA table2, ConstStrA field2); ///<-- ON field=field: ON().field(aa).field(bb)

	Query &WHERE(ConstStrA pattern);

	Query &ORDERBY_pat(ConstStrA pattern);

	Query &ORDERBY(ConstStrA field, bool descending);
	Query &ORDERBY(ConstStrA table, ConstStrA field, bool descending);

	Query &GROUPBY(ConstStrA field);
	Query &GROUPBY(ConstStrA table, ConstStrA field);
	Query &GROUPBY_pat(ConstStrA pattern);

	Query &HAVING(ConstStrA pattern);

	Query &ON_DUPLICATE_KEY_UPDATE(ConstStrA field, ConstStrA expr);
	Query &ON_DUPLICATE_KEY_UPDATE(ConstStrA field);
	Query &ON_DUPLICATE_KEY_UPDATE();

	Query &AND(ConstStrA pattern);

	Query &OR(ConstStrA pattern);

	Query &LIMIT(natural count);
	Query &LIMIT(natural offset,natural count);

	Query &operator,(ConstStrA pattern);

	///end of query - append ;
	/** if there is subquery, also leaves all subqueries */
	virtual Query &end();
	///enters subquery.
	/** Appends ( and all following commands are executed inside subquery. You have to call leave() to leave current query */
	SubQuery sub();
	///end of subquery - this function does nothing unless it is subquery
	virtual Query &leave();

	virtual Query &endsub();

	///leaves all subqueries
	virtual Query &leaveAll();


	Query &FOR_UPDATE();
	Query &LOCK_IN_SHARE_MODE();

	Query &VALUES(ConstStrA pattern);

	CreateTableDef CREATE_TEMPORARY_TABLE(ConstStrA tableName);


	///Depends on first argument, function supplies specified value or NULL
	/**
	 * @param notnull if true, value will be used. if false, NULL will be used
	 * @param val value
	 * @return reference to query
	 */
	template<typename T>
	Query &arg_null(bool notnull, const T &val) {
		if (notnull) return arg(val);
		else return null();
	}

protected:

	///Connection
	IConnection &conn;
	///query pattern
	AutoArray<char> queryText;
	///buffer contains parameters
	/**
	 * It used linear buffer to allow re-using allocated memory while object
	 * is used multiple-times.
	 */
	AutoArray<char> paramBuffer;
	///contains last built query
	mutable AutoArray<char>  queryBuffer;
	///contains indices into paramBuffer where each parameter ends
	AutoArray<std::size_t> paramEnds;

	std::size_t commitPos;

	Query &appendArg() {
		paramEnds.resize(paramEnds.length()-1);
		return *this;
	}

	enum CmdType {
		cmdNotSet,cmdInsert,cmdUpdate,cmdReplace,cmdSet,
		cmdDelete,cmdSelect,cmdFrom,cmdJoin,cmdLeftJoin,
		cmdOn,cmdWhere,cmdOrder,cmdGroupBy,cmdHaving,cmdList,cmdAnd,cmdOr,
		cmdOnDuplicateKeyUpdate,cmdOnDuplicateKeyUpdateBlock,cmdLimit,cmdValues
	};

	CmdType lastCmd;
	bool executed;

	bool beginCommand(CmdType cmd, ConstStrA cmdName);
	bool beginCommand(CmdType cmd, ConstStrA cmdName, ConstStrA separator);
	void appendFieldName(ConstStrA fieldName);
	void appendFieldName(ConstStrA fieldName, ConstStrA asName);

};

template<typename T>
Query &Query::arg(ConstStringT<T> arr) {
	if (arr.length() > 0) {
		typename ConstStringT<T>::Iterator iter = arr.getFwIter();
		this->arg(iter.getNext());
		while (iter.hasItems()) {
			appendArg();
			paramBuffer.add(',');
			this->arg(iter.getNext());
		}
	} else {
		this->null();
	}
	return *this;
}
template<typename T>
Query &Query::arg(ConstStringT<T> arr, ConstStrA separator, Query & (Query::*op)(T)) {
	if (arr.length() > 0) {
		typename ConstStringT<T>::Iterator iter = arr.getFwIter();
		(this->*op)(iter.getNext());
		while (iter.hasItems()) {
			appendArg();
			paramBuffer.append(separator);
			(this->*op)(iter.getNext());
		}
	} else {
		this->null();
	}
return *this;
}

template<typename T>
Query &Query::inline_table(ConstStringT<T> arr, ConstStrA fieldName, ConstStrA colation) {
	if (arr.length() > 0) {
		typename ConstStringT<T>::Iterator iter = arr.getFwIter();
		this->raw("SELECT ").appendArg().arg(iter.getNext()).appendArg().raw(" COLLATE ")
				.appendArg().raw(colation).appendArg().raw(" AS ")
				.appendArg().field(fieldName);
		while (iter.hasItems()) {
			appendArg().raw(" UNION SELECT ").appendArg().arg(iter.getNext());
		}
	} else {
		this->raw("SELECT 1 AS ")
				.appendArg().field(fieldName)
				.appendArg().raw(" LIMIT 0");

	}
	return *this;

}



///Exception - Error during building query
/** Exception is thrown, when Query_t object cannot build the query
 * because not all arguments has been defined.
 */
class UnassignedQueryParameterException_t: public Exception_t {
public:

	///Constructor
	/**
	 * @param loc program location
	 * @param query query pattern
	 * @param pos index of which argument is not defined
	 */
	UnassignedQueryParameterException_t(const ProgramLocation &loc,
			const StringA query, std::size_t pos)
		:Exception_t(loc),query(query),pos(pos) {}

	///dtor
	virtual ~UnassignedQueryParameterException_t() throw() {}
	LIGHTSPEED_EXCEPTIONFINAL;

	///Retrieves the query pattern
	const StringA &getQuery() const {return query;}
	///Retrieves the position
	std::size_t getParamPos() const {return pos;}

protected:
	StringA query;
	std::size_t pos;

	void message(LightSpeed::ExceptionMsg &msg) const;
};

///Temporary object created during building a query. It introduces braces into query. see Query::sub()
class SubQuery: public Query {
public:
	///inicializes subquery and appends ( to query
	SubQuery(Query &q);
	///initializes subquery with suffix (FROM (subquery) name
	/** Note suffix string MUST exists when subquery is leaved
	 */
	SubQuery(Query &q, ConstStrA suffix);
	///leaves subquery, if it is not leaved yet
	~SubQuery();
	///executes query ending all subqueries
	virtual Result exec();
	///leaves query appends ) to a query
	Query &leave();
	///alias to leave
	Query &endsub();
	///ends query leaving all subqueries
	Query &end();
	///leaves all subqueries
	virtual Query &leaveAll();

protected:
	Query &parent;
	ConstStrA suffix;
	mutable bool active;

};





///Temporary class created during field definition of Create Table
class CreateTableDef: public SubQuery {
public:
	CreateTableDef(Query &q);

	CreateTableDef &BIT(ConstStrA name);
	CreateTableDef &TINYINT(ConstStrA name);
	CreateTableDef &SMALLINT(ConstStrA name);
	CreateTableDef &MEDIUMINT(ConstStrA name);
	CreateTableDef &INT(ConstStrA name);
	CreateTableDef &BIGINT(ConstStrA name);
	CreateTableDef &REAL(ConstStrA name);
	CreateTableDef &DOUBLE(ConstStrA name);
	CreateTableDef &FLOAT(ConstStrA name);
	CreateTableDef &DECIMAL(ConstStrA name, natural length, natural decimals);
	CreateTableDef &NUMERIC(ConstStrA name, natural length, natural decimals);
	CreateTableDef &UNSIGNED_BIT(ConstStrA name);
	CreateTableDef &UNSIGNED_TINYINT(ConstStrA name);
	CreateTableDef &UNSIGNED_SMALLINT(ConstStrA name);
	CreateTableDef &UNSIGNED_MEDIUMINT(ConstStrA name);
	CreateTableDef &UNSIGNED_INT(ConstStrA name);
	CreateTableDef &UNSIGNED_BIGINT(ConstStrA name);
	CreateTableDef &UNSIGNED_REAL(ConstStrA name);
	CreateTableDef &UNSIGNED_DOUBLE(ConstStrA name);
	CreateTableDef &UNSIGNED_FLOAT(ConstStrA name);
	CreateTableDef &UNSIGNED_DECIMAL(ConstStrA name, natural length, natural decimals);
	CreateTableDef &UNSIGNED_NUMERIC(ConstStrA name, natural length, natural decimals);
	CreateTableDef &DATE(ConstStrA name);
	CreateTableDef &TIME(ConstStrA name);
	CreateTableDef &TIMESTAMP(ConstStrA name);
	CreateTableDef &DATETIME(ConstStrA name);
	CreateTableDef &YEAR(ConstStrA name);
	CreateTableDef &CHAR(ConstStrA name, natural length);
	CreateTableDef &VARCHAR(ConstStrA name, natural length);
	CreateTableDef &BINARY(ConstStrA name, natural length);
	CreateTableDef &VARBINARY(ConstStrA name, natural length);
	CreateTableDef &TINYBLOB(ConstStrA name);
	CreateTableDef &BLOB(ConstStrA name);
	CreateTableDef &MEDIUMBLOB(ConstStrA name);
	CreateTableDef &LONGBLOB(ConstStrA name);
	CreateTableDef &TINYTEXT(ConstStrA name);
	CreateTableDef &TEXT(ConstStrA name);
	CreateTableDef &MEDIUMTEXT(ConstStrA name);
	CreateTableDef &LONGTEXT(ConstStrA name);
	CreateTableDef &SET(ConstStrA name);
	CreateTableDef &ENUM(ConstStrA name);
	///Value for SET or ENUM. It is terminated by another field definition
	CreateTableDef &val(ConstStrA name);
	///Sets charset of previous text field
	CreateTableDef &charset(ConstStrA charset);
	///Sets collation of previous text field
	CreateTableDef &collate(ConstStrA collate);
	///Allows previous field to be set to NULL
	CreateTableDef &null();
	///Disallow previous field to be set to NULL
	CreateTableDef &not_null();
	///Specify default value. Use arg() to specify value
	CreateTableDef &DEFAULT();
	///Definition of autoincrement integer
	CreateTableDef &AUTO_INCREMENT_INT(ConstStrA name);
	///Definition of autoincrement big integer
	CreateTableDef &AUTO_INCREMENT_BIGINT(ConstStrA name);
	///Definition of autoincrement unsigned integer
	CreateTableDef &AUTO_INCREMENT_UNSIGNED_INT(ConstStrA name);
	///Definition of autoincrement unsigned big integer
	CreateTableDef &AUTO_INCREMENT_UNSIGNED_BIGINT(ConstStrA name);
	///Definition of primary key. Use col() to specify columns
	CreateTableDef &PRIMARY_KEY();
	///Definition of index. Use col() to specify columns
	CreateTableDef &INDEX();
	///Definition of unique key. Use col() to specify columns
	CreateTableDef &UNIQUE();
	///Include column name into previous index defintion. Terminates by another definition
	CreateTableDef &col(ConstStrA name);
	///Definition foreing key. Use col to specify columns. It must be followed by REFERENCES()
	CreateTableDef &FOREIGN_KEY();
	///Definition foreing key with cascade deletion. Use col to specify columns. It must be followed by REFERENCES()
	CreateTableDef &FOREIGN_KEY_CASCADE();
	///Definition foreing key with cascade update. Use col to specify columns. It must be followed by REFERENCES()
	CreateTableDef &FOREIGN_KEY_UPDATABLE();
	///Definition foreing key with cascade deletion and update. Use col to specify columns. It must be followed by REFERENCES()
	CreateTableDef &FOREIGN_KEY_CASCADE_UPDATABLE();
	///Definition of reference table of previous foreign key
	CreateTableDef &REFERENCES(ConstStrA name);

	Query &leave();

protected:
	ConstStrA sep;
	ConstStrA frend;
	ConstStrA fieldsep;

	void addSep();
	CreateTableDef &fieldDef(ConstStrA type, ConstStrA fieldName);
	CreateTableDef &fieldDef(ConstStrA type, ConstStrA fieldName, natural len);
	CreateTableDef &fieldDef(ConstStrA type, ConstStrA fieldName, natural len, natural decm);
	CreateTableDef &append(ConstStrA text) {Query::append(text);return *this;}

};


///Exception - Query_t has been requested to execute empty query
class EmptyQueryException_t: public Exception_t {
public:

	EmptyQueryException_t(const ProgramLocation &loc)
		:Exception_t(loc) {}
	LIGHTSPEED_EXCEPTIONFINAL;
protected:
	void message(LightSpeed::ExceptionMsg &msg) const;
};

}



#endif /* MYSQL_QUERY_T_H_ */
