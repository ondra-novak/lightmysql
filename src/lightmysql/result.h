/** @file
 * Copyright (c) 2006, Seznam.cz, a.s.
 * All Rights Reserved.
 * 
 * $Id: result.h 1549 2011-01-03 12:19:08Z ondrej.novak $
 *
 * DESCRIPTION
 * Short description
 * 
 * AUTHOR
 * Ondrej Novak <ondrej.novak@firma.seznam.cz>
 *
 */

#ifndef MYSQL_RESULT_H_
#define MYSQL_RESULT_H_

#pragma once

#include <list>
#include "connection.h"
#include "exception.h"
#include "row.h"

namespace LightMySQL {


///Object allows to access mysql result or multiple-results
/**
 * Object is used as result container, where all results are stored
 * in the memory. Whole result is stored into the memory, so
 * you are able to work with result independently to current
 * connection state.
 *
 * Object that contains the result can be copied and returned as
 * return value from the function. Every new instance of result
 * shares the stored data, but has own reading position
 *
 * @see hasResult, nextResult, hasItems, getNext, Result_t::Row
 */
class Result: public LightSpeed::SharedResource {


	class ResultInfo: public LightSpeed::SharedResource {
	public:
		MYSQL_RES *resPtr;
		my_ulonglong affected;
		my_ulonglong lastId;
		natural warnings;
		natural fieldCount;
		StringA error;
		natural myerrno;

		ResultInfo()
		:resPtr(0),affected(0),lastId(0),warnings(0),fieldCount(0),myerrno(0) {}
		~ResultInfo();
	};

	typedef std::list<ResultInfo> ResultList_t;
	friend class Connection;

public:

	///Destructor - result is released when there is no instance sharing the result data
	virtual ~Result();

	typedef LightMySQL::Row Row;


	///Tests, whether object has result ready
	/**
	 * @retval true result is available. You can use all functions
	 * 	working with result. This state can be changed by function
	 *  nextResult()
	 * @retval false result is not available
	 *
	 * @note In case, that result is not table, function still
	 * reports true. You can use functions getAffectedRows() and
	 * getInsertId() to retrieve informations about executed
	 * query.
	 */
	bool hasResult() const;

	///Finishes current result and moves to the next-one
	/**
	 * @exception NoMoreResultsException_t if you try to call this
	 * function after hasResult() returns false
	 *
	 * Example of usage
	 * 		for(Result_t res = query.exec(); res.hasResult();res.nextResult()) {...}
	 */
	void nextResult();

	///skips more then one result
	/**
	 * @param count how many results skip. When 1 is specified, function works similar as nextResult(). When 0 specified, nothing happened
	 */
	void nextResult(natural count);

	///Moves result iterator to thr first result in the collection
	void firstResult();

	///Asks whether there are unprocessed rows in the result
	/**
	 * @retval true there are unprocessed rows
	 * @retval false all rows has been processed
	 *	 * @note function can cause, that previously returned row becomes
	 * invalid. Never call this function before you finish current row
	 */
	bool hasItems() const;

	///Retrieves next row, if available
	/**
	 * @return row object
	 * @exception IteratorNoMoreItems if there are no more rows. Use
	 * hasItems() to check, whether there are rows without exception
	 */
	Row getNext();

	///Moves internal pointer to the first row
	void rewind();

	///Retrieves count of affected rows for current result
	/**
	 * @return count of affected rows
	 * @note in contrast to the mysql documentation, you don't
	 * need to check this value immediately after query is
	 * executed. Value is stored the Result_t object and
	 * can be retrieved any-time later
	 */
	my_ulonglong getAffectedRows() const;
	///Retrieves last insert id for current result
	/**
	 * @return insert id of query that generated this result
	 * @note in contrast to the mysql documentation, you don't
	 * need to check this value immediately after query is
	 * executed. Value is stored the Result_t object and
	 * can be retrieved any-time later
	 */
	my_ulonglong getInsertId() const;
	///Retrieves warining count for the query
	/**
	 * @return warining count for the query
	 * @note in contrast to the mysql documentation, you don't
	 * need to check this value immediately after query is
	 * executed. Value is stored the Result_t object and
	 * can be retrieved any-time later
	 */
	natural getWarningCount() const;
	///Retrieves field count for the query
	/**
	 * @return warining count for the query
	 * @note in contrast to the mysql documentation, you don't
	 * need to check this value immediately after query is
	 * executed. Value is stored the Result_t object and
	 * can be retrieved any-time later
	 */
	natural getFieldCount() const;

