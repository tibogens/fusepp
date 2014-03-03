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

#ifndef _FUSEPP_ERROR_H
#define _FUSEPP_ERROR_H

#include <fusepp/Export.h>
#include <string>
#include <stdarg.h>

namespace fusepp
{

FUSEPP_API int LastError();

class FUSEPP_API Error
{
public:
	Error();

	Error(const char* reason, ...);

	static Error System(const std::string& api);

	virtual ~Error();

	inline const std::string& getReason() const { return _reason; }

	friend FUSEPP_API std::ostream& operator<<(std::ostream& stream, const Error& err);

protected:
	void set(const char* reason, ...);
	void format(const char* in, va_list arglist);

private:
	std::string _reason;
};

FUSEPP_API std::ostream& operator<<(std::ostream& stream, const Error& err);

};

#endif // _FUSEPP_ERROR_H
