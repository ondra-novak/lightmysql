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


ResourcePool::ResourcePool(const ConnectParams& params, unsigned long flags,
									IDebugLog *log,
									natural limit, natural resTimeout, natural waitTimeout)
:AbstractResourcePool(limit,resTimeout,waitTimeout),params(params),flags(flags),log(log)
{

}

Resource* ResourcePool::createResource() {
	Resource *r =  new Resource;
	r->setLogObject(log);
	r->connect(params,flags);
	return r;
}


const char *noMasterConfiguredExceptionText = "No master database is configured, data are read only";

MySQLResPtr MasterSlavePool::getMaster() {
	if (master == nil) throw NoMasterDatabaseConfiguredException(THISLOCATION);
	return MySQLResPtr(*master);
}

MySQLResPtr MasterSlavePool::getSlave() {
	if (slave == nil) return getMaster();
	return MySQLResPtr(*slave);
}

static void configureDB(ServerCfg &t, const LightSpeed::IniConfig::Section & cfg) {
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

const MasterSlavePool::PoolPtr &MasterSlavePool::getMasterPool() {
	if (master == nil) throw NoMasterDatabaseConfiguredException(THISLOCATION);
	return master;
}
const MasterSlavePool::PoolPtr &MasterSlavePool::getSlavePool() {
	if (slave == nil) return master; else return slave;
}


void MasterSlavePool::init(const MySQLConfig& cfg, unsigned long flags) {
	if (cfg.master.enabled) {
		master = new ResourcePool(cfg.master.connparams,flags,cfg.master.logObject,cfg.master.maxConn,cfg.master.maxExpire,cfg.master.maxWait);
	} else {
		master = nil;
	}
	if (cfg.slave.enabled) {
		slave = new ResourcePool(cfg.slave.connparams,flags,cfg.slave.logObject,cfg.slave.maxConn,cfg.slave.maxExpire,cfg.slave.maxWait);
	} else {
		slave = nil;
	}
}

bool Resource::expired() const {

	if (AbstractResource::expired()) return true;

	const IConnection &conn = q.getConnection();
	return !conn.isConnected();

}

} /* namespace jsonsrv */

