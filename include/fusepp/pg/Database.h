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

#ifndef _FUSEPP_PG_DATABASE_H
#define _FUSEPP_PG_DATABASE_H

#include <fusepp/pg/Export.h>
#include <fusepp/pg/DBMSBackend.h>
#include <OpenThreads/Mutex>
#include <sstream>
#include <osg/Referenced>
#include <fusepp/Error.h>
#include <map>
#include <memory>

namespace fusepp
{

class FUSEPP_PG_API Database
{
public:
	Database();
	virtual ~Database();
	bool connect(const std::string& host,
				 const std::string& port,
				 const std::string& dbname,
				 const std::string& user,
				 const std::string& password = "");
	bool connectFromPgString(const std::string& connection);
	bool disconnect();
	std::string escape(const char* in) const;
	std::string escape(const std::string& in) const;
	
	friend class Query;
	
	inline bool isConnected() const { return _pgconn != NULL; }
	
	Oid getTypeOid(const std::string& type);

private:
	PGconn* _pgconn;
	std::string _connectionString;

#ifdef _DEBUG
	unsigned int _ownerThread;
#endif

	void init();

	std::map<std::string,Oid> _typesOids;
};

typedef std::shared_ptr<Database> DatabasePtr;

};

#endif //_FUSEPP_PG_DATABASE_H
