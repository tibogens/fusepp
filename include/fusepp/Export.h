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

#ifndef _FUSEPP_EXPORT_H
#define _FUSEPP_EXPORT_H

#ifdef _WIN32
	#ifdef libfusepp_STATIC
		#define FUSEPP_API
	#else
		#ifdef libfusepp_EXPORTS
			#define FUSEPP_API __declspec(dllexport)
		#else
			#define FUSEPP_API __declspec(dllimport)
		#endif
	#endif
#else
	#define FUSEPP_API
#endif


#endif //_FUSEPP_EXPORT_H