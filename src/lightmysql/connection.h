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
#include <imtjson/string.h>

#pragma once

#include <mysql/mysql.h>

#include "exception.h"
#include "logging.h"
#include "iconnection.h"

namespace LightMySQL {

using json::String;
using json::StrViewA;

///Authorization
struct AuthInfo_t {
	///username
	String username;
	///pasword
	String password;

	AuthInfo_t() {}
	AuthInfo_t(const String username,const String password)
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
	String host;
	///portnumber
	std::uintptr_t port;
	///Authorization informations
	AuthInfo_t authInfo;
	///database name
	String dbname;
	///linux socket
	String socket;
	///defines lifetime of the connection
	/** @see Lifetime */
	Lifetime lifetime;

	ConnectParams():port(3306),lifetime(defaultLifetime) {}
	ConnectParams(StrViewA host, std::uintptr_t port, const AuthInfo_t &authInfo,
			StrViewA dbName, StrViewA socket = StrViewA(),
			Lifetime lifetime = defaultLifetime)
		:host(host),port(port),authInfo(authInfo),dbname(dbName),socket(socket),lifetime(lifetime) {}
};

class Result;

///MySQL connection. First object that must be created to communicate with the database
class Connection: public IConnection {
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
	String getLastErrorMsg() const;

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
	String escapeString(StrViewA str);

	///Executes query
	/**
	 * @param query query to execute. Note that function
	 * 	cannot check validity of query and prevent SQL injecting.
	 *  Consider to use Query object to build-up queries
	 * @return result of query
	 */
	Result executeQuery(StrViewA query);
	///Handles any MySQL error throwing appropriate exception
	/**
	 */
	void handleError();

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

	std::uintptr_t getConnectionId();

	void killConnection(std::uintptr_t connectionId);


	virtual void logString(StrViewA str, bool error);
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
