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
#include <fusepp/pg/Query.h>
#include <fusepp/pg/Database.h>
#include <fusepp/Error.h>
#include <stdarg.h>
#include <iomanip>
#ifndef _WIN32
#include <string.h>
#include <stdlib.h>
#endif //_WIN32
#include <assert.h>
using namespace fusepp;


// ===========================================================================
// fusepp::Query implementation
// ===========================================================================

Query::Query(DatabasePtr database, unsigned int constraints)
{
	assert(database);
	_db = database;
	init(constraints);
}

void Query::init(unsigned int constraints)
{
	_result = NULL;
	_wasSuccessful = false;
	_wasCommand = false;
	_constraints = constraints;
	_constraintsViolated = 0;
	_rowsCount = 0;
	_columnsCount = 0;
	_wantBinaryResults = false;
	_extparam = 1;
	_stdIn = NULL;
}

Query::~Query()
{
	if (_result != NULL)
		PQclear(_result);
	if (_stdIn != NULL)
		delete _stdIn;
}

bool Query::execute(bool throwOnFail)
{
	//osg::notify(osg::DEBUG_INFO) << "PGSQL query: " << str() << std::endl;
	
	if (_byteParameters.size() == 0 && !_wantBinaryResults)
	{
		_result = PQexec(_db->_pgconn, str().c_str());
	}
	else
	{
		std::vector<Oid> types;
		std::vector<const char*> values;
		std::vector<int> lengths;
		std::vector<int> formats;
		
		if (_byteParameters.size() != 0)
		{
			types.resize(_byteParameters.size());
			values.resize(_byteParameters.size());
			lengths.resize(_byteParameters.size());
			formats.resize(_byteParameters.size());
			size_t count = 0;
			for (std::list<std::pair<const char* const,size_t> >::const_iterator it=_byteParameters.begin();it!=_byteParameters.end();it++)
			{
				types[count] = _db->getTypeOid("bytea");
				values[count] = it->first;
				lengths[count] = (int)it->second;
				formats[count] = 1; // 1 for binary, 0 for text
				count++;
			}
		}

		int numParams = (int)_byteParameters.size();
		
		_result = PQexecParams(_db->_pgconn, 
								str().c_str(), 
								numParams, 
								(numParams ? &types[0] : NULL), 
								(numParams ? &values[0] : NULL),
								(numParams ? &lengths[0] : NULL),
								(numParams ? &formats[0] : NULL),
								(_wantBinaryResults ? 1 : 0));
	}
	
	int res = PQresultStatus(_result);
	
	if (res == PGRES_COPY_IN)
	{
		if (_stdIn)
		{
			std::string buf = _stdIn->str();
			size_t bufsize = buf.size();
			if (bufsize)
			{
				int copy_res = PQputCopyData(_db->_pgconn, buf.c_str(), bufsize);
				if (copy_res == 1)
				{
					copy_res = PQputCopyEnd(_db->_pgconn, NULL);
					if (copy_res == 1)
					{
						assert(_result);
						PQclear(_result);
						_result = PQgetResult(_db->_pgconn);
						res = PQresultStatus(_result);
					}
				}
			}
		}
	}

	_wasSuccessful = (res == PGRES_TUPLES_OK) || (res == PGRES_COMMAND_OK);

	if (res == PGRES_COMMAND_OK)
		_wasCommand = true;
	
	
	
	if (_wasSuccessful)
	{
		_rowsCount = PQntuples(_result);
		_columnsCount = PQnfields(_result);
		checkConstraints();
	}
	
	bool querySucceeded = _wasSuccessful && _constraintsViolated == 0;
	
	if (!querySucceeded && throwOnFail)
		throw Error(*this, "Query %s failed", str().c_str());
	
	//osg::notify(osg::DEBUG_INFO) << "  " << getRowsCount() << " rows returned" << std::endl;
	return querySucceeded; 
}

unsigned int Query::getRowsCount() const
{
	if (!_wasSuccessful || _wasCommand)
		return 0;
	return _rowsCount;
}

unsigned int Query::getColumnsCount() const
{
	if (!_wasSuccessful || _wasCommand)
		return 0;
	return _columnsCount;
}

