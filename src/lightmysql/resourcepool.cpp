/*
 * MySQLResourcePool.cpp
 *
 *  Created on: 13.5.2013
 *      Author: ondra
 */

#include "resourcepool.h"
#include "lightspeed/utils/configParser.h"
#include "cfghelper.h"
#include "lightspeed/base/text/textParser.tcc"


namespace LightMySQL {

using namespace LightSpeed;


MySQLResourcePool::MySQLResourcePool(const ConnectParams& params, unsigned long flags,
									IDebugLog *log,
									natural limit, natural resTimeout, natural waitTimeout)
:AbstractResourcePool(limit,resTimeout,waitTimeout),params(params),flags(flags),log(log)
{

}

MySQLResource* MySQLResourcePool::createResource() {
	MySQLResource *r =  new MySQLResource;
	r->setLogObject(log);
	r->connect(params,flags);
	return r;
}


const char *noMasterConfiguredExceptionText = "No master database is configured, data are read only";

MySQLResPtr MySQLMasterSlavePool::getMaster() {
	if (master == nil) throw NoMasterDatabaseConfiguredException(THISLOCATION);
	return MySQLResPtr(*master);
}

MySQLResPtr MySQLMasterSlavePool::getSlave() {
	if (slave == nil) return getMaster();
	return MySQLResPtr(*slave);
}

static void configureDB(MySQLServerCfg &t, const LightSpeed::IniConfig::Section & cfg) {
	t.enabled = false;
	cfg.get(t.enabled,"enable");
	if (t.enabled) {
		readConfig(t.connparams,cfg);
		cfg.required(t.maxConn,"maxConnections");
		cfg.required(t.maxExpire,"expireTime");
		cfg.required(t.maxWait,"waitTimeout");
		t.connparams.lifetime = ConnectParams::reconnectTransaction;
	}
}


void MySQLConfig::configure(const LightSpeed::IniConfig& cfg, ConstStrA section) {

	IniConfig::Section s = cfg.openSection(section);
	configureDB(master,s.openSection("master"));
	configureDB(slave,s.openSection("slave"));

}

const MySQLMasterSlavePool::PoolPtr &MySQLMasterSlavePool::getMasterPool() {
	if (master == nil) throw NoMasterDatabaseConfiguredException(THISLOCATION);
	return master;
}
const MySQLMasterSlavePool::PoolPtr &MySQLMasterSlavePool::getSlavePool() {
	if (slave == nil) return master; else return slave;
}


void MySQLMasterSlavePool::init(const MySQLConfig& cfg, unsigned long flags) {
	if (cfg.master.enabled) {
		master = new MySQLResourcePool(cfg.master.connparams,flags,cfg.master.logObject,cfg.master.maxConn,cfg.master.maxExpire,cfg.master.maxWait);
	} else {
		master = nil;
	}
	if (cfg.slave.enabled) {
		slave = new MySQLResourcePool(cfg.slave.connparams,flags,cfg.slave.logObject,cfg.slave.maxConn,cfg.slave.maxExpire,cfg.slave.maxWait);
	} else {
		slave = nil;
	}
}

bool MySQLResource::expired() const {

	if (AbstractResource::expired()) return true;

	const IConnection &conn = q.getConnection();
	return !conn.isConnected();

}

} /* namespace jsonsrv */

