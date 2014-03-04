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

#ifndef _FUSEPP_PG_COMPOSITEQUERY_H
#define _FUSEPP_PG_COMPOSITEQUERY_H

#include <fusepp/pg/Export.h>
#include <fusepp/pg/DBMSBackend.h>
#include <fusepp/pg/Query.h>
#include <set>

namespace fusepp
{

class FUSEPP_PG_API CompositeQuery : public Query
{
public:
	CompositeQuery(DatabasePtr db, unsigned int constraints = 0);
	virtual ~CompositeQuery();

	// Override of the fusepp::Query interface
	bool execute(bool throwOnFail = false);
	void reset(unsigned int constraints = 0);

	// Public interface
	void addSelectColumn(const std::string& column);
	void addFromTable(const std::string& table);
	void addOrderClause(const std::string& orderClause);
	void setOrderAscending(bool ascending);

	void flattenToStream(std::stringstream& stream, bool appendSemicolon) const;

private:
	bool _flattened;
	std::list<std::string> _columns, _tables, _order;
	std::set<std::string> _columnsSet, _tablesSet, _orderSet;
	bool _orderAsc;
};

};

#endif // _FUSEPP_PG_COMPOSITEQUERY_H
