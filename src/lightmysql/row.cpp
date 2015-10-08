/** @file
 * Copyright (c) 2006, Seznam.cz, a.s.
 * All Rights Reserved.
 * 
 * $Id: row.cc 1723 2011-04-28 12:12:29Z ondrej.novak $
 *
 * DESCRIPTION
 * Short description
 * 
 * AUTHOR
 * Ondrej Novak <ondrej.novak@firma.seznam.cz>
 *
 */

#include "row.h"
#include "result.h"
#include <lightspeed/base/exceptions/throws.tcc>
#include <lightspeed/base/exceptions/invalidNumberFormat.h>
#include <string.h>
//#include <lightspeed/base/streams/text.tcc>

namespace LightMySQL {

void FieldContent::checkNull() const {
	if (value == 0) throw NullFieldException_t(THISLOCATION,owner.owner.getFieldName(pos));
}


FieldContent Row::operator [](int index) const {
	if (index < 0 || index >= (int)count)
		throwRangeException_FromTo(THISLOCATION,0,(int)count-1,index);

	return FieldContent(row[index],lengths[index],index,*this);

}

FieldContent Row::operator [](ConstStrA fieldName) const {
	for (natural i = 0; i < count; ++i) {
		const MYSQL_FIELD *info = owner.getFieldInfo(i);
		if (fieldName == ConstStrA(info->name,info->name_length)) return operator[](i);
	}
	throw FieldNotFoundException_t(THISLOCATION,fieldName);
}

unsigned int FieldTypeConv<bool>::convert(const FieldContent& f)
{
	char *endp;
	unsigned int r = (unsigned int)strtoul(f.value,&endp,10);
	if (*endp) throw InvalidNumberFormatException(THISLOCATION,String(f.value));
	return r != 0;
}
unsigned int FieldTypeConv<unsigned int>::convert(const FieldContent& f)
{
	char *endp;
	unsigned int r = (unsigned int)strtoul(f.value,&endp,10);
	if (*endp) throw InvalidNumberFormatException(THISLOCATION,String(f.value));
	return r;
}

signed int FieldTypeConv<signed int>::convert(const FieldContent& f)
{
	char *endp;
	signed int r = (signed int)strtol(f.value,&endp,10);
	if (*endp) throw InvalidNumberFormatException(THISLOCATION,String(f.value));
	return r;
}

unsigned long FieldTypeConv<unsigned long int>::convert(const FieldContent& f)
{
	char *endp;
	unsigned long r = strtol(f.value,&endp,10);
	if (*endp) throw InvalidNumberFormatException(THISLOCATION,String(f.value));
	return r;
}

signed long FieldTypeConv<signed long int>::convert(const FieldContent& f)
{
	char *endp;
	signed  long r = strtol(f.value,&endp,10);
	if (*endp) throw InvalidNumberFormatException(THISLOCATION,String(f.value));
	return r;
}

unsigned long long FieldTypeConv<unsigned long long int>::convert(const FieldContent& f)
{
	char *endp;
	unsigned long long r = strtoull(f.value,&endp,10);
	if (*endp) throw InvalidNumberFormatException(THISLOCATION,String(f.value));
	return r;
}

signed long long FieldTypeConv<signed long long int>::convert(const FieldContent& f)
{
	char *endp;
	signed long long r = strtoll(f.value,&endp,10);
	if (*endp) throw InvalidNumberFormatException(THISLOCATION,String(f.value));
	return r;
}

const char* FieldTypeConv<const char *>::convert(const FieldContent& f)
{
	return f.value;
}

ConstStrA FieldTypeConv<ConstStrA>::convert(const FieldContent& f)
{
	return ConstStrA(f.value,f.length);
}

StringA FieldTypeConv<StringA>::convert(const FieldContent& f)
{
	return StringA(ConstStrA(f.value,f.length));
}

String FieldTypeConv<String>::convert(const FieldContent& f)
{
	return String(ConstStrA(f.value,f.length));
}

float FieldTypeConv<float>::convert(const FieldContent& f)
{
	char *endp;
	float r = strtof(f.value,&endp);
	if (*endp) throw InvalidNumberFormatException(THISLOCATION,String(f.value));
	return r;
}

double FieldTypeConv<double>::convert(const FieldContent& f)
{
	char *endp;
	double r = strtod(f.value,&endp);
	if (*endp) throw InvalidNumberFormatException(THISLOCATION,String(f.value));
	return r;
}

TimeStamp FieldTypeConv<TimeStamp>::convert(const FieldContent& f)
{
	ConstStrA txt (f.value,f.length);
	return TimeStamp::fromDBDate(txt);
}


#if 0

FieldContent Row::getNext() {
	if (!hasItems())
		LightSpeed::throwIteratorNoMoreItems(THISLOCATION,typeid(FieldContent));
	unsigned int idx = index++;
	return FieldContent(row[idx],lengths[idx]);
}
bool Row::hasItems() const {
	return index < count;
}

template<typename T>
Row &Row::parse(T &v, const char *fmt) {
	unsigned int pos = index;
	FieldContent val = getNext();
	if (val.isNull()) throw NullFieldException_t(THISLOCATION,owner.getFieldName(pos));
	sscanf(val.value,fmt,&v);
	return *this;
}



Row & Row::operator >>(ConstStrA & v)
{
	unsigned int pos = index;
	FieldContent val = getNext();
	if (val.isNull()) throw NullFieldException_t(THISLOCATION,owner.getFieldName(pos));
	v = ConstStrA(val.value, val.length);
	return *this;
}


Row & Row::operator >>(signed int & v)
{ return parse(v,"%d");}
Row & Row::operator >>(unsigned int & v)
{ return parse(v,"%u");}
Row & Row::operator >>(signed short & v)
{ return parse(v,"%hd");}
Row & Row::operator >>(unsigned short & v)
{ return parse(v,"%hu");}
Row & Row::operator >>(signed long & v)
{ return parse(v,"%ld");}
Row & Row::operator >>(unsigned long & v)
{ return parse(v,"%lu");}
Row & Row::operator >>(signed long long & v)
{ return parse(v,"%ld");}
Row & Row::operator >>(unsigned long long & v)
{ return parse(v,"%lu");}
Row & Row::operator >>(float & v)
{ return parse(v,"%f");}
Row & Row::operator >>(double & v)
{ return parse(v,"%lf");}




Row &Row::operator[](int index) {
	if ((natural)index >= count)
		LightSpeed::throwRangeException_To(THISLOCATION,count,(natural)index);
	this->index = (natural)index;
	return *this;
}


bool Row::isNull() const {
	if (!hasItems())
		LightSpeed::throwIteratorNoMoreItems(THISLOCATION,typeid(FieldContent));
	return row[index] == 0;
}
#endif

}


