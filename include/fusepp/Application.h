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

#ifndef _FUSEPP_APPLICATION_H
#define _FUSEPP_APPLICATION_H

#include <memory>
#include <fusepp/Export.h>
#include <fusepp/FileSystem.h>

namespace fusepp_impl { class Hooks; };

namespace fusepp
{

class FUSEPP_API Application
{
public:
	Application(std::shared_ptr<FileSystem>& fs);
	virtual ~Application();

	int run(int argc, char* argv[]);
	

private:
	static Application* _s_instance;
	friend class fusepp_impl::Hooks;
	std::shared_ptr<FileSystem> _fs;
};

};

#endif //_FUSEPP_APPLICATION_H
