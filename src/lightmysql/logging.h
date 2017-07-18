/** @file
 * Copyright (c) 2006, Seznam.cz, a.s.
 * All Rights Reserved.
 * 
 * $Id: logging.h 1612 2011-02-11 14:25:19Z ondrej.novak $
 *
 * DESCRIPTION
 * Short description
 * 
 * AUTHOR
 * Ondrej Novak <ondrej.novak@firma.seznam.cz>
 *
 */


#ifndef MYSQL_LOGGING_H_
#define MYSQL_LOGGING_H_
#include <imtjson/string.h>



#pragma once

namespace LightMySQL {

using json::StrViewA;

	///Interface to handle logging queries and results
	class IDebugLog {
	public:

		///Logs query before it processed
		virtual void onQueryExec(StrViewA queryText) = 0;
		///Logs error after query is unsucessfully processed
		virtual void onQueryError(StrViewA errorText) = 0;
		///Logs brief information about processed query
		virtual void onQueryResult(my_ulonglong rows, my_ulonglong cols, my_ulonglong affected, my_ulonglong lastId, my_ulonglong warn) = 0;
		///Logs brief information about processed query
		virtual void onQueryInfo(StrViewA info) = 0;
		///Logs information about server connect state change
		virtual void serverConnect(StrViewA host, int port, StrViewA dbname) = 0;
		///Logs information about server connect state change
		virtual void serverDisconnect() = 0;

		virtual ~IDebugLog() {}

	};

}
#endif /* MYSQL_LOGGING_H_ */
