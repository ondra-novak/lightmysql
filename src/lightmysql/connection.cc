/** @file
 * Copyright (c) 2006, Seznam.cz, a.s.
 * All Rights Reserved.
 * 
 * $Id: connection.cc 1800 2011-05-31 16:18:05Z ondrej.novak $
 *
 * DESCRIPTION
 * Short description
 * 
 * AUTHOR
 * Ondrej Novak <ondrej.novak@firma.seznam.cz>
 *
 */

#include "connection.h"
#include "result.h"
#include "mysql/errmsg.h"
#include <lightspeed/base/containers/autoArray.tcc>
#include <lightspeed/base/memory/smallAlloc.h>

#include "threadHook.h"
namespace LightMySQL {

static ThreadHook thrhook;

Connection::Connection()
	:connected(false),logObject(0),transactionObjects(0)
{
	thrhook.install();
	mysql_init(&conn);
}

Connection::Connection(const ConnectParams &params, unsigned long flags /*= 0*/)
	:connected(false),logObject(0),transactionObjects(0)
{
	mysql_init(&conn);
	connect(params,flags);
}


Connection::~Connection()
{
	close();
}





void Connection::close()
{
	if (connected) {
		reconnectParams.host.clear();
		reconnectParams.lifetime = ConnectParams::defaultLifetime;
		mysql_close(&conn);
		connected = false;
		if (logObject)
			logObject->serverDisconnect();
	}
}

void Connection::closeTemporary()
{
	if (connected) {
		mysql_close(&conn);
		connected = false;
		if (logObject)
			logObject->serverDisconnect();
	}
}

StringA Connection::escapeString(ConstStrA str) {
	AutoArray<char, SmallAlloc<256> > buff;
	buff.resize(str.length()*2+1);
	unsigned int cnt = mysql_real_escape_string(
					&conn,buff.data(),str.data(),str.length());
	return StringA(ConstStrA(buff.data(),cnt));
}

Result Connection::executeQuery(ConstStrA query) {
	if (connected == false)
		throw ServerError_t(THISLOCATION,2006,"mysql is disconnected");
	if (logObject) logObject->onQueryExec(query);
	if (mysql_real_query(&conn,query.data(),query.length()) != 0) {
		if (logObject) logObject->onQueryError(mysql_error(&conn));
		handleError(THISLOCATION);
	}
	return Result(conn, logObject);
}

void Connection::handleError(const ProgramLocation &l) {
	int errnr = mysql_errno(&conn);
	if (errnr == CR_SERVER_GONE_ERROR || errnr == CR_SERVER_LOST) {
		mysql_close(&conn);
		connected = false;
	}
	if (errnr != 0) {
		throw ServerError_t(l,errnr,mysql_error(&conn));
	}
}

unsigned int Connection::getLastError() const {
	return mysql_errno(&conn);
}
StringA Connection::getLastErrorMsg() const {
	return mysql_error(&conn);
}

void Connection::setOption(mysql_option opt, const char *value) {
	if (mysql_options(&conn,opt,value) !=0) handleError(THISLOCATION);
}
void Connection::ping() {
	if (mysql_ping(&conn) !=0 ) handleError(THISLOCATION);
}

void Connection::connect(const ConnectParams &params, unsigned long flags /*= 0*/) {

	flags |= CLIENT_MULTI_STATEMENTS | CLIENT_MULTI_RESULTS;
	int port = params.port;
	if (port == 0) port = 3306;

	reconnectParams = params;
	reconnectFlags = flags;

	if (params.lifetime != ConnectParams::closeTransaction) {
		reconnect();
	}


}

void Connection::setLogObject(IDebugLog *logObject) {
	this->logObject = logObject;
}

IDebugLog *Connection::getLogObject() const {
	return logObject;
}

void Connection::reconnect() {
	closeTemporary();

	ConnectParams &params = reconnectParams;
	unsigned long &flags = reconnectFlags;

	MYSQL *res = mysql_real_connect(&conn,params.host.c_str(),
			params.authInfo.username.c_str(),params.authInfo.password.c_str(),
			params.dbname.c_str(),params.port,params.socket.c_str(), flags );
	if (res == 0) handleError(THISLOCATION);

	connected = true;
	if (logObject)
		logObject->serverConnect(params.host, params.port, params.dbname);
	executeQuery("SET NAMES utf8");
}

void Connection::openTransactionWithLevel(Level isolationLevel) {
	const char* levelstr = 0;
	switch (isolationLevel) {
	case readCommited:
		levelstr = "READ COMMITTED";
		break;
	case readUncommited:
		levelstr = "READ UNCOMMITTED";
		break;
	case repeatable:
		levelstr = "REPEATABLE READ";
		break;
	case serializable:
		levelstr = "SERIALIZABLE";
		break;
	default:
		levelstr = 0;
		break;
	}
	if (levelstr)
		executeQuery(StringA(ConstStrA("SET TRANSACTION ISOLATION LEVEL ")
								+ ConstStrA(levelstr)));

	executeQuery("START TRANSACTION");
}

///Starts transaction
void Connection::startTransaction(Level isolationLevel) {
	if (!connected
			&& reconnectParams.lifetime != ConnectParams::defaultLifetime)
				reconnect();

	try {
		openTransactionWithLevel(isolationLevel);
	} catch (ServerError_t &e) {
		if ((e.getErrno() == CR_SERVER_GONE_ERROR
				|| e.getErrno() == CR_SERVER_LOST)
		&& (reconnectParams.lifetime == ConnectParams::reconnectTransaction)
			) {
			reconnect();
			openTransactionWithLevel(isolationLevel);
		} else {
			throw;
		}
	}
}

void Connection::commitTransaction() {
	executeQuery("COMMIT");
	if (reconnectParams.lifetime == ConnectParams::closeTransaction)
		closeTemporary();

}

void Connection::rollbackTransaction() {
		executeQuery("ROLLBACK");
		if (reconnectParams.lifetime == ConnectParams::closeTransaction)
			closeTemporary();

}


natural Connection::getConnectionId() {
	return mysql_thread_id(&conn);
}

void Connection::killConnection(natural connectionId) {
	char fmt[100];
	sprintf(fmt,"KILL %d",(int)connectionId);
	executeQuery(fmt);
}

void Connection::logString(ConstStrA str, bool error) {
	if (logObject) {
		if (error)
			logObject->onQueryError(str);
		else
			logObject->onQueryInfo(str);
	}
}

bool Connection::isLogEnabled() const {
	return logObject != 0;
}


}

