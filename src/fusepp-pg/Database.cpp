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

#include <libpq-fe.h>
#include <stdarg.h>
#include <algorithm>
#include <fusepp/pg/Database.h>
#include <fusepp/pg/Query.h>
#include <iostream>
#include <locale>
#ifndef _WIN32
#include <string.h>
#endif //_WIN32
#include <assert.h>
using namespace fusepp;

Database::Database() : _pgconn(NULL)
{
#ifdef _DEBUG
	_ownerThread = fusepp::getCurrentThreadId();
#endif
	std::cout << "Created fusepp::Database instance" << std::endl;
}

Database::~Database()
{
	disconnect();
	std::cout << "Destroyed fusepp::Database instance" << std::endl;
}

bool Database::connect(const std::string& host,
				 const std::string& port,
				 const std::string& dbname,
				 const std::string& user,
				 const std::string& password)
{
	//osg::notify(osg::INFO) << "Connecting to PostgreSQL database " << host << ":" << port << " ... " << std::endl;
	
	std::stringstream ss;
	ss << "host=" << host << " port=" << port << " dbname=" << dbname << " user=" << user;
	
	if (!password.empty())
		ss << " password=" << password;

	bool res = connectFromPgString(ss.str());
	//if (res)
	//	osg::notify(osg::INFO) << "OK !" << std::endl;
	//else
	//	osg::notify(osg::INFO) << "Failed !" << std::endl;
	return res;
}

bool Database::connectFromPgString(const std::string& connection)
{
#ifdef _DEBUG
	assert(fusepp::getCurrentThreadId() == _ownerThread);
#endif

	_connectionString = connection;

	_pgconn = PQconnectdb(_connectionString.c_str());
	if (PQstatus(_pgconn) == CONNECTION_OK)
	{
		//osg::notify(osg::INFO) << "PostgreSQL module connected" << std::endl;
		if (PQsetClientEncoding(_pgconn, "UTF8") == 0)
			return true;
		//else
		//	osg::notify(osg::WARN) << "Could not set database client encoding to UTF-8" << std::endl;

	}
	_pgconn = NULL;
	_connectionString = "";
	//osg::notify(osg::WARN) << "PostgreSQL connection failed" << std::endl;
	disconnect();
	return false;
}

bool Database::disconnect()
{
	if (_pgconn)
	{
		PQfinish(_pgconn);
		_pgconn = NULL;
		//osg::notify(osg::INFO) << "Disconnected from the PostgreSQL database" << std::endl;
		return true;
	}else{
		return false;
	}
}

std::string Database::escape(const char* in) const
{
#ifdef _DEBUG
	assert(fusepp::getCurrentThreadId() == _ownerThread);
#endif

	if (in == NULL)
		return "";
	size_t length = in?strlen(in):0;
	char* buffer = new char[2*length+1];
	PQescapeStringConn(_pgconn, buffer, in, length, NULL);
	std::string ret(buffer);
	delete[] buffer;
	return ret;
}

std::string Database::escape(const std::string& in) const
{
#ifdef _DEBUG
	assert(fusepp::getCurrentThreadId() == _ownerThread);
#endif

	size_t length = in.size();
	char* buffer = new char[2*length+1];
	PQescapeStringConn(_pgconn, buffer, in.c_str(), length, NULL);
	std::string ret(buffer);
	delete[] buffer;
	return ret;
}

Oid Database::getTypeOid(const std::string& type)
{
#ifdef _DEBUG
	assert(fusepp::getCurrentThreadId() == _ownerThread);
#endif

	assert(_pgconn);
	std::map<std::string,Oid>::const_iterator typeit = _typesOids.find(type);
	if (typeit == _typesOids.end())
	{
		std::shared_ptr<Database> self(this);
		Query query(self, Query::ONLY_ONE_ROW);
		query << "SELECT oid FROM pg_type WHERE typname = '" << escape(type) << "';";
		query.execute(true);
		Oid value = query.ati(0, 0);
		assert(value != 0);
		_typesOids[type] = value;
		return value;
	}
	else
	{
		return typeit->second;
	}

}