void Query::checkConstraints()
{
	_constraintsViolated = 0;
	unsigned int bit = 0x1;
	while (bit < LAST_CONSTRAINT)
	{
		if ((_constraints & bit) == bit)
		{
			switch (bit)
			{
			case AT_LEAST_ONE_ROW:
				if (_rowsCount < 1)
					_constraintsViolated += bit;
				break;
			case AT_MOST_ONE_ROW:
				if (_rowsCount > 1)
					_constraintsViolated += bit;
				break;
			default:
				assert(false);
				break;
			}
		}
		bit <<= 1;
	}
}

std::ostream& fusepp::operator<<(std::ostream& stream, const fusepp::Query& query)
{
	if (PQresultStatus(query._result) == PGRES_COMMAND_OK)
		stream << "QUERY OK: command succeeded.";
	else if (PQresultStatus(query._result) == PGRES_TUPLES_OK)
		stream << "QUERY OK: " << PQntuples(query._result) << " rows returned.";
	else{
		stream << "QUERY ERROR: " << PQresultErrorMessage(query._result) << "(" << PQresStatus(PQresultStatus(query._result)) << ").";
		return stream;
	}
	if (query._constraintsViolated != 0)
	{
		unsigned int bit = 0x1;
		while (bit < Query::LAST_CONSTRAINT)
		{
			if ((query._constraintsViolated & bit) == bit)
			{
				switch (bit)
				{
				case Query::AT_LEAST_ONE_ROW:
					stream << " CONSTRAINT ERROR: The query should have returned at least one row";
					break;
				case Query::AT_MOST_ONE_ROW:
					stream << " CONSTRAINT ERROR: The query should have returned at most one row";
					break;
				default:
					assert(false);
					break;
				}
			}
			bit <<= 1;
		}	
	}
	return stream;
}

int Query::getColumnIndex(const std::string& columnName)
{
	int column = PQfnumber(_result, columnName.c_str());
	if (column == -1)
		throw fusepp::Error("the column '%s' was not found in the result set", columnName.c_str());
	assert((unsigned int)column < _columnsCount);
	return column;
}

std::string Query::at(unsigned int row, unsigned int column) const
{
	assert(row < _rowsCount);
	assert(column < _columnsCount);
	return PQgetvalue(_result, row, column);
}

std::string Query::at(unsigned int row, const std::string& columnLabel) const
{
	assert(row < _rowsCount);
	int column = PQfnumber(_result, columnLabel.c_str());
	if (column == -1)
		throw fusepp::Error("the column '%s' was not found in the result set", columnLabel.c_str());
	assert((unsigned int)column < _columnsCount);
	return PQgetvalue(_result, row, column);
}

double Query::atf(unsigned int row, unsigned int column) const
{
	assert(row < _rowsCount);
	assert(column < _columnsCount);
	std::stringstream ss;
	ss.imbue(std::locale("C"));
	ss << PQgetvalue(_result, row, column);
	double result;
	ss >> result;
	assert(!ss.fail());
	return result;
}

double Query::atf(unsigned int row, const std::string& columnLabel) const
{
	assert(row < _rowsCount);
	int column = PQfnumber(_result, columnLabel.c_str());
	if (column == -1)
		throw fusepp::Error("the column '%s' was not found in the result set", columnLabel.c_str());
	assert((unsigned int)column < _columnsCount);
	std::stringstream ss;
	ss.imbue(std::locale("C"));
	ss << PQgetvalue(_result, row, column);
	double result;
	ss >> result;
	assert(!ss.fail());
	return result;
}

int Query::ati(unsigned int row, unsigned int column) const
{
	assert(row < _rowsCount);
	assert(column < _columnsCount);
	return atoi(PQgetvalue(_result, row, column));
}

int Query::ati(unsigned int row, const std::string& columnLabel) const
{
	assert(row < _rowsCount);
	int column = PQfnumber(_result, columnLabel.c_str());
	if (column == -1)
		throw fusepp::Error("the column '%s' was not found in the result set", columnLabel.c_str());
	assert((unsigned int)column < _columnsCount);
	return atoi(PQgetvalue(_result, row, column));
}

bool Query::isnull(unsigned int row, unsigned int column) const
{
	assert(row < _rowsCount);
	assert(column < _columnsCount);
	return PQgetisnull(_result, row, column) == 1;
}

