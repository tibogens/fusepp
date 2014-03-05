SET(LIB_NAME fusepp-pg)

IF (NOT WIN32)
find_package(PkgConfig REQUIRED)
pkg_check_modules(FUSE fuse REQUIRED)
include_directories(${FUSE_INCLUDEDIR})
add_definitions(${FUSE_CFLAGS})

find_package(PostgreSQL REQUIRED)
include_directories(${PostgreSQL_INCLUDE_DIR})

ELSE (NOT WIN32)
include_directories(${fusepp_SOURCE_DIR}/fuse-winstub)
add_definitions(-D_FILE_OFFSET_BITS=64)
ENDIF (NOT WIN32)

SET(HEADER_PATH ${fusepp_SOURCE_DIR}/include/fusepp/pg)

SET(LIB_PUBLIC_HEADERS
	${HEADER_PATH}/CompositeQuery.h
	${HEADER_PATH}/CrudObject.h
	${HEADER_PATH}/Database.h
	${HEADER_PATH}/DBMSBackend.h
	${HEADER_PATH}/Export.h
	${HEADER_PATH}/Query.h
	${HEADER_PATH}/SQLio.h
)

SET(LIB_SRC
	CompositeQuery.cpp
	CrudObject.cpp
	Database.cpp
	Query.cpp
	SQLio.cpp
)

IF (FUSEPP_STATIC)
	SET(LIB_STATIC_SHARED STATIC)
	ADD_DEFINITIONS(-Dlibfusepp_STATIC)
ELSE (FUSEPP_STATIC)
	SET(LIB_STATIC_SHARED SHARED)
ENDIF (FUSEPP_STATIC)

ADD_LIBRARY(${LIB_NAME}
	${LIB_STATIC_SHARED}
    ${LIB_PUBLIC_HEADERS}
	${LIB_SRC}
)

TARGET_LINK_LIBRARIES(${LIB_NAME} ${FUSE_LIBRARIES} ${PostgreSQL_LIBRARY}
)

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES PROJECT_LABEL "module - fusepp-pg")
