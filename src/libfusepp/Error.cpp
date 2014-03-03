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

#include <fusepp/Error.h>
#ifndef _WIN32
#include <errno.h>
#include <stdio.h>
#else
#include <windows.h>
#endif
#include <iostream>
using namespace fusepp;

Error::Error()
{
#ifdef _DEBUG
	std::cout << "DEBUG: thrown fusepp::Error (without message)" << std::endl;
#endif
}

Error::~Error()
{
}

Error::Error(const char* reason, ...)
{
	va_list arglist;
	va_start(arglist, reason);
	format(reason, arglist);
	
#ifdef _DEBUG
	std::cout << "DEBUG: thrown fusepp::Error : " << _reason << std::endl;
#endif
}

void Error::set(const char* reason, ...)
{
	va_list arglist;
	va_start(arglist, reason);
	format(reason, arglist);
}

int myvsnprintf(char* buffer, size_t bufsize, const char* format, va_list arglist);

void Error::format(const char* in, va_list arglist)
{
	char buffer[2048];
	myvsnprintf(buffer, 2048, in, arglist);
	va_end(arglist);
	_reason = buffer;
}

std::ostream& fusepp::operator<<(std::ostream& stream, const Error& err)
{
	stream << err._reason;
	return stream;
}




int fusepp::LastError()
{
#ifdef _WIN32
	return (int)GetLastError();
#else
	return errno;
#endif
}



int myvsnprintf(char* buffer, size_t bufsize, const char* format, va_list arglist)
{
#ifdef _WIN32
	return vsnprintf_s(buffer, bufsize, _TRUNCATE, format, arglist);
#else
	return vsnprintf(buffer, bufsize, format, arglist);
#endif
}


Error Error::System(const std::string& api)
{
	LPVOID lpvMessageBuffer;
	CHAR szPrintBuffer[512];
	DWORD nCharsWritten;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpvMessageBuffer, 0, NULL);

	wsprintf(szPrintBuffer,
		"ERROR: API    = %s.\n   error code = %d.\n   message    = %s.\n",
		api.c_str(), GetLastError(), (char *)lpvMessageBuffer);

	WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE),szPrintBuffer,
		lstrlen(szPrintBuffer),&nCharsWritten,NULL);

	std::string copy = szPrintBuffer;
	LocalFree(lpvMessageBuffer);

	return Error(copy.c_str());
}