	///Retrieves error string, if query has been executed with error
	/**
	 * @return text of query
	 */
	const StringA &getError() const;

	///Retrieves error number of query that generated this result
	/**
	 * @return number of error
	 */
	natural getErrNo() const;

	///Tests, whether result is error
	/**
	 * @retval true query executed with error
	 * @retval false query executed normaly
	 *
	 * @note Other than error-functions will throw exception containing
	 * error description
	 */
	bool isError() const {return getErrNo() != 0;}

	///Throws exception, if result is in error state
	/**
	 * @note function tests error state. If result is not in the
	 * error state, function does nothing
	 */
	void throwErrorException() const;

	void throwErrorException(const ProgramLocation &loc) const;

	///Retrieves offset to the current row in the current result
	MYSQL_ROW_OFFSET tell() const;
	///Seeks to the current result
	void seek(MYSQL_ROW_OFFSET rc);

	///returns true, there is no table result
	/**
	 * @retval true no result
	 * @retval false some result (including empty)
	 */
	bool noTableResult() const;

	///return true, if there is noResult() or result has no rows
	bool empty() const;

	///returns count of fields in the result
	natural countFields() const;
	///returns count of rpws in the result
	natural countRows() const;
	///returns field name under given index
	/**
	 * @param i zero-based index
	 * @return field name
	 */
	ConstStrA getFieldName(natural i) const;

	///returns MYSQL field information of field at given index
	/**
	 * @param i zero-based index
	 * @return field information
	 */
	const MYSQL_FIELD *getFieldInfo(natural i) const;

	class RangeIter {
	public:

		RangeIter(Result &owner,  bool isend);
		bool operator==(const RangeIter &other) const;
		bool operator!=(const RangeIter &other) const;
		Row operator *() const;
		RangeIter &operator++();

	protected:
		Result &owner;
		bool isend;
	};

	///Allows to iterate through to result
	/**
	 * @return return iterator initialized to the first item
	 * @note for compatibility reason, it is allowed to have only one iterator active at time (it excludes
	 * iterator to the end). The function begin() also rewinds the result to the begining
	 *
	 */
	RangeIter begin();
	///Returns iterator to the end
	/** You cannot dereference the iterator, it just marks end of result */
	RangeIter end();


protected:
	///protected ctor
	Result(MYSQL &sql, IDebugLog *logObject);


	void freeResult();
	void initResult();
	void loadNextRow() const;

	void checkResult(const ProgramLocation &loc) const;

	///row prepared to read
	mutable MYSQL_ROW readyRow;
	///offset of first row (for rewind() function)
	MYSQL_ROW_OFFSET firstRow;
	///list of results
	ResultList_t *result;
	///current result
	ResultList_t::iterator cur;

};

///Exception - Abstract exception object for all exception that refers a field in the result
class FieldException_t: public Exception_t {
public:
	FieldException_t(const ProgramLocation &loc, const StringA &fieldName)
		:Exception_t(loc),fieldName(fieldName) {}

	virtual ~FieldException_t() throw() {}
	///Retrieves field name
	const StringA &getFieldName() const {return fieldName;}
protected:

	StringA fieldName;

};

///Exception - thrown, Result_t is requested for field, which is not exists
class FieldNotFoundException_t: public FieldException_t {
public:

	LIGHTSPEED_EXCEPTIONFINAL;
	FieldNotFoundException_t(const ProgramLocation &loc, const StringA &fieldName)
		:FieldException_t(loc,fieldName) {}

protected:
	void message(LightSpeed::ExceptionMsg &e) const;

};

///Exception - try to read field with NULL value
/**
 * Field with NULL value cannot be read. You can detect NULL field by calling
 *   Result_t::getNull() function. Test the field for NULL, before you read the field.
 */
class NullFieldException_t: public FieldException_t {
public:
	LIGHTSPEED_EXCEPTIONFINAL;

	NullFieldException_t(const ProgramLocation &loc, const StringA &fieldName)
		:FieldException_t(loc,fieldName) {}
protected:
	void message(LightSpeed::ExceptionMsg &e) const;
};

}

#endif /* MYSQL_RESULT_H_ */
