/** @file
 * Copyright (c) 2006, Seznam.cz, a.s.
 * All Rights Reserved.
 * 
 * $Id: exception.h 1550 2011-01-03 13:08:26Z ondrej.novak $
 *
 * DESCRIPTION
 * Short description
 * 
 * AUTHOR
 * Ondrej Novak <ondrej.novak@firma.seznam.cz>
 *
 */


#ifndef MYSQL_EXCEPTIONS_H_
#define MYSQL_EXCEPTIONS_H_

#include <lightspeed/base/exceptions/exception.h>
#include <lightspeed/base/containers/string.h>
#include "lightspeed/base/debug/programlocation.h"


namespace LightMySQL {


using LightSpeed::StringA;
using LightSpeed::ProgramLocation;

	///Common exception object. Catch it to catch all MySQL errors
	class Exception_t: public virtual LightSpeed::Exception {
	public:
		Exception_t(const ProgramLocation &loc) {
			this->setLocation(loc);
		}
	};


	///Exception - All errors reported by server
	/**
	 * Because MySQL server reports errors using errno and description
	 * only one class is designed to handle all these errors. To
	 * catch specific error in catch handler, catch this exception
	 * and test error number by calling getErrono(). If error is not
	 * excepted, re-throw the exception to the next handler
	 */
	class ServerError_t: public Exception_t{
	public:
		LIGHTSPEED_EXCEPTIONFINAL;

		enum GeneralError {
			///error - duplicate keys
			errDuplicateKeys,
			///error - foreign key constrain check failure
			errForeignKey,
			///unknown or undetermined error
			errUndetermined
		} ;

		///Constructs exception
		/**
		 * @param loc location of construction
		 * @param errnr error number reported by MySQL server
		 * @param errText error message reported by MySQL server
		 */
		ServerError_t(const ProgramLocation &loc,
				unsigned int errnr, const StringA &errText)
			:Exception_t(loc),errnr(errnr),errText(errText) {}

		///Retrieves error number reported by MySQL server
		unsigned int getErrno() const {return errnr;}
		///Retrieves error message reported by MySQL server
		const StringA &getErrorMsg() const {return errText;}
		///dtor
		~ServerError_t() throw () {}

		GeneralError getError() const {
			switch(getErrno()) {
			case 1062: return errDuplicateKeys;
			case 1451:
			case 1452: return errForeignKey;
			default: return errUndetermined;
			}
		}

	protected:
		unsigned int errnr;
		StringA errText;

		void message(LightSpeed::ExceptionMsg &msg) const {
			msg("MySQL server error: %1 %2") << errnr << errText.c_str();
		}
	};

	///Exception - Cannot execute query until transaction is opened
	/**
	 * To execute query through Transaction, first open transaction by
	 * calling Transaction::start()
	 */
	class UnopenedTransactionException_t: public Exception_t {
	public:
		LIGHTSPEED_EXCEPTIONFINAL;

		UnopenedTransactionException_t(const ProgramLocation &loc)
			:Exception_t(loc) {}
	protected:
		void message(LightSpeed::ExceptionMsg &msg) const;
	};

	class EnumException: public Exception_t {
	public:
		LIGHTSPEED_EXCEPTIONFINAL;

		EnumException(const ProgramLocation &loc, LightSpeed::ConstStrA table,
				LightSpeed::ConstStrA field, LightSpeed::ConstStrA errorMsg)
			:Exception_t(loc), table(table), field(field), errorMsg(errorMsg) {}


		~EnumException() throw() {}

	public:
		LightSpeed::StringA table, field, errorMsg;
		void message(LightSpeed::ExceptionMsg &msg) const {
			msg("%1(%2) %3") << table << field << errorMsg;
		}
	};
}

#endif /* MYSQL_EXCEPTIONS_H_ */
