/*
 * cfgparser.cpp
 *
 *  Created on: Sep 24, 2012
 *      Author: ondra
 */

#include "cfghelper.h"
#include <lightspeed/utils/configParser.tcc>



void LightMySQL::readConfig(ConnectParams& cfg,
		const LightSpeed::IniConfig::Section& parser) {

	LightSpeed::integer port = (int)cfg.port;
	parser.required(cfg.authInfo.username,"username");
	parser.required(cfg.authInfo.password,"password");
	parser.required(cfg.host,"host");
	parser.get(port,"port");
	parser.get(cfg.socket,"socket");
	parser.required(cfg.dbname,"database");
	StringA lifestr;
	parser.get(lifestr,"conControl");
	if (lifestr == "standard") cfg.lifetime = ConnectParams::defaultLifetime;
	else if (lifestr == "transaction") cfg.lifetime = ConnectParams::reconnectTransaction;
	else if (lifestr == "transactionClose") cfg.lifetime = ConnectParams::closeTransaction;
	cfg.port = (unsigned int)port;
}



