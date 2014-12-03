/*
 * MySQLReport.h
 *
 *  Created on: 14.5.2013
 *      Author: ondra
 */

#ifndef LIGHTMYSQL_BREDY_MYDBGLOG_H__
#define LIGHTMYSQL_BREDY_MYDBGLOG_H__
#include "logging.h"
#include "lightspeed/base/containers/constStr.h"


namespace LightMySQL {

class MySQLReport: public LightMySQL::IDebugLog  {
public:
	MySQLReport(bool master = true);

protected:
	///Logs query before it processed
	virtual void onQueryExec(LightSpeed::ConstStrA queryText) ;
	///Logs error after query is unsucessfully processed
	virtual void onQueryError(LightSpeed::ConstStrA errorText) ;
	///Logs brief information about processed query
	virtual void onQueryResult(my_ulonglong rows, my_ulonglong cols, my_ulonglong affected, my_ulonglong lastId, my_ulonglong warn) ;
	///Logs brief information about processed query
	virtual void onQueryInfo(LightSpeed::ConstStrA info) ;
	///Logs information about server connect state change
	virtual void serverConnect(LightSpeed::ConstStrA host, int port, LightSpeed::ConstStrA dbname) ;
	///Logs information about server connect state change
	virtual void serverDisconnect() ;
bool master;

};

} /* namespace jsonsrv */
#endif /* LIGHTMYSQL_BREDY_MYDBGLOG_H__ */
