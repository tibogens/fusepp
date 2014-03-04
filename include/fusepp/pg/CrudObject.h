/*
fusepp, An extensible C++ wrapper to FUSE, the Filesystem in Userspace
Copyright (C) 2014 University of Lausanne, Switzerland
Author: Thibault Genessay
https://github.com/tibogens/fusepp

This file is part of fusepp.

fusepp is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

fusepp is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with fusepp.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef _FUSEPP_PG_CRUDOBJECT_H
#define _FUSEPP_PG_CRUDOBJECT_H

#include <fusepp/pg/Export.h>
#include <fusepp/pg/Database.h>
#include <vector>
#include <memory>

namespace fusepp
{

class FUSEPP_PG_API CrudObject;
typedef std::shared_ptr<CrudObject> CrudObjectPtr;

class FUSEPP_PG_API CrudModel
{
protected:
	enum FieldType
	{
		FT_INT			=    0x0001,
		FT_TEXT			=    0x0002,
		FT_DOUBLE		=    0x0004,
		FT_DATE			=    0x0008,
		//FT_BYTEA		=    0x0010,
		FT_BOOL			=    0x0020,

		FT_PK			=   0x10000,
	};

	CrudModel(DatabasePtr database, const std::string& name);

	void addField(const std::string& name, unsigned int type, const std::string& table = "");

	void setFindByNameColumn(const std::string& column);

	virtual CrudObject* createInstance(CrudModel* model) const;

public:
	virtual ~CrudModel();

	// Public interface (all const)
	size_t getFieldIndex(const std::string& column) const;
	

	CrudObjectPtr create() const;
	CrudObjectPtr get(int id) const;
	CrudObjectPtr findByName(const std::string& name) const;

private:
	
	struct Field
	{
		Field(const std::string& name, unsigned int type, const std::string& table) : name(name), type(type), table(table) {}
		std::string name;
		unsigned int type;
		std::string table;
	};
	typedef std::vector<Field> Fields;
	Fields _fields;
	
	typedef std::map<std::string,size_t> FieldsIndex;
	FieldsIndex _index;
	
	typedef std::vector<size_t> PrimaryKey;
	PrimaryKey _pk;

	DatabasePtr _database;
	std::string _name;
	std::string _findByNameColumn;
};

#define CRUDOBJ_GETTER(suffix, accessor, type, const_type) \
	const_type get##suffix(size_t index) const { return getValue(index)->accessor()->value; } \
	type get##suffix(size_t index) { return getValue(index)->accessor()->value; } \
	const_type get##suffix(const std::string& column) const { return getValue(column)->accessor()->value; } \
	type get##suffix(const std::string& column) { return getValue(column)->accessor()->value; }

#define CRUDOBJ_SETTER(suffix, accessor, type, const_type) \
	void set##suffix(size_t index, const_type value) { getValue(index)->accessor()->value = value; } \
	void set##suffix(const std::string& column, const_type value) { getValue(column)->accessor()->value = value; }

#define CRUDOBJ_GETTER_SETTER(suffix, accessor, type, const_type) \
	CRUDOBJ_GETTER(suffix, accessor, type, const_type) \
	CRUDOBJ_SETTER(suffix, accessor, type, const_type)


class FUSEPP_PG_API CrudObject 
{
private:
	friend class CrudModel;
	// Private constructor with friend class so that only CrudModels
	// can instantiate CrudObjects
	CrudObject(CrudModel* model);

public:
	virtual ~CrudObject();

	CRUDOBJ_GETTER_SETTER(i, asInt, int&, int);
	CRUDOBJ_GETTER_SETTER(t, asText, std::string&, const std::string&);
	CRUDOBJ_GETTER_SETTER(f, asDouble, double&, double);
	CRUDOBJ_GETTER_SETTER(d, asDate, std::string&, const std::string&);
	//CRUDOBJ_GETTER(ba, asBytea, uflr::RefBuffer<char>*, const uflr::RefBuffer<char>* const);
	CRUDOBJ_GETTER_SETTER(b, asBool, bool&, bool);

private:
	struct IntValue;
	struct TextValue;
	struct DoubleValue;
	struct DateValue;
	//struct ByteaValue;
	struct BoolValue;

	struct Value
	{
		virtual ~Value();

		IntValue* asInt();
		const IntValue* const asInt() const;
		TextValue* asText();
		const TextValue* const asText() const;
		DoubleValue* asDouble();
		const DoubleValue* const asDouble() const;
		DateValue* asDate();
		const DateValue* const asDate() const;
		//ByteaValue* asBytea();
		//const ByteaValue* const asBytea() const;
		BoolValue* asBool();
		const BoolValue* const asBool() const;
	};

	struct IntValue : public Value
	{
		IntValue() {}
		//IntValue(int value) : value(value) {}
		int value;
	};

	struct TextValue : public Value
	{
		TextValue() {}
		//TextValue(const std::string& value) : value(value) {}
		std::string value;
	};

	struct DoubleValue : public Value
	{
		DoubleValue() {}
		//DoubleValue(double value) : value(value) {}
		double value;
	};

	struct DateValue : public Value
	{
		DateValue() {}
		//DateValue(const std::string& value) : value(value) {}
		std::string value;
	};

	/*struct ByteaValue : public Value
	{
		ByteaValue() {}
		//ByteaValue(const std::string& value) : value(value) {}
		osg::ref_ptr<uflr::RefBuffer<char> > value;
	};*/

	struct BoolValue : public Value
	{
		BoolValue() {}
		//BoolValue(const std::string& value) : value(value) {}
		bool value;
	};

	const Value* const getValue(size_t index) const;
	const Value* const getValue(const std::string& column) const;
	Value* getValue(size_t index);
	Value* getValue(const std::string& column);


private:
	CrudModel* _model;
	std::vector<Value*> _values;
	
};


};

#endif // _FUSEPP_PG_CRUDOBJECT_H
