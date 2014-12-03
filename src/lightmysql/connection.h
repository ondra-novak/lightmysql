/** @file
 * Copyright (c) 2006, Seznam.cz, a.s.
 * All Rights Reserved.
 * 
 * $Id: connection.h 1612 2011-02-11 14:25:19Z ondrej.novak $
 *
 * DESCRIPTION
 * Short description
 * 
 * AUTHOR
 * Ondrej Novak <ondrej.novak@firma.seznam.cz>
 *
 */

#ifndef MYSQL_CONNECTION_H_
#define MYSQL_CONNECTION_H_

#pragma once

#include <mysql/mysql.h>
#include <lightspeed/base/memory/sharedResource.h>
#include <lightspeed/base/containers/string.h>

#include "exception.h"
#include "logging.h"
#include "iconnection.h"

namespace LightMySQL {

using namespace LightSpeed;

///Authorization
struct AuthInfo_t {
	///username
	StringA username;
	///password
	StringA password;

	AuthInfo_t() {}
	AuthInfo_t(const StringA username,const StringA password)
		:username(username),password(password) {}

};

///Defines connection paramaters
struct ConnectParams {

	enum Lifetime {

		///Default behaviour, if lost - error
		defaultLifetime,
		///Tests and reconnects on begin of every transaction
		reconnectTransaction,
		///Opens connection before transaction and closes it on commit/rollback
		closeTransaction,

	};

	///hostname - if empty, socket is used
	StringA host;
	///portnumber
	natural port;
	///Authorization informations
	AuthInfo_t authInfo;
	///database name
	StringA dbname;
	///linux socket
	StringA socket;
	///defines lifetime of the connection
	/** @see Lifetime */
	Lifetime lifetime;

	ConnectParams():port(3306),lifetime(defaultLifetime) {}
	ConnectParams(ConstStrA host, natural port, const AuthInfo_t &authInfo,
			ConstStrA dbName, ConstStrA socket = ConstStrA(),
			Lifetime lifetime = defaultLifetime)
		:host(host),port(port),authInfo(authInfo),dbname(dbName),socket(socket),lifetime(lifetime) {}
	ConnectParams(const ConnectParams &other)
		:host(other.host.getMT())
		,port(other.port)
		,authInfo(other.authInfo.username.getMT(),other.authInfo.password.getMT())
		,dbname(other.dbname.getMT())
		,socket(other.socket.getMT())
		,lifetime(other.lifetime)
		{

	}
};

class Result;

///MySQL connection. First object that must be created to communicate with the database
class Connection: public LightSpeed::SharedResource, public IConnection {
public:

	///Constructor - initializes internal structures
	Connection();
	///Destructor - closes connection and frees up the memory
	~Connection();

	Connection(const ConnectParams &params, unsigned long flags = 0);

	///Connects the database server
	/**
	 * @param params parameters of connections
	 * @param flags additional flags.
	 * @note creates multi-result client
	 */
	void connect(const ConnectParams &params, unsigned long flags = 0);
	///Explicitly closes connection
	void close();

	///Retrieves last error number
	unsigned int getLastError() const;
	///Retrieves last error message
	StringA getLastErrorMsg() const;

	///Sets connection options
	/**
	 * @param opt option type
	 * @param value option value
	 * @note refer MySQL documentation mysql_options()
	 */
	void setOption(mysql_option opt, const char *value);
	///Checks, whether database server is alive
	void ping();
	///Escapes string
	/**
	 * To prevent SQL injection, you have to escape all strings passed
	 * to the query. This function escapes all dangerous characters in
	 * the string.
	 * @param str string to escape
	 * @return escaped string
	 *
	 * @see Query
	 */
	StringA escapeString(ConstStrA str);

	///Executes query
	/**
	 * @param query query to execute. Note that function
	 * 	cannot check validity of query and prevent SQL injecting.
	 *  Consider to use Query object to build-up queries
	 * @return result of query
	 */
	Result executeQuery(ConstStrA query);
	///Handles any MySQL error throwing appropriate exception
	/**
	 * @param e location in the program, where error has been retrieved.
	 * Use THISLOCATION
	 */
	void handleError(const ProgramLocation &e);

	///Checks, whether object is connected
	/**
	 * @retval true object is connected. Note that function doesn't check
	 * state of connection. It only remembers, whether connect has been called
	 * @retval false object is not connected
	 */
	bool isConnected() const {return connected;}

	///Sets logging interface
	/**
	 * @param logObject pointer to object implementing IDebugLog interface.
	 * If parameter is NULL, logging is turned off
	 */
	void setLogObject(IDebugLog *logObject);

	///Retrieves object implementing logging interface
	/**
	 * @return pointer to object. If NULL returned, logging is disabled
	 */
	IDebugLog *getLogObject() const;

	///Reconnects database
	void reconnect();

	///Starts transaction
	void startTransaction(Level isolationLevel);

	///Commits transaction
	void commitTransaction();

	///Rollbacks transaction
	void rollbackTransaction();

	natural getConnectionId();

	void killConnection(natural connectionId);


	virtual void logString(ConstStrA str, bool error);
	virtual bool isLogEnabled() const;


protected:

	friend class Result;

	mutable MYSQL conn;
	bool connected;

	IDebugLog *logObject;
	ConnectParams reconnectParams;
	unsigned long reconnectFlags;
	unsigned long transactionObjects;

	void closeTemporary();
	void openTransactionWithLevel(Level isolationLevel);
};

}


#endif /* MYSQL_CONNECTION_H_ */
