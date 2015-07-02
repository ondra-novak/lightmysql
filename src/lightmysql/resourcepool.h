/*
 * MySQLResourcePool.h
 *
 *  Created on: 13.5.2013
 *      Author: ondra
 */

#ifndef LIGHTMYSQL_RESOURCEPOOL
#define LIGHTMYSQL_RESOURCEPOOL

#include "lightspeed/base/containers/resourcePool.h"
#include "connection.h"
#include "query.h"
#include "transaction.h"

namespace LightSpeed {
class IniConfig;
}

namespace LightMySQL {

///Resource which contains one mysql connection
class Resource: public AbstractResource, public Connection {
public:

	///construct mysql resource
	Resource():q(*this) {}

	///Retrieve transaction object
	/** You should use transaction object for most of the
	 * queries carried on database. Note that transaction
	 * can be used only once per one thread at time. Don't
	 * acquire transaction object until previous one is is
	 * closed
	 *
	 * @return transaction object
	 */
	Transaction getTransact() {return q;}
	///Retrieve query object
	/** Query object can be used to perform other queries
	 * that cannot be executed in the transaction.
	 *
	 * @return reference to the query object
	 */
	Query &getQueryObject() {return q;}

	virtual bool expired() const;

protected:
	Query q;
private:
	Resource(const Resource &other);
	Resource &operator=(const Resource &other);
};

typedef Resource MySQLResource;

///Pool of MYSQL resources (connection)
/** Object keeps opened connections or additionally creates
 *  fresh on the request
 */
class ResourcePool: public AbstractResourcePool {
public:
	///Construction of the resource pool
	/**
	 * @param params connection parameters (see Connection object)
	 * @param flags connection flags (see Connection object)
	 * @param log pointer to IDebugLog object which will be set to newly created connection
	 * @param limit specifies maximum count of opened connections at the time
	 * @param resTimeout defines how long connection can be opened (in miliseconds)
	 * @param waitTimeout defines how long thread can wait to acquire connection
	 *        before exception is thrown
	 */
	ResourcePool(const ConnectParams &params,
			unsigned long flags,
			IDebugLog *log,
			natural limit, natural resTimeout, natural waitTimeout);

	///creates resource
	/** You can call this function directly if you want to create
	 * new resource outside of pool. Newly created resource is not counted into
	 * pool limits and you should never try to release such connection to that pool.
	 * If you no longer need such connection, use delete operator on its pointer
	 *
	 * To retrieve connection from the pool, use MySqlResPtr
	 *
	 * @return pointer to created resource
	 */
	virtual Resource *createResource();
protected:
	virtual const char *getResourceName() const {return "mysql connection";}


protected:

	ConnectParams params;
	unsigned long flags;
	IDebugLog *log;

};

typedef ResourcePool MySQLResourcePool;

///Pointer to resource acquired from the pool.
/** Acquired resource on creation and releases resource on destruction */
typedef ResourcePtr<Resource> MySQLResPtr;
typedef ResourcePtr<Resource> ResPtr;

///Complete configuration of the mysql resource in the pool
struct ServerCfg {
	///true whether this configuration is enabled (and valid)
	bool enabled;
	///connection params
	ConnectParams connparams;
	///maximum count of connections
	natural maxConn;
	///maximum wait for the connection in the milliseconds
	natural maxWait;
	///how long resource is valid in milliseconds
	natural maxExpire;
	///pointer to log object
	Pointer<IDebugLog> logObject;


};

typedef ServerCfg MySQLServerCfg;

///Complete configuration of dual mysql architecture - master/slave
struct MySQLConfig {
	///configuration for master and slave
	ServerCfg master,slave;


	///reads configuration from the IniConfig
	/**
	 * @param cfg reference to the configuration
	 * @param section section name
	 */
	void configure(const IniConfig &cfg, ConstStrA section);
};

///Pool of the mysql connection for dual (master/slave) architecture
class MasterSlavePool {
public:
	typedef AllocPointer<ResourcePool> PoolPtr;


	///Initializes pool using configuration
	/**
	 * @param cfg reference to configuration
	 * @param flags flags
	 */
	void init(const MySQLConfig &cfg, unsigned long flags);

	///retrieves master database
	/** Master database must be enabled. If it is not enabled, function throws exception
	 *
	 * @return pointer to mysql-resource object which can be used to execute queries
	 * @exception NoMasterDatabaseConfiguredException master database is not configured (disabled)
	 */
	MySQLResPtr getMaster();
	///retrieves slave database
	/**
	 * @return pointer to mysql-resource object which can be used to execute queries. if
	 * slave database is not configured, function takes resource pointer from the master
	 * pool.
	 *
	 * @note you should use slave connection to execute R/O queries, In most of the case
	 * it should be only queries with command "SELECT"
	 *
	 */
	MySQLResPtr getSlave();
	///retrieves pointer to connection which can execute R/O commands
	/**
	 * @return pointer to mysql-resource object which can be used to execute queries. if
	 * slave database is not configured, function takes resource pointer from the master
	 * pool.
	 *
	 * @note you should use slave connection to execute R/O queries, In most of the case
	 * it should be only queries with command "SELECT"
	 *
	 * @note there can be a delay after writing while data arrive to the R/O connection
	 *
	 */
	MySQLResPtr getROConn() {return getSlave();}
	///retrieves pointer to connection which can execute R/W commands
	/** Master database must be enabled. If it is not enabled, function throws exception
	 *
	 * @return pointer to mysql-resource object which can be used to execute queries.
	 * @exception NoMasterDatabaseConfiguredException master database is not configured (disabled)
	 *
	 * @note for the best performance, you should use getROConn to execute R/O commands. Also
	 * note, that there can be delay after R/W command when changes are propagated to the
	 * slave database.
	 *
	 * @note don't use R/O connection to perform SELECTs with the locking request. they
	 * are not propagated to the master
	 */
	MySQLResPtr getRWConn() {return getMaster();}

	///Retrieves reference to the master pool
	/** If you need to access pool directly */
	const PoolPtr &getMasterPool();
	///Retrieves reference to the slave pool
	/** If you need to access pool directly */
	const PoolPtr &getSlavePool();

protected:
	PoolPtr master;
	PoolPtr slave;

};

typedef MasterSlavePool MySQLMasterSlavePool;

extern const char *noMasterConfiguredExceptionText;
typedef GenException<noMasterConfiguredExceptionText> NoMasterDatabaseConfiguredException;

} /* namespace jsonsrv */
#endif /* LIGHTMYSQL_RESOURCEPOOL */
