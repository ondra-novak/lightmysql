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


namespace LightMySQL {


using json::StringView;
using json::StrViewA;
using json::StrViewW;
using json::BinaryView;
using json::String;

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
	Query &operator()(StrViewA queryText);


	///Executes the query
	/**
	 * @return result of query
	 */
	virtual Result exec();

	void clear();

	///feed by argument
	/** function escapes and adds quotes */
	Query &arg(StrViewA str);
	///feed by argument
	/** function escapes and adds quotes */
	Query &arg(StrViewW str);
	///feed by argument
	/** function escapes and adds quotes */
	Query &arg(BinaryView str);
	///feed by argument
	/** function escapes, but will no add quotes */
	Query &escaped(StrViewA str);
	///feed by argument
	/** function escapes, but will no add quotes */
	Query &escaped(StrViewW str);
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
	///insert multiple values
	template<typename H, typename T>
	Query &arg(const std::pair<H,T> &val);
	///feed by name of field
	Query &field(StrViewA field);
	Query &field(StrViewA table, StrViewA val);
	Query &field(StrViewA database, StrViewA table, StrViewA val);
	Query &field(StrViewW field);
	Query &field(StrViewW table, StrViewW val);
	Query &field(StrViewW database, StrViewW table, StrViewA val);
	Query &null();
	Query &now();
	Query &curDate();
	///feed by array separated by ,
	/** Useful to construct x in [...] condition */
	template<typename T>
	Query &arg(StringView<T> arr);

	///inserts inline table created as series simple selects joined with union.
	/** do not forget to alias such table. */
	template<typename T>
	Query &inline_table(StringView<T> arr,StrViewA fieldName, StrViewA colation);

	template<typename T>
	Query &arg(StringView<T> arr, StrViewA separator) {
		return arg(arr,separator,&Query::arg);
	}

	///feed by array separated by specified separator
	/**
	 * @param arr array to use
	 * @param separator separator between items
	 * @param fn conversion function. It have to convert item to some known item
	 * @return this object
	 */
	template<typename T, typename Fn>
	Query &arg(StringView<T> arr, StrViewA separator, Fn fn);


	///feed by raw string.
	/**
	 * @param text raw string
	 * @note DON'T USE, IF YOU DON'T REALLY KNOW, WHAT ARE YOU DOING
	 */
	Query &raw(StrViewA text);

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
	Query &append(StrViewA queryText);


	///Builds query, but will not execute it
	/**
	 * @return built query
	 */
	StrViewA build() const;

	///Retrieves executed query
	/**
	 * @return returns query which has been executed. Function also
	 *  returns query that has been built by last build() function
	 */
	StrViewA getExecutedQuery() const {return queryBuffer;}

	///Returns query pattern
	/**
	 * @return pattern
	 */
	StrViewA  getQueryText() const {return queryText;}


	virtual ~Query() {}

	IConnection &getConnection() {return conn;}
	const IConnection &getConnection() const {return conn;}

	//builder commands

	Query &INSERT(StrViewA pattern);
	Query &INSERT_IGNORE(StrViewA pattern);

	Query &SET(StrViewA field, StrViewA expr);
	Query &SET(StrViewA field);


	Query &UPDATE(StrViewA pattern);

	Query &REPLACE(StrViewA pattern);

	Query &DELETE(StrViewA pattern);
	Query &DELETE();


	Query &SELECT(StrViewA pattern, bool calcfoundrows = false);
	Query &SELECT_DISTINCT(StrViewA pattern, bool calcfoundrows = false);
	Query &SELECT_DISTINCTROW(StrViewA pattern, bool calcfoundrows = false);

	Query &FROM(StrViewA tableName);
	Query &FROM(StrViewA tableName, StrViewA asTable);
	Query &FROM_pat(StrViewA pattern);
	///FROM subquery
	SubQuery FROM_SUB(StrViewA subqueryName);
	///FROM subquery
	SubQuery JOIN_SUB(StrViewA subqueryName);
	///FROM subquery
	SubQuery LEFT_JOIN_SUB(StrViewA subqueryName);

	Query &JOIN(StrViewA table);
	Query &JOIN(StrViewA table, StrViewA asTable);
	Query &JOIN_pat(StrViewA pattern);

	Query &LEFT_JOIN(StrViewA table);
	Query &LEFT_JOIN(StrViewA table, StrViewA asTable);
	Query &LEFT_JOIN_pat(StrViewA pattern);

	Query &ON(StrViewA pattern);
	Query &ON(StrViewA table1, StrViewA field1,StrViewA table2, StrViewA field2); ///<-- ON field=field: ON().field(aa).field(bb)

	Query &WHERE(StrViewA pattern);

	Query &ORDERBY_pat(StrViewA pattern);

	Query &ORDERBY(StrViewA field, bool descending);
	Query &ORDERBY(StrViewA table, StrViewA field, bool descending);

	Query &GROUPBY(StrViewA field);
	Query &GROUPBY(StrViewA table, StrViewA field);
	Query &GROUPBY_pat(StrViewA pattern);

