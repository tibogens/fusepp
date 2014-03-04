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

#include <fusepp/pg/CrudObject.h>
#include <fusepp/Error.h>
#include <fusepp/pg/Query.h>
#include <assert.h>
#include <set>
using namespace fusepp;

// ===========================================================================
// CrudModel implementation
// ===========================================================================

CrudModel::CrudModel(DatabasePtr database, const std::string& name)
	: _database(database), _name(name)
{
	assert(database);
}

CrudModel::~CrudModel()
{
}


void CrudModel::addField(const std::string& name, unsigned int type, const std::string& table)
{
	Field field(name, type, table);
	if (table.empty())
		field.table = _name;

	_fields.push_back(field);

	size_t index = _fields.size() - 1;
	_index[field.name] = index;

	if (field.type & FT_PK)
		_pk.push_back(index);
}

void CrudModel::setFindByNameColumn(const std::string& column)
{
	_findByNameColumn = column;
}

CrudObject* CrudModel::createInstance(CrudModel* model) const
{
	return new CrudObject(model);
}

size_t CrudModel::getFieldIndex(const std::string& column) const
{
	FieldsIndex::const_iterator it = _index.find(column);
	if (it == _index.end())
		throw fusepp::Error("fusepp::CrudModel::getIndex() : column %s does not exist", column.c_str());
	assert(it->second < _fields.size());
	return it->second;
}

CrudObjectPtr CrudModel::create() const
{
	CrudObjectPtr obj(createInstance(const_cast<CrudModel*>(this)));

	for (Fields::const_iterator it = _fields.begin(); it != _fields.end(); ++it)
	{
		switch (it->type & 0xffff)
		{
		case FT_INT:
			obj->_values.push_back(new CrudObject::IntValue());
			break;
		case FT_TEXT:
			obj->_values.push_back(new CrudObject::TextValue());
			break;
		case FT_DOUBLE:
			obj->_values.push_back(new CrudObject::DoubleValue());
			break;
		case FT_DATE:
			obj->_values.push_back(new CrudObject::DateValue());
			break;
		//case FT_BYTEA:
		//	obj->_values.push_back(new CrudObject::ByteaValue());
		//	break;
		case FT_BOOL:
			obj->_values.push_back(new CrudObject::BoolValue());
			break;
		default:
			throw fusepp::Error("Unknown or unhandled field type: %d", (it->type & 0xffff));
		}
	}
	
	assert(obj->_values.size() == _fields.size());

	return obj;
}

