/*
 * row_std.h
 *
 *  Created on: 10.7.2013
 *      Author: ondra
 */

#ifndef LIGHTMYSQW_ROW_STD_H_
#define LIGHTMYSQW_ROW_STD_H_

#include "row.h"

namespace LightMySQL {

template<> struct FieldTypeConv<std::string> {static std::string convert(const FieldContent &f);};
template<> struct FieldTypeConv<std::wstring> {static std::wstring convert(const FieldContent &f);};



}



#endif /* LIGHTMYSQW_ROW_STD_H_ */