	Query &HAVING(StrViewA pattern);

	Query &ON_DUPLICATE_KEY_UPDATE(StrViewA field, StrViewA expr);
	Query &ON_DUPLICATE_KEY_UPDATE(StrViewA field);
	Query &ON_DUPLICATE_KEY_UPDATE();

	Query &AND(StrViewA pattern);

	Query &OR(StrViewA pattern);

	Query &LIMIT(unsigned int count);
	Query &LIMIT(unsigned int offset,unsigned int count);

	Query &operator,(StrViewA pattern);

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

	Query &VALUES(StrViewA pattern);

	CreateTableDef CREATE_TEMPORARY_TABLE(StrViewA tableName);


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
	std::vector<char> queryText;
	///buffer contains parameters
	/**
	 * It used linear buffer to allow re-using allocated memory while object
	 * is used multiple-times.
	 */
	std::vector<char> paramBuffer;
	///contains last built query
	mutable std::vector<char>  queryBuffer;
	///contains indices into paramBuffer where each parameter ends
	std::vector<std::size_t> paramEnds;

	std::size_t commitPos;

	Query &appendArg() {
		paramEnds.resize(paramEnds.size()-1);
		return *this;
	}
	Query &finishArg() {
		paramEnds.push_back(paramBuffer.size());
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
	int pairlevel;

	bool beginCommand(CmdType cmd, StrViewA cmdName);
	bool beginCommand(CmdType cmd, StrViewA cmdName, StrViewA separator);
	void appendFieldName(StrViewA fieldName);
	void appendFieldName(StrViewA fieldName, StrViewA asName);


};

template<typename T>
Query &Query::arg(StringView<T> arr) {
	if (arr.length() > 0) {
		typename StringView<T>::Iterator iter = arr.getFwIter();
		this->arg(iter.getNext());
		while (iter.hasItems()) {
			appendArg();
			paramBuffer.push_back(',');
			this->arg(iter.getNext());
		}
	} else {
		this->null();
	}
	return *this;
}
template<typename T, typename Fn>
	Query &Query::arg(StringView<T> arr, StrViewA separator, Fn fn) {
	if (arr.length() > 0) {
		typename StringView<T>::Iterator iter = arr.getFwIter();
		arg(fn(iter.getNext()));
		while (iter.hasItems()) {
			appendArg();
			paramBuffer.reserve(paramBuffer.size()+separator.length);
			for (auto &&c : separator)
				paramBuffer.push_back(c);
			arg(fn(iter.getNext()));
		}
	} else {
		this->null();
	}
return *this;
}

template<typename T>
Query &Query::inline_table(StringView<T> arr, StrViewA fieldName, StrViewA colation) {
	if (arr.length() > 0) {
		typename StringView<T>::Iterator iter = arr.getFwIter();
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

template<typename H, typename T>
Query &Query::arg(const std::pair<H,T> &val) {
	if (pairlevel == 0) {
		paramBuffer.push_back('(');
	}
	pairlevel++;
	arg(val.first);
	appendArg();
	paramBuffer.push_back(',');
	arg(val.second);
	pairlevel--;
	if (pairlevel == 0) {
		appendArg();
		paramBuffer.push_back(')');
		finishArg();
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
	UnassignedQueryParameterException_t(
			const String query, std::size_t pos)
		:query(query),pos(pos) {}

	///dtor
	virtual ~UnassignedQueryParameterException_t() throw() {}

	///Retrieves the query pattern
	const String &getQuery() const {return query;}
	///Retrieves the position
	std::size_t getParamPos() const {return pos;}

protected:
	String query;
	std::size_t pos;

	String getMessage() const throw() override;
};

///Temporary object created during building a query. It introduces braces into query. see Query::sub()
class SubQuery: public Query {
public:
	///inicializes subquery and appends ( to query
	SubQuery(Query &q);
	///initializes subquery with suffix (FROM (subquery) name
	/** Note suffix string MUST exists when subquery is leaved
	 */
	SubQuery(Query &q, StrViewA suffix);
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
	StrViewA suffix;
	mutable bool active;

};





///Temporary class created during field definition of Create Table
class CreateTableDef: public SubQuery {
public:
	CreateTableDef(Query &q);

	CreateTableDef &BIT(StrViewA name);
	CreateTableDef &TINYINT(StrViewA name);
	CreateTableDef &SMALLINT(StrViewA name);
	CreateTableDef &MEDIUMINT(StrViewA name);
	CreateTableDef &INT(StrViewA name);
	CreateTableDef &BIGINT(StrViewA name);
	CreateTableDef &REAL(StrViewA name);
	CreateTableDef &DOUBLE(StrViewA name);
	CreateTableDef &FLOAT(StrViewA name);
	CreateTableDef &DECIMAL(StrViewA name, unsigned int length, unsigned int decimals);
	CreateTableDef &NUMERIC(StrViewA name, unsigned int length, unsigned int decimals);
	CreateTableDef &UNSIGNED_BIT(StrViewA name);
	CreateTableDef &UNSIGNED_TINYINT(StrViewA name);
	CreateTableDef &UNSIGNED_SMALLINT(StrViewA name);
	CreateTableDef &UNSIGNED_MEDIUMINT(StrViewA name);
	CreateTableDef &UNSIGNED_INT(StrViewA name);
	CreateTableDef &UNSIGNED_BIGINT(StrViewA name);
	CreateTableDef &UNSIGNED_REAL(StrViewA name);
	CreateTableDef &UNSIGNED_DOUBLE(StrViewA name);
	CreateTableDef &UNSIGNED_FLOAT(StrViewA name);
	CreateTableDef &UNSIGNED_DECIMAL(StrViewA name, unsigned int length, unsigned int decimals);
	CreateTableDef &UNSIGNED_NUMERIC(StrViewA name, unsigned int length, unsigned int decimals);
	CreateTableDef &DATE(StrViewA name);
	CreateTableDef &TIME(StrViewA name);
	CreateTableDef &TIMESTAMP(StrViewA name);
	CreateTableDef &DATETIME(StrViewA name);
	CreateTableDef &YEAR(StrViewA name);
	CreateTableDef &CHAR(StrViewA name, unsigned int length);
	CreateTableDef &VARCHAR(StrViewA name, unsigned int length);
	CreateTableDef &BINARY(StrViewA name, unsigned int length);
	CreateTableDef &VARBINARY(StrViewA name, unsigned int length);
	CreateTableDef &TINYBLOB(StrViewA name);
	CreateTableDef &BLOB(StrViewA name);
	CreateTableDef &MEDIUMBLOB(StrViewA name);
	CreateTableDef &LONGBLOB(StrViewA name);
	CreateTableDef &TINYTEXT(StrViewA name);
	CreateTableDef &TEXT(StrViewA name);
	CreateTableDef &MEDIUMTEXT(StrViewA name);
	CreateTableDef &LONGTEXT(StrViewA name);
	CreateTableDef &SET(StrViewA name);
	CreateTableDef &ENUM(StrViewA name);
	///Value for SET or ENUM. It is terminated by another field definition
	CreateTableDef &val(StrViewA name);
	///Sets charset of previous text field
	CreateTableDef &charset(StrViewA charset);
	///Sets collation of previous text field
	CreateTableDef &collate(StrViewA collate);
	///Allows previous field to be set to NULL
	CreateTableDef &null();
	///Disallow previous field to be set to NULL
	CreateTableDef &not_null();
	///Specify default value. Use arg() to specify value
	CreateTableDef &DEFAULT();
	///Definition of autoincrement integer
	CreateTableDef &AUTO_INCREMENT_INT(StrViewA name);
	///Definition of autoincrement big integer
	CreateTableDef &AUTO_INCREMENT_BIGINT(StrViewA name);
	///Definition of autoincrement unsigned integer
	CreateTableDef &AUTO_INCREMENT_UNSIGNED_INT(StrViewA name);
	///Definition of autoincrement unsigned big integer
	CreateTableDef &AUTO_INCREMENT_UNSIGNED_BIGINT(StrViewA name);
	///Definition of primary key. Use col() to specify columns
	CreateTableDef &PRIMARY_KEY();
	///Definition of index. Use col() to specify columns
	CreateTableDef &INDEX();
	///Definition of unique key. Use col() to specify columns
	CreateTableDef &UNIQUE();
	///Include column name into previous index defintion. Terminates by another definition
	CreateTableDef &col(StrViewA name);
	///Definition foreing key. Use col to specify columns. It must be followed by REFERENCES()
	CreateTableDef &FOREIGN_KEY();
	///Definition foreing key with cascade deletion. Use col to specify columns. It must be followed by REFERENCES()
	CreateTableDef &FOREIGN_KEY_CASCADE();
	///Definition foreing key with cascade update. Use col to specify columns. It must be followed by REFERENCES()
	CreateTableDef &FOREIGN_KEY_UPDATABLE();
	///Definition foreing key with cascade deletion and update. Use col to specify columns. It must be followed by REFERENCES()
	CreateTableDef &FOREIGN_KEY_CASCADE_UPDATABLE();
	///Definition of reference table of previous foreign key
	CreateTableDef &REFERENCES(StrViewA name);

	Query &leave();

protected:
	StrViewA sep;
	StrViewA frend;
	StrViewA fieldsep;

	void addSep();
	CreateTableDef &fieldDef(StrViewA type, StrViewA fieldName);
	CreateTableDef &fieldDef(StrViewA type, StrViewA fieldName, unsigned int len);
	CreateTableDef &fieldDef(StrViewA type, StrViewA fieldName, unsigned int len, unsigned int decm);
	CreateTableDef &append(StrViewA text) {Query::append(text);return *this;}

};


///Exception - Query_t has been requested to execute empty query
class EmptyQueryException_t: public Exception_t {
public:

protected:
	String getMessage() const throw() override;
};

}



#endif /* MYSQL_QUERY_T_H_ */
