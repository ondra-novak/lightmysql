/*
 * row_std.cpp
 *
 *  Created on: 10.7.2013
 *      Author: ondra
 */

#include <string>
#include "row.h"
#include "row_std.h"

namespace LightMySQL {

std::string FieldTypeConv<std::string>::convert(const FieldContent &f) {
	return std::string(f.value,f.length);
}
std::wstring FieldTypeConv<std::wstring>::convert(const FieldContent &f) {
	ConstStrA val(f.value,f.length);
	String wval = val;
	return std::wstring(wval.data(),wval.length());
}

}
