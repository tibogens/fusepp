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

#ifndef _FUSEPP_PG_QUERY_H
#define _FUSEPP_PG_QUERY_H

#include <fusepp/pg/Export.h>
#include <fusepp/pg/DBMSBackend.h>
#include <vector>
#include <list>
#include <sstream>
#include <fusepp/Error.h>
//#include <uflr/Utils.h>
#include <fusepp/pg/Database.h>
#include <memory>

namespace fusepp
{

class FUSEPP_PG_API Query : public std::stringstream
{
public:
	class FUSEPP_PG_API Error : public fusepp::Error
	{
	public:
		Error(const Query& query);
		Error(const Query& query, const char* reason, ...);
	};
	enum Constraints
	{
		NONE				= 0,
		AT_LEAST_ONE_ROW	= 1,
		AT_MOST_ONE_ROW		= 2,
		ONLY_ONE_ROW		= 3,
		
		LAST_CONSTRAINT,
	};

	Query(DatabasePtr db, unsigned int constraints = 0);
	virtual ~Query();
	
	virtual bool execute(bool throwOnFail = false);

	unsigned int getRowsCount() const;
	unsigned int getColumnsCount() const;

	inline PGresult* getResult() { return _result; }

	int getColumnIndex(const std::string& columnName);
	std::string at(unsigned int row, unsigned int column) const;
	std::string at(unsigned int row, const std::string& columnLabel) const;
	double atf(unsigned int row, unsigned int column) const;
	double atf(unsigned int row, const std::string& columnLabel) const;
	int ati(unsigned int row, unsigned int column) const;
	int ati(unsigned int row, const std::string& columnLabel) const;
	bool isnull(unsigned int row, unsigned int column) const;
	bool isnull(unsigned int row, const std::string& columnLabel) const;
	bool atb(unsigned int row, unsigned int column) const;
	bool atb(unsigned int row, const std::string& columnLabel) const;

	void at_array(unsigned int row, unsigned int column, std::vector<std::string>& result) const;
	void at_array(unsigned int row, const std::string& columnLabel, std::vector<std::string>& result) const;
	void ati_array(unsigned int row, unsigned int column, std::vector<int>& result) const;
	void ati_array(unsigned int row, const std::string& columnLabel, std::vector<int>& result) const;
	void atf_array(unsigned int row, unsigned int column, std::vector<double>& result) const;
	void atf_array(unsigned int row, const std::string& columnLabel, std::vector<double>& result) const;

	friend FUSEPP_PG_API std::ostream& operator<<(std::ostream& stream, const Query& result);

	//void addParameterBytea(const std::vector<char>& buffer);
	void addParameterBytea(const char* const data, size_t size);
	
	void setResultTypeBinary();
	void setResultTypeText();

	// Well, I could return std::vector<char>, but that implies I have to check that NRVO is used.
	// (If it's not, then the current API is better, as fills 'output' directly from the DB driver)
	// TODO: start here: http://stackoverflow.com/questions/3721217/returning-a-c-stdvector-without-a-copy
	void getBinaryResult(std::vector<char>& output, unsigned int row = 0, unsigned int column = 0);

	std::string extparam();

	virtual void reset(unsigned int constraints = 0);

	inline unsigned int getConstraints() const { return _constraints; }
	inline void setConstraints(unsigned int constraints) { _constraints = constraints; }

	std::ostream& getStdIn();

	inline Database* getDatabase() { return _db.get(); }

private:
	void init(unsigned int constraints);
	void checkConstraints();
	PGresult* _result;
	std::shared_ptr<Database> _db;
	bool _wasSuccessful, _wasCommand;
	unsigned int _constraints, _constraintsViolated;
	unsigned int _rowsCount, _columnsCount;
	std::stringstream* _stdIn;;


	std::list<std::pair<const char* const,size_t> > _byteParameters;

	bool _wantBinaryResults;
	int _extparam;
};

FUSEPP_PG_API std::ostream& operator<<(std::ostream& stream, const Query& result);

// Helper class that issues a BEGIN call when created and ROLLBACK when destroyed,
// unless commit() is called, in which case it issues a COMMIT and does not roll
// back in its destructor.
// Create on the stack to make sure all transactions in the current scope are either
// all executed or not at all.
// WARNING: do *not* nest TransactionBlocks ! PostgreSQL will refuse another BEGIN
// to be emitted before a COMMIT or a ROLLBACK has been issued.

class FUSEPP_PG_API TransactionBlock
{
public:
	TransactionBlock(DatabasePtr db);
	virtual ~TransactionBlock();
	void commit(bool startAnother = false);
private:
	DatabasePtr _db;
	bool _hasComitted;
};

};

#endif //_FUSEPP_PG_QUERY_H