CrudObjectPtr CrudModel::get(int id) const
{
	assert(_pk.empty() || _pk[0] < _fields.size());
	if (_pk.size() != 1 || _pk[0] >= _fields.size() || ((_fields[_pk[0]].type)&0xffff) != FT_INT)
		throw fusepp::Error("fusepp::CrudModel::get(int) : only models with a single integer primary key can load objects this way");

	CrudObjectPtr obj = create();

	bool hasBinaryColumns = false;
	for (int pass=0;pass<2;++pass)
	{
		bool isBinaryPass = pass == 1;
	
		Query query(_database);
		query << "SELECT ";
		
		std::set<std::string> fromTables;

		typedef std::pair<int,const Field*> QueryField;
		typedef std::vector<QueryField> QueryFields;
		QueryFields queryFields;
		queryFields.reserve(_fields.size());

		for (size_t i=0; i<_fields.size(); ++i)
		{
			const Field& field(_fields[i]);

			/*if (!isBinaryPass && field.type == FT_BYTEA)
			{
				hasBinaryColumns = true;
				continue;
			}
			else if (isBinaryPass && field.type != FT_BYTEA)
				continue;*/
			if (isBinaryPass)
				continue;

			if (!queryFields.empty()) query << ", ";
			
			query << field.table << "." << field.name;
			queryFields.push_back(QueryField(i,&field));
			
			fromTables.insert(field.table);
		}

		query << " FROM ";
		int i=0;
		for (std::set<std::string>::const_iterator it = fromTables.begin(); it != fromTables.end(); ++it)
			query << (i++>0?", ":"") << *it;

		const Field& pk(_fields[_pk[0]]);
		query << " WHERE " << pk.table << "." << pk.name << " = " << id;
		
		while (fromTables.size() > 1)
		{
			query << " AND ";
			i=0;
			for (std::set<std::string>::const_iterator it = fromTables.begin(); it != fromTables.end(); ++it)
			{
				if (i == 1) query << " = ";
				query << *it << "." << pk.name;
				if (i >= 2) break;
				++i;
			}
			fromTables.erase(fromTables.begin());
		}

		query << ";";

		if (isBinaryPass)
			query.setResultTypeBinary();

		query.execute(true);
		if (query.getRowsCount() != 1)
			throw fusepp::Error("fusepp::CrudModel::get() : no object was found with ID = %d", id);

		for (size_t i=0; i<queryFields.size(); ++i)
		{
			
			const QueryField& qfield(queryFields[i]);
			const Field& field(*(qfield.second));
			const int& fieldIndex = qfield.first;

			switch (field.type & 0xffff)
			{
			case FT_INT:
				obj->seti(fieldIndex, query.ati(0, i));
				break;
			case FT_TEXT:
				obj->sett(fieldIndex, query.at(0, i));
				break;
			case FT_DOUBLE:
				obj->setf(fieldIndex, query.atf(0, i));
				break;
			case FT_DATE:
				obj->setd(fieldIndex, query.at(0, i));
				break;
			//case FT_BYTEA:
			//	obj->getValue(fieldIndex)->asBytea()->value = query.getBinaryResult(0, i);
			//	break;
			case FT_BOOL:
				obj->getValue(fieldIndex)->asBool()->value = query.atb(0, i);
				break;
			default:
				throw fusepp::Error("Unknown or unhandled field type: %d", (field.type & 0xffff));
			}
		}

		if (!hasBinaryColumns)
			break;
	}
	return obj;
}

CrudObjectPtr CrudModel::findByName(const std::string& name) const
{
	if (_findByNameColumn.empty())
		throw fusepp::Error("fusepp::CrudModel::findByName() : the model must have a name column set through setFindByNameColumn()");

	assert(_pk.empty() || _pk[0] < _fields.size());
	if (_pk.size() != 1 || _pk[0] >= _fields.size() || ((_fields[_pk[0]].type)&0xffff) != FT_INT)
		throw fusepp::Error("fusepp::CrudModel::findByName() : only models with a single integer primary key can load objects this way");

	Query query(_database);
	query << "SELECT " << _fields[_pk[0]].table << "." << _fields[_pk[0]].name 
		<< " FROM " << _name 
		<< " WHERE " << _findByNameColumn << " = E'" << _database->escape(name.c_str()) << "';";
	query.execute(true);

	if (query.getRowsCount() != 1)
		throw fusepp::Error("fusepp::CrudModel::findByName() : no entity found with name %s", name.c_str());

	int id = query.ati(0,0);

	return get(id);
}

// ===========================================================================
// CrudObject implementation
// ===========================================================================

CrudObject::CrudObject(CrudModel* model)
	: _model(model)
{
	assert(_model);
}

CrudObject::~CrudObject()
{
	for (std::vector<Value*>::iterator it = _values.begin(); it != _values.end(); ++it)
		delete *it;
	_values.clear();
}

const CrudObject::Value* const CrudObject::getValue(size_t index) const
{
	if (index >= _values.size())
		throw fusepp::Error("fusepp::CrudObject::getValue(%u) : index out of range (max=%i)", index, (int)_values.size()-1);
	const Value* const val = _values[index];
	return val;
}

const CrudObject::Value* const CrudObject::getValue(const std::string& column) const
{
	assert(_model);
	size_t index = _model->getFieldIndex(column);
	if (index >= _values.size())
		throw fusepp::Error("fusepp::CrudObject::getValue(%u) : index out of range (max=%i)", index, (int)_values.size()-1);
	const Value* const val = _values[index];
	return val;
}

CrudObject::Value* CrudObject::getValue(size_t index)
{
	if (index >= _values.size())
		throw fusepp::Error("fusepp::CrudObject::getValue(%u) : index out of range (max=%i)", index, (int)_values.size()-1);
	Value* val = _values[index];
	return val;
}

