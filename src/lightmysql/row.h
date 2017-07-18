#ifndef MYSQL_ROW_OBJECT
#define MYSQL_ROW_OBJECT

#pragma once

#include "connection.h"

namespace LightMySQL {

class Result;

template<typename T>
struct FieldTypeConv;

class Row;

///General object holding content of field
/** content is referenced as pointer to the memory and length
 * Copying this object causes that only reference will be copied.
 *
 * Object is valid until result is destroyed
 */
class FieldContent {
	///Pointer to begin of content.
	/** In most of case, content is stored as string with terminating zero */
	const char *value;
	///Length of contnet in characters, not including terminating zero
	unsigned long length;

	std::uintptr_t pos;

	const Row &owner;

	template<typename T>
	friend struct FieldTypeConv;

public:

	///Constructs object
	/**
	 * @param value pointer to content
	 * @param length length of content in bytes
	 */
	FieldContent(const char *value,unsigned long length, std::uintptr_t pos, const Row &owner)
		:value(value),length(length),pos(pos),owner(owner) {}

	///Tests, whether object is NULL
	/**
	 * @retval true object contains NULL value
	 * @retval false object contains a valid value
	 */
	bool isNull() const {return value == 0;}

	void checkNull() const;

	template<typename T>
	operator T() const {return as<T>();}


	template<typename T>
	T as() const;

	const Row &getRow() const {return owner;}

	std::uintptr_t getPos() const {return pos;}

	bool empty() const {return length == 0;}
};

///Object that provides access to fields in the row.
/**
 * Object works as iterator. Every read of field value
 * advances internal position to next field. You can
 * anytime set internal position to specified field
 *
 * You can use operator >> to store values, or iterator
 * can be assigned to variable. It always increases
 * internal position.
 *
 * To receive NULL field, use getNull() to test, whether field is NULL
 *
 *
 */
class Row {
public:


	///Seeks to field referred by index
	/**
	 * @param index zero based index of the field.
	 * @return As result, function moves internal
	 * 	pointer to specified field. Return value allows to
	 *  use cast operator to retrieve value of the field
	 * @exception RangeException when index if out of range
	 */
	FieldContent operator[](int index) const;
	///Seeks to field referred by field-name
	/**
	 * @param fieldName name of field
	 * @return As result, function moves internal
	 * 	pointer to specified field. Return value allows to
	 *  use cast operator to retrieve value of the field
	 */
	FieldContent operator[](StrViewA fieldName) const;

	///Retrieves count of fields in the result
	/**
	 * @return count of fields
	 */
	std::uintptr_t size() const {return count;}

	///Retrieves owner of this row.
	/**
	 * Function allows to give a caller access to the owner
	 * object for additional requests
	 *
	 * @return reference to the owner
	 */
	Result &getResultObject() {return owner;}

protected:
	///owner of this row
	Result &owner;
	///Current MySQL row
	MYSQL_ROW row;
	///array of lengths
	unsigned long *lengths;
	///count of fields
	std::uintptr_t count;

	///Constructor - cannot be called directly
	Row(Result &owner, MYSQL_ROW row, unsigned long *lengths,unsigned int count)
		:owner(owner),row(row),lengths(lengths),count(count) {}

	friend class Result;
	friend class FieldContent;
private:
	void operator=(const Result &other);
};

template<> struct FieldTypeConv<bool> {static bool convert(const FieldContent &f);};
template<> struct FieldTypeConv<unsigned int> {static unsigned int convert(const FieldContent &f);};
template<> struct FieldTypeConv<signed int> {static signed int convert(const FieldContent &f);};
template<> struct FieldTypeConv<unsigned long> {static unsigned long convert(const FieldContent &f);};
template<> struct FieldTypeConv<signed long> {static signed long convert(const FieldContent &f);};
template<> struct FieldTypeConv<unsigned long long> {static unsigned long long convert(const FieldContent &f);};
template<> struct FieldTypeConv<signed long long> {static signed long long convert(const FieldContent &f);};
template<> struct FieldTypeConv<const char *> {static const char *convert(const FieldContent &f);};
template<> struct FieldTypeConv<StrViewA> {static StrViewA convert(const FieldContent &f);};
template<> struct FieldTypeConv<BinaryView> {static BinaryView convert(const FieldContent &f);};
template<> struct FieldTypeConv<float> {static float convert(const FieldContent &f);};
template<> struct FieldTypeConv<double> {static double convert(const FieldContent &f);};


template<typename T>
T FieldContent::as() const {
	checkNull();
	return FieldTypeConv<T>::convert(*this);
}

}


#endif

