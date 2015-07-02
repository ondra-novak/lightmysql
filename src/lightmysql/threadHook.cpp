/*
 * threadHook.cpp
 *
 *  Created on: 6. 2. 2015
 *      Author: ondra
 */

#include "threadHook.h"

#include <mysql/mysql.h>
namespace LightMySQL {

ThreadHook::ThreadHook() {

}

void ThreadHook::onThreadInit(LightSpeed::Thread&) {
	mysql_thread_init();
}

void ThreadHook::onThreadExit(LightSpeed::Thread&) {
	mysql_thread_end();
}

void ThreadHook::onThreadException(LightSpeed::Thread&) throw () {
	mysql_thread_end();
}


} /* namespace LightMySQL */
