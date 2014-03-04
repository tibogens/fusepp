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

#include <fusepp/pg/CompositeQuery.h>
#include <fusepp/pg/Database.h>
#include <assert.h>
using namespace fusepp;

CompositeQuery::CompositeQuery(DatabasePtr db, unsigned int constraints)
	: Query(db, constraints)
{
	_flattened = false;
	_orderAsc = true;
}

CompositeQuery::~CompositeQuery()
{
}

bool CompositeQuery::execute(bool throwOnFail)
{
	if (!_flattened)
	{
		flattenToStream(*this, true);
		_flattened = true;
	}

	return Query::execute(throwOnFail);
}

void CompositeQuery::reset(unsigned int constraints)
{
	Query::reset(constraints);

	_columns.clear();
	_columnsSet.clear();
	_tables.clear();
	_tablesSet.clear();
	_order.clear();
	_orderSet.clear();
	_orderAsc = true;
}

void CompositeQuery::addSelectColumn(const std::string& column)
{
	if (_columnsSet.find(column) == _columnsSet.end())
	{
		_columns.push_back(column);
		_columnsSet.insert(column);
	}
}

void CompositeQuery::addFromTable(const std::string& table)
{
	if (_tablesSet.find(table) == _tablesSet.end())
	{
		_tables.push_back(table);
		_tablesSet.insert(table);
	}
}

void CompositeQuery::addOrderClause(const std::string& orderClause)
{
	if (_orderSet.find(orderClause) == _orderSet.end())
	{
		_order.push_back(orderClause);
		_orderSet.insert(orderClause);
	}
}

void CompositeQuery::setOrderAscending(bool ascending)
{
	_orderAsc = ascending;
}

void CompositeQuery::flattenToStream(std::stringstream& stream, bool appendSemicolon) const
{
	if (_flattened)
	{
		if (&stream != this)
			stream << str();
	}
	else
	{
		assert(_columns.size() && _tables.size());

		std::string whereClause = str();
		stream.str("");

		stream << "SELECT ";
		int i=0;
		for (std::list<std::string>::const_iterator it = _columns.begin(); it != _columns.end(); ++it)
		{
			if (i++ > 0)
				stream << ",";
			stream << *it;
		}
		stream << " FROM ";
		i=0;
		for (std::list<std::string>::const_iterator it = _tables.begin(); it != _tables.end(); ++it)
		{
			if (i++ > 0)
				stream << ",";
			stream << *it;
		}

		if (whereClause.size())
			stream << " WHERE " << whereClause;

		if (_order.size())
		{
			stream << " ORDER BY ";
			i=0;
			for (std::list<std::string>::const_iterator it = _order.begin(); it != _order.end(); ++it)
			{
				if (i++ > 0)
					stream << ",";
				stream << *it;
			}
			if (_orderAsc)
				stream << " ASC";
			else
				stream << " DESC";
		}

		if (appendSemicolon)
			stream << ";";
	}
}
