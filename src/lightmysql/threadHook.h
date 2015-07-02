/*
 * threadHook.h
 *
 *  Created on: 6. 2. 2015
 *      Author: ondra
 */

#ifndef LIGHTMYSQL_THREADHOOK_H_
#define LIGHTMYSQL_THREADHOOK_H_

#include <lightspeed/mt/threadHook.h>

namespace LightMySQL {


class ThreadHook: public LightSpeed::AbstractThreadHook {
public:
	ThreadHook();
	virtual void onThreadInit(LightSpeed::Thread &);
	virtual void onThreadExit(LightSpeed::Thread &);
	virtual void onThreadException(LightSpeed::Thread &) throw();

};

} /* namespace LightMySQL */

#endif /* LIGHTMYSQL_THREADHOOK_H_ */
