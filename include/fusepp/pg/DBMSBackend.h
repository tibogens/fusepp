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

#ifndef _FUSEPP_PG_DBMSBACKEND_H
#define _FUSEPP_PG_DBMSBACKEND_H

#include <fusepp/pg/Export.h>

// Forward declare internal PostgreSQL types so that we can have them
// as class members without forcing clients to include <libpq-fe.h> as well.
// This softens the depedency of fusepp-pg on PostgreSQL
#ifndef LIBPQ_FE_H
struct PGconn;
struct PGresult;
typedef unsigned int Oid;
#endif

#endif // _FUSEPP_PG_DBMSBACKEND_H
