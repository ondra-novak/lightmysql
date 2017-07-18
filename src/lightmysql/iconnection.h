/*
 * iconnection.h
 *
 *  Created on: Sep 30, 2012
 *      Author: ondra
 */

#ifndef LightMySQL_ICONNECTION_H_
#define LightMySQL_ICONNECTION_H_

#include <imtjson/string.h>


namespace LightMySQL {

using json::StrViewA;
using json::String;


	class Result;

	class IConnection {
	public:

		enum Level {
			defaultLevel,
			readUncommited,
			readCommited,
			repeatable,
			serializable
		};


		virtual Result executeQuery(StrViewA query) = 0;
		virtual String escapeString(StrViewA str) = 0;
		virtual void startTransaction(Level isolationLevel = defaultLevel) = 0;
		virtual void commitTransaction() = 0;
		virtual void rollbackTransaction() = 0;
		///Sends string to the internal logging system
		/**
		 * @param str string to log
		 * @param error true if it is error, otherwise it is logged as info
		 */
		virtual void logString(StrViewA str, bool error) = 0;
		///Determines, whether logging is enabled
		/**
		 * @retval true logging is enabled
		 * @retval false logging is not enabled
		 */
		virtual bool isLogEnabled() const = 0;
		virtual bool isConnected() const = 0;



	};

}



#endif /* LightMySQL_ICONNECTION_H_ */
