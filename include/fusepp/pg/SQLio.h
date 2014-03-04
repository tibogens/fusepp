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

#ifndef _FUSEPP_PG_SQLIO_H
#define _FUSEPP_PG_SQLIO_H

#include <fusepp/pg/Export.h>
#include <fusepp/pg/DBMSBackend.h>
#include <fusepp/pg/Query.h>

namespace fusepp
{

class Database;

enum ArrayGetMode
{
	ZERO_SIZED_ARRAY_ON_EMPTY,
	NULL_ON_EMPTY,
};

// Helper class to output an STL container (vector or list) in the SQL array syntax. 
// The result will be ARRAY[value_1,value_2,...,value_n].
// The optional parameter specifies how empty arrays should be handled: with the '{}' 
// syntax (i.e. empty SQL array) or a NULL value.
template <class _Container_>
class toArray
{
public:
	toArray(const _Container_& anArray, ArrayGetMode getMode = ZERO_SIZED_ARRAY_ON_EMPTY)
		: _anArray(anArray), _getMode(getMode) {}

public:
	const _Container_& _anArray;
	ArrayGetMode _getMode;
};


// Helper class to output an STL container (vector or list) in the SQL tuple syntax.
// The result will be (value_1,value_2,...,value_n)
template <class _Container_>
class toTuple
{
public:
	toTuple(const _Container_& anArray)
		: _anArray(anArray) {}

public:
	const _Container_& _anArray;
};

// Helper class to have more control over SQL arrays construction. It is used
// internally by the toArray() IO manipulator, so if the array is simple (i.e.
// does not contain binary data or data that is generated on the fly), prefer
// toArray().
// Usage:
// - create an ArraysHelper instance, passing a connected Database and the number
//   of arrays you wish to create (typically only one, but many arrays can be
//   handled by a single instance of ArraysHelper where you need more than one)
// - call update(n, value) where n is the array number, starting from 0 (e.g. 0
//   if you have only one array) and value is a string, an [unsigned] int or a double
// - call end()
// - call get(n, mode) where mode is can take one of the ArrayGetMode enumeration
//   values. This mode is only used when the array is empty. With the mode
//   ZERO_SIZED_ARRAY_ON_EMPTY, get() returns "'{}'". When the mode is NULL_ON_EMPTY,
//   get() returns "NULL" - that is, the string "NULL" that represents NULL values
//   in SQL.
class FUSEPP_PG_API ArraysHelper
{
public:
	ArraysHelper(DatabasePtr database, size_t count);
	void update(size_t index, const std::string& value);
	void updateWithSQL(size_t index, const std::string& sqlcode);
	void update(size_t index, unsigned int value);
	void update(size_t index, int value);
	void update(size_t index, double value);
	void end();
	std::string get(size_t index, ArrayGetMode mode = ZERO_SIZED_ARRAY_ON_EMPTY) const;
private:
	struct Item
	{
		std::string value;
		bool isEmpty;
	};
	std::vector<Item> _arrays;
	DatabasePtr _database;

	void doUpdate(size_t index, const std::string& value, bool isString);
};

template <class _Container_>
std::ostream& operator<<(std::ostream& stream, const toArray<_Container_>& anArray)
{
	Query* query = dynamic_cast<Query*>(&stream);
	if (query)
	{
		ArraysHelper helper(query->getDatabase(), 1);
		for (typename _Container_::const_iterator it=anArray._anArray.begin();it!=anArray._anArray.end();++it)
			helper.update(0, *it);
		helper.end();
		*query << helper.get(anArray._getMode);
	}
	return stream;
}

template <class _Container_>
std::ostream& operator<<(std::ostream& stream, const toTuple<_Container_>& anArray)
{
	stream << "(";
	size_t i=0;
	for (typename _Container_::const_iterator it=anArray._anArray.begin();it!=anArray._anArray.end();++it)
	{
		if (i++ > 0)
			stream << ",";
		stream << *it;
	}
	stream << ")";

	return stream;
}


};

#endif // _FUSEPP_PG_SQLIO_H
