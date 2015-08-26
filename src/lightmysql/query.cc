/** @file
 * Copyright (c) 2006, Seznam.cz, a.s.
 * All Rights Reserved.
 * 
 * $Id: query.cc 1555 2011-01-07 17:02:49Z ondrej.novak $
 *
 * DESCRIPTION
 * Short description
 * 
 * AUTHOR
 * Ondrej Novak <ondrej.novak@firma.seznam.cz>
 *
 */

#include "query.h"
#include <stdio.h>
#include <sstream>
#include "result.h"
#include <lightspeed/base/containers/autoArray.tcc>
#include "lightspeed/base/memory/smallAlloc.h"
#include <lightspeed/base/streams/utf.h>
#include <lightspeed/base/streams/utf.tcc>

using LightSpeed::SmallAlloc;


namespace LightMySQL {

Query::Query(IConnection &conn):conn(conn),commitPos(0),lastCmd(cmdNotSet),executed(0) {

}

Query::Query(const Query &other):conn(other.conn),lastCmd(cmdNotSet),executed(true) {}

Query & Query::arg(long long i)
{
	char buff[100];
	sprintf(buff,"%lld",i);
	paramBuffer.append(ConstStrA(buff));
	paramEnds.add(paramBuffer.length());
	return *this;
}

Query & Query::arg(unsigned long long i)
{
	char buff[100];
	sprintf(buff,"%llu",i);
	paramBuffer.append(ConstStrA(buff));
	paramEnds.add(paramBuffer.length());
	return *this;
}

Query & Query::arg(int i)
{
	char buff[100];
	sprintf(buff,"%d",i);
	paramBuffer.append(ConstStrA(buff));
	paramEnds.add(paramBuffer.length());
	return *this;
}

ConstStrA Query::build() const {

	if (commitPos)
		queryBuffer.erase(commitPos,queryBuffer.length()-commitPos);
	else
		queryBuffer.clear();
	for (AutoArray<char>::Iterator iter = queryText.getFwIter(); iter.hasItems(); ) {

		char c = iter.getNext();
		if (c != '%') queryBuffer.add(c);
		else {
			if (!iter.hasItems() || iter.peek() == '%') {
				queryBuffer.add(iter.getNext());
			} else {
				std::size_t idx = 0;
				while (iter.hasItems() && isdigit(iter.peek())) {
					idx = idx * 10 + ((iter.getNext()) - '0');
				}
				if (idx < 1 || idx > paramEnds.length())
					throw UnassignedQueryParameterException_t(
							THISLOCATION,queryText,idx);
				idx--;
				std::size_t b = idx == 0?0:paramEnds[idx-1];
				std::size_t e = paramEnds[idx];
				while (b < e) queryBuffer.add(paramBuffer[b++]);

			}
		}
	}
	return queryBuffer;
}

Result Query::exec()
{
	if (queryText.empty())
		throw EmptyQueryException_t(THISLOCATION);
	build();
	paramBuffer.clear();
	paramEnds.clear();
	executed = true;
	lastCmd = cmdNotSet;
	return conn.executeQuery(queryBuffer);

}

Query &Query::append(ConstStrA queryText) {
	build();
	commitPos = queryBuffer.length();
	this->queryText.clear();
	this->queryText.append(queryText);
	paramBuffer.clear();
	paramEnds.clear();
	return *this;
}


Query & Query::arg(ConstStrA str)
{
	paramBuffer.add('\'');
	paramBuffer.append(conn.escapeString(str));
	paramBuffer.add('\'');
	paramEnds.add(paramBuffer.length());
	return *this;
}

Query &Query::arg(ConstStrW str)
{
	AutoArray<char,SmallAlloc<256> > buff;
	WideToUtf8Reader<ConstStrW::Iterator> iter(str.getFwIter());
	while (iter.hasItems()) {
		buff.add(iter.getNext());
	}
	return arg(ConstStrA(buff));
}


Query & Query::arg(double val)
{
	char buff[100];
	sprintf(buff,"%g",val);
	paramBuffer.append(ConstStrA(buff));
	paramEnds.add(paramBuffer.length());
	return *this;
}

Query & Query::operator ()(ConstStrA queryText)
{
	if (executed) {
		clear();
		this->queryText.append(queryText);
	} else {
		if (commitPos) append(";");
		append(queryText);
	}
	return *this;
}


Query & Query::arg(unsigned int i)
{
	char buff[100];
	sprintf(buff,"%u",i);
	paramBuffer.append(ConstStrA(buff));
	paramEnds.add(paramBuffer.length());
	return *this;
}

Query & Query::arg(unsigned long i)
{
	char buff[100];
	sprintf(buff,"%lu",i);
	paramBuffer.append(ConstStrA(buff));
	paramEnds.add(paramBuffer.length());
	return *this;
}

Query & Query::arg(long i)
{
	char buff[100];
	sprintf(buff,"%ld",i);
	paramBuffer.append(ConstStrA(buff));
	paramEnds.add(paramBuffer.length());
	return *this;
}

static bool isUserDefVar(ConstStrA val) {
	ConstStrA::Iterator iter = val.getFwIter();
	if (!iter.hasItems()) return false;
	if (iter.getNext() != '@') return false;
	while (iter.hasItems()) {
		if (!isalnum(iter.getNext())) return false;
	}
	return true;
}

Query &Query::field(ConstStrA val) {
	if (isUserDefVar(val)) {
		raw(val);
		return *this;
	} else {
		paramBuffer.add('`');
		for (ConstStrA::Iterator iter = val.getFwIter(); iter.hasItems(); ){
			char f = iter.getNext();
			if (f == '`') paramBuffer.add('\\');
			paramBuffer.add(f);
		}
		paramBuffer.add('`');
		paramEnds.add(paramBuffer.length());
		return *this;
	}
}

Query &Query::field(ConstStrA table, ConstStrA val) {
	field(table).appendArg().raw('.').appendArg().field(val);
	return *this;
}

Query &Query::field(ConstStrA database, ConstStrA table, ConstStrA val) {
	field(database).appendArg().raw('.').appendArg().field(table).appendArg().raw('.').appendArg().field(val);
	return *this;
}


Query &Query::raw(ConstStrA field) {
	paramBuffer.append(field);
	paramEnds.add(paramBuffer.length());
	return *this;

}

Query &Query::null() {
	return raw("NULL");
}

Query &Query::now() {
	return raw("NOW()");
}

Query &Query::curDate() {
	return raw("CURDATE()");
}

void UnassignedQueryParameterException_t::message(LightSpeed::ExceptionMsg & msg) const
 {
	msg("Unassigned parameter %%%1  in the query: '%2'") << pos << query.c_str();
}

void EmptyQueryException_t::message(LightSpeed::ExceptionMsg & msg) const
 {
	msg("Execution of an empty query");
}


Query& Query::escaped(ConstStrA str) {
	paramBuffer.append(conn.escapeString(str));
	paramEnds.add(paramBuffer.length());
	return *this;
}

Query& Query::escaped(ConstStrW str) {
	AutoArray<char,SmallAlloc<256> > buff;
	WideToUtf8Reader<ConstStrW::Iterator> iter(str.getFwIter());
	while (iter.hasItems()) {
		buff.add(iter.getNext());
	}
	return escaped(ConstStrA(buff));
}

Query& Query::field(ConstStrW f) {
	AutoArray<char,SmallAlloc<256> > buff;
	WideToUtf8Reader<ConstStrW::Iterator> iter(f.getFwIter());
	while (iter.hasItems()) {
		buff.add(iter.getNext());
	}
	return field(ConstStrA(buff));
}

Query& Query::field(ConstStrW table, ConstStrW val) {
	field(table).appendArg().raw('.').appendArg().field(val);
	return *this;
}

Query& Query::field(ConstStrW database, ConstStrW table, ConstStrA val) {
	field(database).appendArg().raw('.').appendArg().field(table).appendArg().raw('.').appendArg().field(val);
	return *this;
}


void Query::clear() {
	this->queryText.clear();
	paramBuffer.clear();
	paramEnds.clear();
	queryBuffer.clear();
	commitPos = 0;
	executed = false;
	lastCmd = cmdNotSet;
}

Query& Query::INSERT(ConstStrA pattern) {
	beginCommand(cmdInsert, "INSERT INTO");
	append(pattern);
	return *this;
}

Query &Query::INSERT_IGNORE(ConstStrA pattern) {
	beginCommand(cmdInsert, "INSERT IGNORE INTO ");
	append(pattern);
	return *this;

}


Query& Query::SET(ConstStrA f, ConstStrA expr) {
	if (lastCmd == cmdOnDuplicateKeyUpdate)
		beginCommand(cmdSet,"ON DUPLICATE KEY UPDATE");
	else
		beginCommand(cmdSet, "SET");
	append("%1=").field(f);
	append(expr);
	return *this;
}

Query& Query::UPDATE(ConstStrA pattern) {
	beginCommand(cmdUpdate, "UPDATE");
	append(pattern);
	return *this;
}

Query& Query::REPLACE(ConstStrA pattern) {
	beginCommand(cmdReplace, "REPLACE");
	append(pattern);
	return *this;
}

Query& Query::DELETE(ConstStrA pattern) {
	beginCommand(cmdDelete, "DELETE");
	append(pattern);
	return *this;
}

Query& Query::DELETE() {
	beginCommand(cmdDelete, "DELETE");
	return *this;
}

Query& Query::SELECT(ConstStrA pattern, bool calcfoundrows) {
	if (beginCommand(cmdSelect, "SELECT") && calcfoundrows) {
		append("SQL_CALC_FOUND_ROWS ");
	}
	append(pattern);
	return *this;
}

Query& Query::SELECT_DISTINCT(ConstStrA pattern, bool calcfoundrows) {
	if (beginCommand(cmdSelect, "SELECT DISTINCT") && calcfoundrows) {
		append("SQL_CALC_FOUND_ROWS ");
	}
	append(pattern);
	return *this;
}

Query& Query::SELECT_DISTINCTROW(ConstStrA pattern, bool calcfoundrows) {
	if (beginCommand(cmdSelect, "SELECT DISTINCTROW") && calcfoundrows) {
		append("SQL_CALC_FOUND_ROWS ");
	}
	append(pattern);
	return *this;
}

Query& Query::FROM(ConstStrA table) {
	beginCommand(cmdFrom, "FROM");
	appendFieldName(table);
	return *this;
}

Query& Query::FROM(ConstStrA table,ConstStrA asTable) {
	beginCommand(cmdFrom, "FROM");
	appendFieldName(table,asTable);
	return *this;
}

Query& Query::FROM_pat(ConstStrA pattern) {
	beginCommand(cmdFrom, "FROM");
	append(pattern);
	return *this;
}

SubQuery Query::FROM_SUB(ConstStrA subqueryName) {
	beginCommand(cmdFrom, "FROM");
	return SubQuery(*this,subqueryName);
}

SubQuery Query::JOIN_SUB(ConstStrA subqueryName) {
	beginCommand(cmdJoin, "JOIN");
	return SubQuery(*this,subqueryName);
}

SubQuery Query::LEFT_JOIN_SUB(ConstStrA subqueryName) {
	beginCommand(cmdLeftJoin, "LEFT JOIN");
	return SubQuery(*this,subqueryName);
}


Query& Query::JOIN(ConstStrA table) {
	beginCommand(cmdJoin, "JOIN");
	appendFieldName(table);
	return *this;

}
Query& Query::JOIN(ConstStrA table, ConstStrA asTable) {
	beginCommand(cmdJoin, "JOIN");
	appendFieldName(table,asTable);
	return *this;

}

Query& Query::JOIN_pat(ConstStrA pattern) {
	beginCommand(cmdJoin, "JOIN");
	append(pattern);
	return *this;
}

Query& Query::LEFT_JOIN(ConstStrA table) {
	beginCommand(cmdLeftJoin, "LEFT JOIN");
	appendFieldName(table);
	return *this;
}
Query& Query::LEFT_JOIN(ConstStrA table, ConstStrA asTable) {
	beginCommand(cmdLeftJoin, "LEFT JOIN");
	appendFieldName(table,asTable);
	return *this;
}

Query& Query::LEFT_JOIN_pat(ConstStrA pattern) {
	beginCommand(cmdLeftJoin, "LEFT JOIN");
	append(pattern);
	return *this;
}

Query& Query::ON(ConstStrA pattern) {
	beginCommand(cmdOn, "ON","AND");
	append(pattern);
	return *this;
}

Query& Query::ON(ConstStrA table1, ConstStrA field1,ConstStrA table2, ConstStrA field2) {
	return ON("%1=%2").field(table1,field1).field(table2,field2);
}

Query& Query::WHERE(ConstStrA pattern) {
	beginCommand(cmdWhere, "WHERE","AND");
	append(pattern);
	return *this;
}

Query& Query::ORDERBY_pat(ConstStrA pattern) {
	beginCommand(cmdOrder, "ORDER BY");
	append(pattern);
	return *this;
}

Query &Query::ORDERBY(ConstStrA field, bool descending) {
	beginCommand(cmdOrder, "ORDER BY");
	append("%1 %2").field(field);
	if (descending) raw("DESC"); else raw("ASC");
	return *this;
}

Query &Query::ORDERBY(ConstStrA table, ConstStrA field, bool descending) {
	beginCommand(cmdOrder, "ORDER BY");
	append("%1 %2").field(table,field);
	if (descending) raw("DESC"); else raw("ASC");
	return *this;
}


Query& Query::GROUPBY_pat(ConstStrA pattern) {
	beginCommand(cmdGroupBy, "GROUP BY");
	append(pattern);
	return *this;
}

Query& Query::GROUPBY(ConstStrA field) {
	beginCommand(cmdGroupBy, "GROUP BY");
	append("%1").field(field);
	return *this;
}

Query& Query::GROUPBY(ConstStrA table, ConstStrA field) {
	beginCommand(cmdGroupBy, "GROUP BY");
	append("%1").field(table,field);
	return *this;
}


Query& Query::HAVING(ConstStrA pattern) {
	beginCommand(cmdHaving, "HAVING","AND");
	append(pattern);
	return *this;
}

Query& Query::ON_DUPLICATE_KEY_UPDATE(ConstStrA field, ConstStrA expr) {
	beginCommand(cmdSet, "ON DUPLICATE KEY UPDATE");
	append("%1=").arg(field);
	append(expr);
	return *this;
}

Query& Query::operator ,(ConstStrA pattern) {
	append(",");
	append(pattern);
	return *this;
}

Query& Query::AND(ConstStrA pattern) {
	beginCommand(cmdAnd,"","AND");
	append(pattern);
	return *this;
}

Query& Query::OR(ConstStrA pattern) {
	beginCommand(cmdOr,"","OR");
	append(pattern);
	return *this;
}

Query& Query::LIMIT(natural count) {
	beginCommand(cmdLimit,"LIMIT","");
	append("%1").arg(count);
	return *this;
}

Query &Query::LIMIT(natural offset,natural count) {
	beginCommand(cmdLimit,"LIMIT","");
	append("%1,%2").arg(offset).arg(count);
	return *this;
}

Query &Query::VALUES(ConstStrA pattern) {
	beginCommand(cmdValues,"VALUES",",");
	append("(");
	this->queryText.append(pattern);
	this->queryText.append(ConstStrA(")"));
	return *this;

}

bool Query::beginCommand(CmdType cmd, ConstStrA cmdName) {
	if (executed) clear();
	bool res = cmd != lastCmd;
	this->queryText.append(ConstStrA(" "));
	append(res?cmdName:",");
	this->queryText.append(ConstStrA(" "));
	lastCmd = cmd;
	return res;
}

Query& Query::end() {
	beginCommand(cmdNotSet,";","");
	return *this;
}

bool Query::beginCommand(CmdType cmd, ConstStrA cmdName, ConstStrA separator) {
	if (executed) clear();
	bool res = cmd != lastCmd;
	this->queryText.append(ConstStrA(" "));
	append(res?cmdName:separator);
	this->queryText.append(ConstStrA(" "));
	lastCmd = cmd;
	return res;
}

void Query::appendFieldName(ConstStrA fieldName) {
	const char *p1 = "%1";
	const char *p2 = ".%1";
	for (ConstStrA::SplitIterator iter = fieldName.split('.');iter.hasItems();) {
		ConstStrA part = iter.getNext();
		append(p1).field(part); p1 = p2;
	}
}
void Query::appendFieldName(ConstStrA fieldName, ConstStrA asName){
	appendFieldName(fieldName);
	append(ConstStrA(" AS "));
	appendFieldName(asName);

}

CreateTableDef Query::CREATE_TEMPORARY_TABLE(ConstStrA tableName) {
	append("DROP TEMPORARY TABLE IF EXIST %1;").field(tableName);
	append("CREATE TEMPORARY TABLE %1").field(tableName);
	return CreateTableDef(*this);
}


Query& Query::SET(ConstStrA field) {
	return SET(field,"%1");
}

Query& Query::ON_DUPLICATE_KEY_UPDATE(ConstStrA field) {
	return ON_DUPLICATE_KEY_UPDATE(field,"%1");
}


Query& Query::ON_DUPLICATE_KEY_UPDATE() {
	lastCmd = cmdOnDuplicateKeyUpdate;
	return *this;
}

Query &Query::FOR_UPDATE() {
	leaveAll().append(" FOR UPDATE");
	return *this;
}

Query &Query::LOCK_IN_SHARE_MODE() {
	leaveAll().append(" LOCK IN SHARE MODE");
	return *this;
}

Query &Query::leave() {return *this;}
Query &Query::endsub() {return *this;}

Query &Query::leaveAll() {return *this;}

SubQuery Query::sub() {return SubQuery(*this);}


SubQuery::SubQuery(Query &q):Query(q),parent(q),active(true)
{
	q.append("(");
}
SubQuery::~SubQuery() {
	leave();
}
Result SubQuery::exec() {
	return leave().exec();
}
Query &SubQuery::leave() {
	if (active) {
		append(") %1").raw(suffix);
		ConstStrA res = build();
		parent.append("%1").raw(res);
	}
	active = false;
	return parent;
}
Query &SubQuery::end() {
	return leave().end();
}

Query &SubQuery::endsub() {
	return leave();
}

SubQuery::SubQuery(Query& q, ConstStrA suffix):Query(q),parent(q),suffix(suffix),active(true)
{
	q.append("(");
}

Query &SubQuery::leaveAll() {
	return leave().leaveAll();
}

CreateTableDef::CreateTableDef(Query &q):SubQuery(q) {
}

CreateTableDef &CreateTableDef::fieldDef(ConstStrA type, ConstStrA fieldName) {
	append("%1 %2 %3").raw(sep).field(fieldName).raw(type);sep=",";
	return *this;

}
CreateTableDef &CreateTableDef::fieldDef(ConstStrA type, ConstStrA fieldName, natural len) {
	append("%1 %2 %3(%4)").raw(sep).field(fieldName).raw(type).arg(len);sep=",";
	return *this;

}
CreateTableDef &CreateTableDef::fieldDef(ConstStrA type, ConstStrA fieldName, natural len, natural decm) {
	append("%1 %2 %3(%4,%5)").raw(sep).field(fieldName).raw(type).arg(len).arg(decm);sep=",";
	return *this;
}

CreateTableDef &CreateTableDef::BIT(ConstStrA name) {return fieldDef("BIT",name);}
CreateTableDef &CreateTableDef::TINYINT(ConstStrA name) {return fieldDef("TINYINT",name);}
CreateTableDef &CreateTableDef::SMALLINT(ConstStrA name) {return fieldDef("SMALLINT",name);}
CreateTableDef &CreateTableDef::MEDIUMINT(ConstStrA name) {return fieldDef("MEDIUMINT",name);}
CreateTableDef &CreateTableDef::INT(ConstStrA name) {return fieldDef("INT",name);}
CreateTableDef &CreateTableDef::BIGINT(ConstStrA name) {return fieldDef("BIGINT",name);}
CreateTableDef &CreateTableDef::REAL(ConstStrA name) {return fieldDef("REAL",name);}
CreateTableDef &CreateTableDef::DOUBLE(ConstStrA name) {return fieldDef("DOUBLE",name);}
CreateTableDef &CreateTableDef::FLOAT(ConstStrA name) {return fieldDef("FLOAT",name);}
CreateTableDef &CreateTableDef::DECIMAL(ConstStrA name, natural length, natural decimals) {return fieldDef("DECIMAL",name,length,decimals);}
CreateTableDef &CreateTableDef::NUMERIC(ConstStrA name, natural length, natural decimals) {return fieldDef("NUMERIC",name,length,decimals);}
CreateTableDef &CreateTableDef::UNSIGNED_TINYINT(ConstStrA name)  {return fieldDef("TINYINT",name).append(" UNSIGNED");}
CreateTableDef &CreateTableDef::UNSIGNED_SMALLINT(ConstStrA name) {return fieldDef("SMALLINT",name).append(" UNSIGNED");}
CreateTableDef &CreateTableDef::UNSIGNED_MEDIUMINT(ConstStrA name) {return fieldDef("MEDIUMINT",name).append(" UNSIGNED");}
CreateTableDef &CreateTableDef::UNSIGNED_INT(ConstStrA name) {return fieldDef("INT",name).append(" UNSIGNED");}
CreateTableDef &CreateTableDef::UNSIGNED_BIGINT(ConstStrA name) {return fieldDef("BIGINT",name).append(" UNSIGNED");}
CreateTableDef &CreateTableDef::UNSIGNED_REAL(ConstStrA name) {return fieldDef("REAL",name).append(" UNSIGNED");}
CreateTableDef &CreateTableDef::UNSIGNED_DOUBLE(ConstStrA name) {return fieldDef("DOUBLE",name).append(" UNSIGNED");}
CreateTableDef &CreateTableDef::UNSIGNED_FLOAT(ConstStrA name) {return fieldDef("FLOAT",name).append(" UNSIGNED");}
CreateTableDef &CreateTableDef::UNSIGNED_DECIMAL(ConstStrA name, natural length, natural decimals) {return fieldDef("DECIMAL",name,length,decimals).append(" UNSIGNED");}
CreateTableDef &CreateTableDef::UNSIGNED_NUMERIC(ConstStrA name, natural length, natural decimals) {return fieldDef("NUMERIC",name,length,decimals).append(" UNSIGNED");}
CreateTableDef &CreateTableDef::DATE(ConstStrA name)  {return fieldDef("DATE",name);}
CreateTableDef &CreateTableDef::TIME(ConstStrA name)  {return fieldDef("TIME",name);}
CreateTableDef &CreateTableDef::TIMESTAMP(ConstStrA name)  {return fieldDef("TIMESTAMP",name);}
CreateTableDef &CreateTableDef::DATETIME(ConstStrA name)  {return fieldDef("DATETIME",name);}
CreateTableDef &CreateTableDef::YEAR(ConstStrA name)  {return fieldDef("YEAR",name);}
CreateTableDef &CreateTableDef::CHAR(ConstStrA name, natural length)  {return fieldDef("CHAR",name,length);}
CreateTableDef &CreateTableDef::VARCHAR(ConstStrA name, natural length)  {return fieldDef("VARCHAR",name,length);}
CreateTableDef &CreateTableDef::BINARY(ConstStrA name, natural length)  {return fieldDef("BINARY",name,length);}
CreateTableDef &CreateTableDef::VARBINARY(ConstStrA name, natural length)  {return fieldDef("VARCHAR",name,length);}
CreateTableDef &CreateTableDef::TINYBLOB(ConstStrA name)  {return fieldDef("TINYBLOB",name);}
CreateTableDef &CreateTableDef::BLOB(ConstStrA name)   {return fieldDef("BLOB",name);}
CreateTableDef &CreateTableDef::MEDIUMBLOB(ConstStrA name)   {return fieldDef("MEDIUMBLOB",name);}
CreateTableDef &CreateTableDef::LONGBLOB(ConstStrA name)   {return fieldDef("LONGBLOB",name);}
CreateTableDef &CreateTableDef::TINYTEXT(ConstStrA name) {return fieldDef("TINYTEXT",name);}
CreateTableDef &CreateTableDef::TEXT(ConstStrA name)   {return fieldDef("TEXT",name);}
CreateTableDef &CreateTableDef::MEDIUMTEXT(ConstStrA name) {return fieldDef("MEDIUMTEXT",name);}
CreateTableDef &CreateTableDef::LONGTEXT(ConstStrA name) {return fieldDef("LONGTEXT",name);}
CreateTableDef &CreateTableDef::SET(ConstStrA name) {fieldDef("SET",name).append("("); sep = "),";fieldsep = "";return *this;}
CreateTableDef &CreateTableDef::ENUM(ConstStrA name)  {fieldDef("ENUM",name).append("("); sep = "),";fieldsep = "";return *this;}
CreateTableDef &CreateTableDef::val(ConstStrA name) {append("%1 %2").raw(fieldsep).arg(name);fieldsep = ",";return *this;}
CreateTableDef &CreateTableDef::charset(ConstStrA charset) {append(" CHARACTER SET %1").raw(charset);return *this;}
CreateTableDef &CreateTableDef::collate(ConstStrA collate) {append(" COLLATE %1").raw(collate);return *this;}
CreateTableDef &CreateTableDef::null() {return append(" NULL");}
CreateTableDef &CreateTableDef::not_null() {return append(" NOT NULL");}
CreateTableDef &CreateTableDef::DEFAULT() {return append(" DEFAULT %1");}
CreateTableDef &CreateTableDef::AUTO_INCREMENT_INT(ConstStrA name) {
	return INT(name).append(" AUTO_INCREMENT");
}
CreateTableDef &CreateTableDef::AUTO_INCREMENT_BIGINT(ConstStrA name){
	return BIGINT(name).append(" AUTO_INCREMENT");
}
///Definition of autoincrement unsigned integer
CreateTableDef &CreateTableDef::AUTO_INCREMENT_UNSIGNED_INT(ConstStrA name){
	return UNSIGNED_INT(name).append(" AUTO_INCREMENT");
}
///Definition of autoincrement unsigned big integer
CreateTableDef &CreateTableDef::AUTO_INCREMENT_UNSIGNED_BIGINT(ConstStrA name) {
	return UNSIGNED_BIGINT(name).append(" AUTO_INCREMENT");
}
///Definition of primary key. Use col() to specify columns
CreateTableDef &CreateTableDef::PRIMARY_KEY() {
	append("%1 PRIMARY_KEY (").raw(sep);
	sep = "),"; fieldsep = "";
	return *this;
}
///Definition of index. Use col() to specify columns
CreateTableDef &CreateTableDef::INDEX()  {
	append("%1 INDEX (").raw(sep);
	sep = "),"; fieldsep = "";
	return *this;
}
///Definition of unique key. Use col() to specify columns
CreateTableDef &CreateTableDef::UNIQUE()  {
	append("%1 UNIQUE (").raw(sep);
	sep = "),"; fieldsep = "";
	return *this;
}
///Include column name into previous index defintion. Terminates by another definition
CreateTableDef &CreateTableDef::col(ConstStrA name) {
	append("%1 %2").raw(fieldsep).field(name);
	fieldsep = ",";
	return *this;

}
///Definition foreing key. Use col to specify columns. It must be followed by REFERENCES()
CreateTableDef &CreateTableDef::FOREIGN_KEY() {
	append("%1 FOREIGN KEY (").raw(sep);
	sep = "),"; fieldsep = "";frend = "),";
	return *this;

}
///Definition foreing key with cascade deletion. Use col to specify columns. It must be followed by REFERENCES()
CreateTableDef &CreateTableDef::FOREIGN_KEY_CASCADE()  {
	FOREIGN_KEY();
	frend = ") ON DELETE CASCADE,";
	return *this;

}
///Definition foreing key with cascade update. Use col to specify columns. It must be followed by REFERENCES()
CreateTableDef &CreateTableDef::FOREIGN_KEY_UPDATABLE() {
	FOREIGN_KEY();
	frend = ") ON UPDATE CASCADE,";
	return *this;

}
///Definition foreing key with cascade deletion and update. Use col to specify columns. It must be followed by REFERENCES()
CreateTableDef &CreateTableDef::FOREIGN_KEY_CASCADE_UPDATABLE() {
	FOREIGN_KEY();
	frend = ") ON DELETE CASCADE ON UPDATE CASCADE,";
	return *this;

}
///Definition of reference table of previous foreign key
CreateTableDef &CreateTableDef::REFERENCES(ConstStrA name) {
	append(" REFERENCES %1 (").field(name);
	sep = frend; fieldsep = "";frend = ConstStrA();
	return *this;
}

Query &CreateTableDef::leave() {
	if (sep.length()>1) append(sep.crop(0,1));
	sep = ConstStrA();
	return SubQuery::leave();
}

}