bool Query::isnull(unsigned int row, const std::string& columnLabel) const
{
	assert(row < _rowsCount);
	int column = PQfnumber(_result, columnLabel.c_str());
	if (column == -1)
		throw fusepp::Error("the column '%s' was not found in the result set", columnLabel.c_str());
	assert((unsigned int)column < _columnsCount);
	return PQgetisnull(_result, row, column) == 1;
}

bool Query::atb(unsigned int row, unsigned int column) const
{
	assert(row < _rowsCount);
	assert(column < _columnsCount);
	return strcmp(PQgetvalue(_result, row, column),"t") == 0;
}

bool Query::atb(unsigned int row, const std::string& columnLabel) const
{
	int column = PQfnumber(_result, columnLabel.c_str());
	if (column == -1)
		throw fusepp::Error("the column '%s' was not found in the result set", columnLabel.c_str());
	assert((unsigned int)column < _columnsCount);
	return strcmp(PQgetvalue(_result, row, column),"t") == 0;
}

void Query::atf_array(unsigned int row, unsigned int column, std::vector<double>& result) const
{
	const char* res = PQgetvalue(_result, row, column);

	std::vector<char> buf(strlen(res)+1);
#ifdef _MSC_VER
	strcpy_s(&buf[0], buf.size(), res);
#else
	strcpy(&buf[0], res);
#endif
	buf[buf.size()-1] = 0;

	char* p = &buf[0];

	int l = strlen(p);
	if (l < 2)
		throw fusepp::Error("invalid array value");
	const char* end = p + l;
	if (p[0] != '{' || p[l-1] != '}')
		throw fusepp::Error("invalid array value");

	p[l-1] = ',';

	std::stringstream atofs;
	atofs.imbue(std::locale("C"));

	++p;
	while (p < end)
	{
		char* q = strchr(p, ',');
		if (q == NULL)
			break;

		*q = 0;
		
		atofs.str(p);
		atofs.clear();
		atofs << p;
		double x;
		atofs >> x;
		if (!atofs)
			throw fusepp::Error("invalid array value");

		result.push_back(x);

		++q;
		p = q;
	}
}

void Query::atf_array(unsigned int row, const std::string& columnLabel, std::vector<double>& result) const
{
	int column = PQfnumber(_result, columnLabel.c_str());
	if (column == -1)
		throw fusepp::Error("the column '%s' was not found in the result set", columnLabel.c_str());
	assert((unsigned int)column < _columnsCount);
	atf_array(row, column, result);
}

void Query::at_array(unsigned int row, unsigned int column, std::vector<std::string>& result) const
{
	const char* res = PQgetvalue(_result, row, column);

	std::vector<char> buf(strlen(res)+1);
#ifdef _MSC_VER
	strcpy_s(&buf[0], buf.size(), res);
#else
	strcpy(&buf[0], res);
#endif
	buf[buf.size()-1] = '\0';

	int state = 0;
	size_t l = buf.size()-1;
	std::string token;
	for (size_t i=0;i<l;++i)
	{
		char c = buf[i];
		switch (state)
		{
		case 0:
			if (c == '{')
				state = 1;
			else
				state = -1;
			break;
		case 1:
			if (c == '"')
			{
				token = "";
				state = 2;
			}
			else if (c == '}')
				state = 4;
			else 
				state = -1;
			break;
		case 2:
			if (c == '\\')
			{
				if (i >= l)
					state = -1;
				else
					token += buf[++i];
			}
			else if (c == '"')
			{
				result.push_back(token);
				state = 3;
			}
			else
				token += c;
			break;
		case 3:
			if (c == ',')
				state = 1;
			else if (c == '}')
				state = 4;
			else
				state = -1;
			break;
		case 4:
			state = -1;
			break;
		}

		if (state == -1)
			throw fusepp::Error("invalid array value");
	}
}

void Query::at_array(unsigned int row, const std::string& columnLabel, std::vector<std::string>& result) const
{
	int column = PQfnumber(_result, columnLabel.c_str());
	if (column == -1)
		throw fusepp::Error("the column '%s' was not found in the result set", columnLabel.c_str());
	assert((unsigned int)column < _columnsCount);
	at_array(row, column, result);
}

