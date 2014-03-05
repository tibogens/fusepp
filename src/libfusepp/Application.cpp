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


#include <fusepp/Application.h>
#define FUSE_USE_VERSION 26
#include <fuse.h>
#include <string.h>
#include <assert.h>
#include <iostream>
#include <fusepp/Error.h>
using namespace fusepp;

Application* Application::_s_instance(NULL);

Application::Application(FileSystemPtr fs)
	: _fs(fs)
{
	assert(!_s_instance);
	assert(_fs.get());
	_s_instance = this;
}

Application::~Application()
{
}

namespace fusepp_impl
{

#define GET_FS_INSTANCE(cls) \
	assert(Application::_s_instance); \
	cls* instance = dynamic_cast<cls*>(Application::_s_instance->_fs.get()); \
	assert(instance)

#define CALL_FS_IMPL_BEGIN() \
	try {

#define CALL_FS_IMPL_END(errval) \
	} catch (Error& err) { \
		std::cout << "[ERROR] Unhandled fusepp::Error: " << err << std::endl; \
	} catch (std::exception& err) { \
		std::cout << "[ERROR] Unhandled std::exception: " << err.what() << std::endl; \
	} catch (...) { \
		std::cout << "[ERROR] Unhandled (unknown) exception!" << std::endl; \
	} \
	return errval

#define CALL_FS_IMPL(cls, funcall, errval) \
	GET_FS_INSTANCE(cls); \
	CALL_FS_IMPL_BEGIN() \
		return instance->funcall; \
	CALL_FS_IMPL_END(errval)

	class Hooks
	{
	public:

		static int getattr(const char* path, struct stat* buf)
		{
			CALL_FS_IMPL(FS_getattr, getattr(path, buf), -ENOENT);
		}

		class RealDirectoryFiller : public fusepp::FS_readdir::DirectoryFiller
		{
		public:
			RealDirectoryFiller(void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
				: fusepp::FS_readdir::DirectoryFiller(), _buf(buf), _filler(filler), _offset(offset), _fi(fi)
			{}
			void add(const std::string& name, ino_t id = INVALID_ID)
			{
				if (id != INVALID_ID)
				{
					struct stat s;
					memset(&s, 0, sizeof(s));
					s.st_mode = S_IFREG | 0644;
					s.st_ino = id;
					s.st_nlink = 1;
					_filler(_buf, name.c_str(), &s, 0);
					std::cout << "set using struct stat to ino " << id << std::endl;
				}
				else
				{
					_filler(_buf, name.c_str(), NULL, 0);
				}
			}
		private:
			void* _buf;
			fuse_fill_dir_t _filler;
			off_t _offset;
			fuse_file_info* _fi;
		};

		static int readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
		{
			GET_FS_INSTANCE(FS_readdir);
			CALL_FS_IMPL_BEGIN();
				RealDirectoryFiller f(buf, filler, offset, fi);
				return instance->readdir(path, f);
			CALL_FS_IMPL_END(-EACCES);
		}

	};

#undef GET_FS_INSTANCE
#undef CALL_FS_IMPL

};


int Application::run(int argc, char* argv[])
{
	fuse_operations ops;
	memset(&ops, 0, sizeof(ops));

	FS_getattr* check_getattr = dynamic_cast<FS_getattr*>(_fs.get());
	if (check_getattr) ops.getattr = fusepp_impl::Hooks::getattr;

	FS_readdir* check_readdir = dynamic_cast<FS_readdir*>(_fs.get());
	if (check_readdir) ops.readdir = fusepp_impl::Hooks::readdir;

	fuse_main(argc, argv, &ops, this);
	
	return 0;
}
