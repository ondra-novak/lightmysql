/*
 * MySQLReport.cpp
 *
 *  Created on: 14.5.2013
 *      Author: ondra
 */


#include <mysql/mysql.h>
#include "mydbglog.h"
#include "lightspeed/base/debug/dbglog.h"

namespace LightMySQL {

using namespace LightSpeed;

const char *masterDBTitle = "mysql-master";
const char *slaveDBTitle = "mysql-slave";

#define DBTITLE (master?masterDBTitle:slaveDBTitle)

void MySQLReport::onQueryExec(LightSpeed::ConstStrA queryText) {
	natural l = queryText.find(ConstStrA("PASSWORD("));
	if (l != naturalNull) {
		natural b = queryText.find(ConstStrA(")"),l);
		if (b != naturalNull) {
			StringA tmp = queryText.head(l) + ConstStrA("('xxxxxxx'")+queryText.offset(b);
			onQueryExec(tmp);
			return;
		}
	}
	LogObject(THISLOCATION).debug("%1 Query: %2") << DBTITLE << queryText;
}

void MySQLReport::onQueryError(LightSpeed::ConstStrA errorText) {
	LogObject(THISLOCATION).error("%1 Error: %2") << DBTITLE << errorText ;
}

void MySQLReport::onQueryResult(my_ulonglong rows, my_ulonglong cols,
		my_ulonglong affected, my_ulonglong lastId, my_ulonglong warn) {
	if (cols > 0) {
		LogObject(THISLOCATION).debug("%1 Result: rows %2, cols %3")
				  << DBTITLE << (natural)rows << (natural)cols ;
	} else if (lastId > 0) {
		LogObject(THISLOCATION).debug("%1 Inserted rows: %2, ID: %3")
				<< DBTITLE << (natural)affected << (natural)lastId;
	} else if (affected > 0) {
		LogObject(THISLOCATION).debug("%+ Affected rows: %2")
				<< DBTITLE << (natural)affected;
	}
	if (warn > 0) LogObject(THISLOCATION).debug("%1 Warnings: %2") << DBTITLE << (natural)warn;

}

void MySQLReport::onQueryInfo(LightSpeed::ConstStrA info) {
	LogObject(THISLOCATION).debug("%1 Info: %2") << DBTITLE << info;
}

void MySQLReport::serverConnect(LightSpeed::ConstStrA host, int port,
		LightSpeed::ConstStrA dbname) {
	LogObject(THISLOCATION).info("%1 Connect: %2:%3 database %4") << DBTITLE<< host << port << dbname;

}

MySQLReport::MySQLReport(bool master):master(master) {

}

void MySQLReport::serverDisconnect() {
	LogObject(THISLOCATION).info("%1 Disconnect")<< DBTITLE;
}

} /* namespace jsonsrv */
