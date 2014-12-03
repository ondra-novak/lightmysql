/** @file
 * Copyright (c) 2010, Seznam.cz, a.s.
 * All Rights Reserved.
 * 
 * $Id: cfghelper.h 1608 2011-02-04 15:53:07Z ondrej.novak $
 *
 * DESCRIPTION
 * Reads config using config parser.
 * 
 * AUTHOR
 * Ondrej Novak <ondrej.novak@firma.seznam.cz>
 *
 */


#ifndef _MYSQLWRAP_CONFIG_H_
#define _MYSQLWRAP_CONFIG_H_

#include "connection.h"
#include <lightspeed/utils/configParser.h>


namespace LightMySQL {


	void readConfig(ConnectParams &cfg, const LightSpeed::IniConfig::Section &parser);


}

#endif /* _MYSQLWRAP_CONFIG_H_ */
