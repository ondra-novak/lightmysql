/*
 * enum.h
 *
 *  Created on: 31.5.2014
 *      Author: ondra
 */

#ifndef LIGHTMYSQL_ENUM_H_
#define LIGHTMYSQL_ENUM_H_

#include <lightspeed/base/containers/stringpool.h>
#include <lightspeed/base/containers/map.h>
#include "transaction.h"

namespace LightMySQL {

	using namespace LightSpeed;

	class Enum {
	public:
		Enum():invalidValue(naturalNull) {}

		void loadMap(Transaction &trn, ConstStrA table, ConstStrA col, natural invalidValue = naturalNull);
		natural operator()(ConstStrA name) const;

		bool empty() const {return map.empty();}
	protected:
		typedef StringPoolA::Str Str;
		typedef Map<Str, natural> StrMap;
		StringPoolA strpool;
		StrMap map;
		natural invalidValue;
	};




}


#endif /* LIGHTMYSQL_ENUM_H_ */
