/*
 * iconnection.h
 *
 *  Created on: Sep 30, 2012
 *      Author: ondra
 */

#ifndef LightMySQL_ICONNECTION_H_
#define LightMySQL_ICONNECTION_H_
#include <lightspeed/base/containers/constStr.h>
#include <lightspeed/base/containers/string.h>
#include <lightspeed/base/interface.h>



namespace LightMySQL {

using namespace LightSpeed;

	class Result;

	class IConnection: public LightSpeed::IInterface {
	public:

		enum Level {
			defaultLevel,
			readUncommited,
			readCommited,
			repeatable,
			serializable
		};


		virtual Result executeQuery(ConstStrA query) = 0;
		virtual StringA escapeString(ConstStrA str) = 0;
		virtual void startTransaction(Level isolationLevel = defaultLevel) = 0;
		virtual void commitTransaction() = 0;
		virtual void rollbackTransaction() = 0;
		///Sends string to the internal logging system
		/**
		 * @param str string to log
		 * @param error true if it is error, otherwise it is logged as info
		 */
		virtual void logString(ConstStrA str, bool error) = 0;
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
