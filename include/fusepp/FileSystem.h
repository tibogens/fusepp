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

#ifndef _FUSEPP_FILESYSTEM_H
#define _FUSEPP_FILESYSTEM_H

#include <fusepp/Export.h>
#include <string>
#include <sys/stat.h>
#include <memory>

namespace fusepp
{

class FUSEPP_API FileSystem
{
public:
	FileSystem();
	virtual ~FileSystem();

private:
};

typedef std::shared_ptr<FileSystem> FileSystemPtr;



class FUSEPP_API FS_getattr : public virtual FileSystem
{
public:
	virtual int getattr(const std::string& path, struct stat* buf) = 0;
};



class FUSEPP_API FS_readdir : public virtual FileSystem
{
public:
	class FUSEPP_API DirectoryFiller
	{
	public:
		static const ino_t INVALID_ID = (ino_t)~0;
		DirectoryFiller();
		virtual ~DirectoryFiller();
		virtual void add(const std::string& name, ino_t id = INVALID_ID) = 0;
	};
	virtual int readdir(const std::string& path, DirectoryFiller& filler) = 0;
};

};

#endif //_FUSEPP_FILESYSTEM_H
