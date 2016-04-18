/*
 * enum.cpp
 *
 *  Created on: 31.5.2014
 *      Author: ondra
 */

#include "enum.h"
#include <lightspeed/base/containers/stringpool.tcc>
#include "result.h"
#include "exception.h"
#include "lightspeed/base/containers/map.tcc"


namespace LightMySQL {


void Enum::loadMap(Transaction& trn, ConstStrA table, ConstStrA col, natural invalidValue) {

	Result res = trn("SHOW COLUMNS FROM %1 LIKE %2").field(table).arg(col).exec();
	strpool.clear();
	map.clear();
	if (res.empty()) {
		throw EnumException(THISLOCATION,table,col,"Field doesn't exist");
	}
	Row rw = res.getNext();
	ConstStrA val = rw["Type"].as<ConstStrA>();
	bool isSet;
	if (val.head(4) == ConstStrA("set(")) {
		val = val.crop(4,1);
		isSet = true;
	}
	else if (val.head(5) == ConstStrA("enum(")) {
		val = val.crop(5,1);
		isSet = false;
	}
	else throw EnumException(THISLOCATION,table,col,"Field is neither SET nor ENUM");

	natural id = 0;
	for (ConstStrA::SplitIterator iter = val.split(','); iter.hasItems();) {
		ConstStrA p = iter.getNext();
		p = p.crop(1,1);
		Str str = strpool.add(p);
		natural val = isSet?(1 << id):id;
		map.insert(str,val);
		id++;
	}

	this->invalidValue = invalidValue;

}

natural Enum::operator ()(ConstStrA name) const {
	const natural *id = map.find(name);
	if (id == 0) return invalidValue;
	else return *id;
}



}
