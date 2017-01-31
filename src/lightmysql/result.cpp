/** @file
 * Copyright (c) 2006, Seznam.cz, a.s.
 * All Rights Reserved.
 * 
 * $Id: result.cc 1723 2011-04-28 12:12:29Z ondrej.novak $
 *
 * DESCRIPTION
 * Short description
 * 
 * AUTHOR
 * Ondrej Novak <ondrej.novak@firma.seznam.cz>
 *
 */

#include "result.h"
#include <string.h>
#include <lightspeed/base/exceptions/throws.tcc>
//#include <lightspeed/base/text/textOut.tcc>
#include <memory>

namespace LightMySQL {


Result::~Result() {
	//delete result list if no longer shared
	if (!this->isShared()) {
		delete result;
	}
}

Result::ResultInfo::~ResultInfo() {
	if (!isShared())
		//free mysql result, if not shared
		if (resPtr) mysql_free_result(resPtr);
}


Result::Result(MYSQL & sql, IDebugLog *logObject)
{
	//keep in auto-ptr to release this when exception
	std::auto_ptr<ResultList_t> tmp(new ResultList_t);
	//initialize result variable
	result = tmp.get();
	bool nextRes = true;
	while (nextRes) {

		ResultInfo nfo;

		MYSQL_RES *res = mysql_store_result(&sql);
		if (res == 0) {

			if ((nfo.myerrno = mysql_errno(&sql))!=0) {
				nfo.error = mysql_error(&sql);
				if (logObject) logObject->onQueryError(nfo.error);
			} else if ((nfo.fieldCount = mysql_field_count(&sql)) == 0) {
				nfo.affected = mysql_affected_rows(&sql);
				nfo.lastId = mysql_insert_id(&sql);
				if (logObject) {
					const char *info = mysql_info(&sql);
					if (info) logObject->onQueryInfo(info);
					logObject->onQueryResult(0,0,nfo.affected,nfo.lastId,mysql_warning_count(&sql));
				}
			}
		} else {
			nfo.resPtr = res;
			nfo.fieldCount = mysql_field_count(&sql);
			if (logObject) {
				const char *info = mysql_info(&sql);
				if (info) logObject->onQueryInfo(info);
				logObject->onQueryResult(mysql_num_rows(res),nfo.fieldCount,0,0,mysql_warning_count(&sql));
			}
		}

		nfo.warnings = mysql_warning_count(&sql);
		result->push_back(nfo);

		nextRes = mysql_more_results(&sql) != 0;
		if (nextRes) {
			if (mysql_next_result(&sql) != 0) {
				if (logObject) logObject->onQueryError(nfo.error);
				throw ServerError_t(THISLOCATION,mysql_errno(&sql), mysql_error(&sql));
			}
		}
	}

	cur = result->begin();
	initResult();
	if (result->size() == 1 && cur->myerrno != 0)
		throw ServerError_t(THISLOCATION,cur->myerrno,cur->error);
	//release the auto_ptr - object is constructed, it will track ownership
	tmp.release();
}

void Result::freeResult()
{
	for (ResultList_t::iterator iter = result->begin();
			iter != result->end();iter++)
		mysql_free_result(iter->resPtr);
	result->clear();
}


void Result::initResult() {
	if (cur != result->end() && cur->resPtr) {
		firstRow = mysql_row_tell(cur->resPtr);
	}
	readyRow = 0;
}

bool Result::hasResult() const {
	return (cur != result->end());
}
void Result::nextResult() {
	if (hasResult()) ++cur;
	else LightSpeed::throwIteratorNoMoreItems(THISLOCATION,typeid(Result));
	initResult();
}

void Result::nextResult(natural count) {
	while (count--) nextResult();
}

void Result::firstResult() {
	cur = result->begin();
	initResult();
}

bool Result::hasItems() const {
	if (readyRow == 0) loadNextRow();
	return readyRow != 0;
}
Result::Row Result::getNext() {
	if (!hasItems())
		LightSpeed::throwIteratorNoMoreItems(THISLOCATION,typeid(Row));
	class Unset_t {
	public:
		Unset_t(MYSQL_ROW *row):row(row) {}
		~Unset_t() {*row=0;}
	protected:
		MYSQL_ROW  *row;
	};

	Unset_t unset(&readyRow);
	return Row(*this,readyRow,mysql_fetch_lengths(cur->resPtr),mysql_num_fields(cur->resPtr));
}

void Result::loadNextRow() const {
	checkResult(THISLOCATION);
	if (cur->resPtr) {
		readyRow = mysql_fetch_row(cur->resPtr);
	} else {
		readyRow = 0;
	}
}

my_ulonglong Result::getAffectedRows() const {
	checkResult(THISLOCATION);
	return cur->affected;
}
my_ulonglong Result::getInsertId() const {
	checkResult(THISLOCATION);
	return cur->lastId;
}
natural Result::getWarningCount() const {
	checkResult(THISLOCATION);
	return cur->warnings;
}
natural Result::getFieldCount() const {
	checkResult(THISLOCATION);
	return cur->fieldCount;
}



void Result::rewind() {
	seek(firstRow);
}

const StringA &Result::getError() const {
	if (!hasResult())
		LightSpeed::throwIteratorNoMoreItems(THISLOCATION,typeid(Result));
	return cur->error;
}

natural Result::getErrNo() const {
	if (!hasResult())
		LightSpeed::throwIteratorNoMoreItems(THISLOCATION,typeid(Result));
	return cur->myerrno;
}

void Result::throwErrorException() const {
	if (isError()) throwErrorException(THISLOCATION);
}

void Result::throwErrorException(const ProgramLocation &loc) const {
	if (isError()) throw ServerError_t(loc, cur->myerrno, cur->error);
}

MYSQL_ROW_OFFSET Result::tell() const {
	if (empty()) return 0;
	else return mysql_row_tell(cur->resPtr);
}

void Result::seek(MYSQL_ROW_OFFSET rc) {
	if (!empty() && rc != 0)
		mysql_row_seek(cur->resPtr,rc);

}

void Result::checkResult(const ProgramLocation &loc) const {
	if (!hasResult())
		LightSpeed::throwIteratorNoMoreItems(loc,typeid(Result));
	throwErrorException(loc);
}
bool Result::noTableResult() const {
	checkResult(THISLOCATION);
	return cur->resPtr == 0;
}

bool Result::empty() const {
	return countRows() == 0;
}

std::size_t Result::countFields() const {
	if (noTableResult()) return 0;
	else return mysql_num_fields(cur->resPtr);
}

std::size_t Result::countRows() const {
	if (noTableResult()) return 0;
	else return mysql_num_rows(cur->resPtr);
}

ConstStrA Result::getFieldName(std::size_t i) const {
	return getFieldInfo(i)->name;
}

const MYSQL_FIELD *Result::getFieldInfo(std::size_t i) const {
	checkResult(THISLOCATION);
	if (i >= cur->fieldCount)
		LightSpeed::throwRangeException_To(THISLOCATION,(std::size_t)cur->fieldCount,i);
	return mysql_fetch_field_direct(cur->resPtr,i);
}

void FieldNotFoundException_t::message(LightSpeed::ExceptionMsg &msg) const {
	msg("Field '%1' has not been found in the result") << fieldName.c_str();
}
void NullFieldException_t::message(LightSpeed::ExceptionMsg &msg) const {
	msg("Field '%1' is NULL") << fieldName.c_str();
}

Result::RangeIter::RangeIter(Result& owner, bool isend)
	:owner(owner),isend(isend) {}

bool Result::RangeIter::operator ==(const RangeIter& other) const {
	return &owner == &other.owner && isend == other.isend;
}

bool Result::RangeIter::operator !=(const RangeIter& other) const {
	return !operator==(other);
}

Row Result::RangeIter::operator *() const {
	return Row(owner,owner.readyRow,mysql_fetch_lengths(owner.cur->resPtr),mysql_num_fields(owner.cur->resPtr));
}

Result::RangeIter& Result::RangeIter::operator ++() {
	owner.loadNextRow();
	if (owner.readyRow == 0) isend = true;
	return *this;
}

Result::RangeIter Result::begin() {
	rewind();
	return RangeIter(*this,!hasItems());
}

Result::RangeIter Result::end() {
	return RangeIter(*this,true);
}

}
