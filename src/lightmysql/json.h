/*
  * json.h - convert result to JSON
 *
 *  Created on: 3. 7. 2016
 *      Author: ondra
 */

#ifndef LIGHTMYSQL_JSON_H_023540399701
#define LIGHTMYSQL_JSON_H_023540399701


#include "result.h"
namespace LightMySQL {


class DBResultToJSON {
public:


	DBResultToJSON(const LightSpeed::JSON::PFactory &factory, const LightMySQL::Result &result, bool strDate);
	DBResultToJSON(const LightSpeed::JSON::PFactory &factory, const LightMySQL::Result &result, bool strDate, bool noInit, bool buildArray);

	enum FieldFormat {
		///skip this field
		skip,
		///store as integer
		integer,
		///store as float
		floatnum,
		///store as date
		datetime,
		///store as string
		string,
		///store as set
		set,
		///store as set with custom separator
		setWithCustomSep1,
		///store as set with custom separator
		setWithCustomSep2,
		///store as binary (base64)
		binary,
		///store as boolean
		boolean,
		///value is json, parse it
		jsonstr
	};

	///set format by index
	void setFormat(LightSpeed::natural index, FieldFormat fld);
	///set format by name
	void setFormat(LightSpeed::ConstStrA name, FieldFormat fld);
	///rename field at index
	void rename(LightSpeed::natural index, LightSpeed::ConstStrA newName);
	///rename field
	void rename(LightSpeed::ConstStrA name, LightSpeed::ConstStrA newName);
	///make group
	/**
	 * @param prefix prefix of the group with an separator. Separator is ignored. Example "prefix_"
	 * creates the object prefix:{...} and puts everything with prefix inside (removing the prefix)
	 */
	void setGroup(LightSpeed::ConstStrA prefix);
	///Enable automatic grouping by separator
	/**
	 * @param sep specifies separator for automatic grouping. For example, use '_' cause,that a_b_c will
	 * be stored as "a":{"b":{"c":...}}
	 *
	 * To disable automatic grouping set separator to zero
	 */
	void setGroupSeparator(char sep);
	LightSpeed::JSON::PNode getRow(const LightMySQL::Row &row);
	LightSpeed::JSON::PNode operator()(const LightMySQL::Row &row) {return getRow(row);}
	void setCustomSet1(char x) {customSep1 = x;}
	void setCustomSet2(char x) {customSep2 = x;}
	LightSpeed::JSON::PNode getHeader();

	static void enableISODate(bool isoDate);
	static bool isISODateEnabled();

protected:
	struct RowItem {
		LightSpeed::ConstStrA name;
		FieldFormat format;
		LightSpeed::natural groupSep;
		RowItem (LightSpeed::ConstStrA name, FieldFormat format):name(name),format(format),groupSep(LightSpeed::naturalNull) {}
		RowItem (LightSpeed::ConstStrA name, FieldFormat format,LightSpeed::natural groupSep):name(name),format(format),groupSep(groupSep) {}
	};

	typedef LightSpeed::AutoArray<RowItem, LightSpeed::SmallAlloc<16> > RowDesc;
	LightSpeed::JSON::PFactory factory;
	const LightMySQL::Result &result;
	bool strDate;
	bool buildArray;
	RowDesc rowDesc;
	char customSep1;
	char customSep2;
	char groupSep;

private:
	void init(LightSpeed::natural count, const LightMySQL::Result& result, bool hideAl);
};

extern const char *message_InvalidDateTimeFormat;

typedef LightSpeed::GenException1<message_InvalidDateTimeFormat, ConstStrA> InvalidDateTimeFormat;

LightSpeed::JSON::PNode createArrayFromSet(LightSpeed::JSON::IFactory &factory, ConstStrA text, char sep = ',');

LightSpeed::TimeStamp parseDateTime(ConstStrA text);
LightSpeed::JSON::PNode parseDateTime(LightSpeed::JSON::IFactory &factory, ConstStrA text, bool strDate);
LightSpeed::StringA dateTimeToDB(const LightSpeed::JSON::INode &nd);
LightSpeed::StringA flagsToDB(const LightSpeed::JSON::INode &nd);


}



#endif /* LIGHTMYSQL_JSON_H_023540399701 */