void Query::ati_array(unsigned int row, unsigned int column, std::vector<int>& result) const
{
	const char* res = PQgetvalue(_result, row, column);

	std::vector<char> buf(strlen(res)+1);
#ifdef _MSC_VER
	strcpy_s(&buf[0], buf.size(), res);
#else
	strcpy(&buf[0], res);
#endif
	buf[buf.size()-1] = 0;

	char* p = &buf[0];

	int l = strlen(p);
	if (l < 2)
		throw fusepp::Error("invalid array value");
	else if (l == 2)
	{
		result.clear();
		return;
	}
	const char* end = p + l;
	if (p[0] != '{' || p[l-1] != '}')
		throw fusepp::Error("invalid array value");

	p[l-1] = ',';

	std::stringstream atois;

	result.clear();
	++p;
	while (p < end)
	{
		char* q = strchr(p, ',');
		if (q == NULL)
			break;

		*q = 0;
		
		atois.str(p);
		atois.clear();
		atois << p;
		int x;
		atois >> x;
		if (!atois)
			throw fusepp::Error("invalid array value");

		result.push_back(x);

		++q;
		p = q;
	}
}

void Query::ati_array(unsigned int row, const std::string& columnLabel, std::vector<int>& result) const
{
	int column = PQfnumber(_result, columnLabel.c_str());
	if (column == -1)
		throw fusepp::Error("the column '%s' was not found in the result set", columnLabel.c_str());
	assert((unsigned int)column < _columnsCount);
	ati_array(row, column, result);
}

void Query::reset(unsigned int constraints)
{
	if (_result != NULL)
		PQclear(_result);
	str("");
	_byteParameters.clear();
	//_buffers.clear();
	if (_stdIn != NULL)
	{
		delete _stdIn;
		_stdIn = NULL;
	}
	init(constraints);
}

void Query::addParameterBytea(const char* const data, size_t size)
{
	_byteParameters.push_back(std::pair<const char* const,size_t>(data, size));
}

//void Query::addParameterBytea(fusepp::RefBuffer<char>* buffer)
//{
//	assert(buffer);
//	_buffers.push_back(buffer);
//	_byteParameters.push_back(std::pair<char*,size_t>(buffer->ptr(), buffer->size()));
//}

void Query::setResultTypeBinary()
{
	_wantBinaryResults = true;
}

void Query::setResultTypeText()
{
	_wantBinaryResults = false;
}

void Query::getBinaryResult(std::vector<char>& output, unsigned int row, unsigned int column)
{
	assert(_wantBinaryResults);
	size_t len = PQgetlength(_result, row, column);
	output.resize(len);
	memcpy(&output, PQgetvalue(_result, row, column), len);
}

std::string Query::extparam()
{
	std::stringstream ss;
	ss << "$" << _extparam << "::bytea";
	_extparam++;
	return ss.str();
}

std::ostream& Query::getStdIn()
{
	if (!_stdIn)
		_stdIn = new std::stringstream;
	return *_stdIn;
}



// ===========================================================================
// fusepp::Query::Error implementation
// ===========================================================================

Query::Error::Error(const Query& query) : fusepp::Error()
{
	std::stringstream ss;
	ss << query;

	set("%s", ss.str().c_str());
}

Query::Error::Error(const Query& query, const char* reason, ...)
{
	va_list arglist;
	va_start(arglist, reason);
	format(reason, arglist);
	std::stringstream ss;
	ss << getReason() << std::endl << query;
	set("%s", ss.str().c_str());
}


// ===========================================================================
// fusepp::TransactionBlock implementation
// ===========================================================================

TransactionBlock::TransactionBlock(DatabasePtr db) 
	: _db(db), 
	_hasComitted(false)
{
	assert(_db.get());
	Query query(_db);
	query << "BEGIN;";
	query.execute(true);
}

TransactionBlock::~TransactionBlock()
{
	if (!_hasComitted)
	{
		Query query(_db);
		query << "ROLLBACK;";
		query.execute(true);
	}
}

void TransactionBlock::commit(bool startAnother)
{
	Query query(_db);
	query << "COMMIT;";
	query.execute(true);
	_hasComitted = true;

	if (startAnother)
	{
		query.reset();
		query << "BEGIN;";
		query.execute(true);
		_hasComitted = false;
	}
}



