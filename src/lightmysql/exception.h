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
#include <exception>
#include <string>

#include <imtjson/string.h>

namespace LightMySQL {

	using namespace json;


	///Common exception object. Catch it to catch all MySQL errors
	class Exception_t: public virtual std::exception {
	public:

		virtual const char *what() const throw() override {
			if (msgdata.empty()) {
				msgdata = getMessage();
			}
			return msgdata.c_str();
		}

		virtual ~Exception_t() throw() {}

		virtual String getMessage() const throw() = 0;
	private:
		mutable String msgdata;
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
		ServerError_t(unsigned int errnr, const String &errText)
			:errnr(errnr),errText(errText) {}

		///Retrieves error number reported by MySQL server
		unsigned int getErrno() const {return errnr;}
		///Retrieves error message reported by MySQL server
		const String &getErrorMsg() const {return errText;}
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
		String errText;

		String getMessage() const throw() override {
			return String({"MySQL server error: ", std::to_string(errnr), " ", errText});
		}
	};

	///Exception - Cannot execute query until transaction is opened
	/**
	 * To execute query through Transaction, first open transaction by
	 * calling Transaction::start()
	 */
	class UnopenedTransactionException_t: public Exception_t {
	public:

	protected:
		String getMessage() const throw() override {
			return "Transaction is not opened (MySQL)";
		}
	};

	class EnumException: public Exception_t {
	public:

		EnumException(StrViewA table,StrViewA field, StrViewA errorMsg)
			:table(table), field(field), errorMsg(errorMsg) {}


		~EnumException() throw() {}

	public:
		String table, field, errorMsg;
		String getMessage() const throw() override {
			return String({table,"(",field,") ",errorMsg});
		}
	};
}

#endif /* MYSQL_EXCEPTIONS_H_ */
