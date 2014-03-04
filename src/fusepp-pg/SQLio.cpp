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

#include <fusepp/pg/SQLio.h>
#include <fusepp/pg/Database.h>
#include <iomanip>
#include <assert.h>
using namespace fusepp;


ArraysHelper::ArraysHelper(DatabasePtr database, size_t count)
	: _database(database)
{
	assert(_database.get());
	_arrays.resize(count);
	for (size_t i=0;i<count;i++)
	{
		_arrays[i].value = "ARRAY[";
		_arrays[i].isEmpty = true;
	}
}

void ArraysHelper::update(size_t index, const std::string& value)
{
	doUpdate(index, value, true);
}

void ArraysHelper::updateWithSQL(size_t index, const std::string& sqlcode)
{
	doUpdate(index, sqlcode, false);
}

void ArraysHelper::update(size_t index, unsigned int value)
{
	std::stringstream ss;
	ss << value;
	doUpdate(index, ss.str(), false);
}

void ArraysHelper::update(size_t index, int value)
{
	std::stringstream ss;
	ss << value;
	doUpdate(index, ss.str(), false);
}

void ArraysHelper::update(size_t index, double value)
{
	std::stringstream ss;
	ss << std::setprecision(15) << value;
	doUpdate(index, ss.str(), false);
}

void ArraysHelper::doUpdate(size_t index, const std::string& value, bool isString)
{
	Item& item = _arrays[index];
	if (!item.isEmpty)
		item.value += ",";
	if (isString)
		item.value += "E'" + _database->escape(value) + "'";
	else
		item.value += value;
	item.isEmpty = false;
}

void ArraysHelper::end()
{
	for (size_t i=0;i<_arrays.size();i++)
	{
		_arrays[i].value += "]";
	}
}

std::string ArraysHelper::get(size_t index, ArrayGetMode mode) const
{
	const Item& item = _arrays[index];
	if (item.isEmpty)
		return (mode == ZERO_SIZED_ARRAY_ON_EMPTY ? "'{}'" : "NULL");
	else
		return item.value;
}