CrudObject::Value* CrudObject::getValue(const std::string& column)
{
	assert(_model);
	size_t index = _model->getFieldIndex(column);
	if (index >= _values.size())
		throw fusepp::Error("fusepp::CrudObject::getValue(%u) : index out of range (max=%i)", index, (int)_values.size()-1);
	Value* val = _values[index];
	return val;
}
// ===========================================================================
// CrudObject::Value implementation
// ===========================================================================

CrudObject::Value::~Value()
{
}

CrudObject::IntValue* CrudObject::Value::asInt()
{
	IntValue* instance = dynamic_cast<IntValue*>(this);
	if (!instance)
		throw fusepp::Error("fusepp::CrudObject::Value::asInt() : value is not an integer");
	return instance;
}

const CrudObject::IntValue* const CrudObject::Value::asInt() const
{
	const IntValue* const instance = dynamic_cast<const IntValue* const>(this);
	if (!instance)
		throw fusepp::Error("fusepp::CrudObject::Value::asInt() : value is not an integer");
	return instance;
}

CrudObject::TextValue* CrudObject::Value::asText()
{
	TextValue* instance = dynamic_cast<TextValue*>(this);
	if (!instance)
		throw fusepp::Error("fusepp::CrudObject::Value::asText() : value is not text");
	return instance;
}

const CrudObject::TextValue* const CrudObject::Value::asText() const
{
	const TextValue* const instance = dynamic_cast<const TextValue* const>(this);
	if (!instance)
		throw fusepp::Error("fusepp::CrudObject::Value::asText() : value is not text");
	return instance;
}

CrudObject::DoubleValue* CrudObject::Value::asDouble()
{
	DoubleValue* instance = dynamic_cast<DoubleValue*>(this);
	if (!instance)
		throw fusepp::Error("fusepp::CrudObject::Value::asDouble() : value is not a double");
	return instance;
}

const CrudObject::DoubleValue* const CrudObject::Value::asDouble() const
{
	const DoubleValue* const instance = dynamic_cast<const DoubleValue* const>(this);
	if (!instance)
		throw fusepp::Error("fusepp::CrudObject::Value::asDouble() : value is not a double");
	return instance;
}

CrudObject::DateValue* CrudObject::Value::asDate()
{
	DateValue* instance = dynamic_cast<DateValue*>(this);
	if (!instance)
		throw fusepp::Error("fusepp::CrudObject::Value::asDate() : value is not a date");
	return instance;
}

const CrudObject::DateValue* const CrudObject::Value::asDate() const
{
	const DateValue* const instance = dynamic_cast<const DateValue* const>(this);
	if (!instance)
		throw fusepp::Error("fusepp::CrudObject::Value::asDate() : value is not a date");
	return instance;
}
//
//CrudObject::ByteaValue* CrudObject::Value::asBytea()
//{
//	ByteaValue* instance = dynamic_cast<ByteaValue*>(this);
//	if (!instance)
//		throw fusepp::Error("fusepp::CrudObject::Value::asBytea() : value is not a byte array");
//	return instance;
//}

//const CrudObject::ByteaValue* const CrudObject::Value::asBytea() const
//{
//	const ByteaValue* const instance = dynamic_cast<const ByteaValue* const>(this);
//	if (!instance)
//		throw fusepp::Error("fusepp::CrudObject::Value::asBytea() : value is not byte array");
//	return instance;
//}

CrudObject::BoolValue* CrudObject::Value::asBool()
{
	BoolValue* instance = dynamic_cast<BoolValue*>(this);
	if (!instance)
		throw fusepp::Error("fusepp::CrudObject::Value::asBool() : value is not a boolean");
	return instance;
}

const CrudObject::BoolValue* const CrudObject::Value::asBool() const
{
	const BoolValue* const instance = dynamic_cast<const BoolValue* const>(this);
	if (!instance)
		throw fusepp::Error("fusepp::CrudObject::Value::asBool() : value is not a boolean");
	return instance;
}

