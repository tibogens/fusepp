/*
Copyright (c) 2014, University of Lausanne, Switzerland 
All rights reserved. 

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are 
met: 

1. Redistributions of source code must retain the above copyright 
notice, this list of conditions and the following disclaimer. 

2. Redistributions in binary form must reproduce the above copyright 
notice, this list of conditions and the following disclaimer in the 
documentation and/or other materials provided with the distribution. 

3. Neither the name of the copyright holder nor the names of its 
contributors may be used to endorse or promote products derived from 
this software without specific prior written permission. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS 
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED 
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A 
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED 
TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 

*/

#include <fusepp/Application.h>
#include <unistd.h>
#include <sys/types.h>
#include <iostream>
#include <pthread.h>
#include <fusepp/Error.h>
#include <assert.h>
#include <fusepp/pg/Database.h>
#include <fusepp/pg/Query.h>

struct TLSData
{
	fusepp::DatabasePtr db;
};

std::ostream& operator<<(std::ostream& stream, const struct stat& buf)
{
	stream << "<struct stat @" << &buf << ":" << std::endl
		<< "\t" << "st_dev = " << buf.st_dev << std::endl
		<< "\t" << "st_ino = " << buf.st_ino << std::endl
		<< "\t" << "st_mode = " << buf.st_mode << std::endl
		<< "\t" << "st_nlink = " << buf.st_nlink << std::endl
		<< "\t" << "st_uid = " << buf.st_uid << std::endl
		<< "\t" << "st_gid = " << buf.st_gid << std::endl
		<< "\t" << "st_blksize = " << buf.st_blksize << std::endl
		<< "\t" << "st_blocks = " << buf.st_blocks << std::endl
		<< "\t" << "st_atime = " << buf.st_atime << std::endl
		<< "\t" << "st_mtime = " << buf.st_mtime << std::endl
		<< "\t" << "st_ctime = " << buf.st_ctime << " >" << std::endl;
		return stream;
}

class PostgresFileSystem : public fusepp::FS_getattr, public fusepp::FS_readdir
{
public:

	PostgresFileSystem(const std::string& dbname, const std::string& host, const std::string& port, const std::string& username, const std::string& password)
		: _dbname(dbname), _host(host), _port(port), _username(username), _password(password)
	{

		if (pthread_key_create(&_tls, tls_destroy) != 0)
			throw fusepp::Error("pthread_key_create() failed with error %d", errno);
	}

	~PostgresFileSystem()
	{
		if (pthread_key_delete(_tls) != 0)
			throw fusepp::Error("pthread_key_delete() failed with error %d", errno);
	}

	static void tls_destroy(void* value)
	{
		do_tls_destroy(value, false);
	}

	static void do_tls_destroy(void* value, bool explicit_call)
	{
		assert(value);
		TLSData* data = static_cast<TLSData*>(value);
		delete data;
	}

private:
	pthread_key_t _tls;
	std::string _dbname, _host, _port, _username, _password;

	TLSData* get_tls_data()
	{
		TLSData* data = NULL;
		void* ptr = pthread_getspecific(_tls);
		if (!ptr)
		{
			data = new TLSData;
			pthread_setspecific(_tls, data);
		}
		else
		{
			data = static_cast<TLSData*>(ptr);
		}
		if (!data->db)
		{
			data->db.reset(new fusepp::Database());
			if (!data->db->connect(_host, _port, _dbname, _username, _password))
			{
				data->db.reset();
				throw fusepp::Error("Database connection failed");
			}
		}
		return data;
	}

public:
	int getattr(const std::string& path, struct stat* buf)
	{
		TLSData* data = get_tls_data();
		if (path == "/print.jpg")
			std::cout << "stat of '" << path << "' before: " << *buf << std::endl;
		if (path == "/")
		{
			buf->st_mode = S_IFDIR | 0755;
			buf->st_uid = getuid();
			buf->st_gid = getgid();
			return 0;
		}
		//else
		//{
		//	fusepp::Query q(data->db, fusepp::Query::ONLY_ONE_ROW);
		//	q << "select * from imagev where dfsa;";
		//	q.execute(true);

		//}
		return -ENOENT;
	}

	int readdir(const std::string& path, DirectoryFiller& filler)
	{
		TLSData* data = get_tls_data();
		fusepp::Query q(data->db);
		q << "select * from imagev where typeid <> (select id from imagetype where short = 'THUMBNAIL') order by name asc;";
		q.execute(true);
		for (unsigned int i = 0; i < q.getRowsCount(); ++i)
		{
			filler.add(q.at(i, "name"), q.ati(i, "id"));
		}
		return 0;
	}
	
};


int main(int argc, char* argv[])
{
	char buffer[256];
	std::cout << "current directory=" << getcwd(buffer, 256) << std::endl;
	fusepp::FileSystemPtr fs(new PostgresFileSystem("pianos", "127.0.0.1", "5432", "tibo", ""));
	fusepp::ApplicationPtr app(new fusepp::Application(fs));
	
	return app->run(argc, argv);
}
