SET(SAMPLE_NAME pg)
SET(PROGRAM_NAME sample_${SAMPLE_NAME})

SET(PLUGIN_SRC
	main.cpp
)

IF (FUSEPP_STATIC)
	ADD_DEFINITIONS(-Dlibfuse_STATIC)
ENDIF (FUSEPP_STATIC)

ADD_EXECUTABLE (${PROGRAM_NAME}
	${PLUGIN_SRC}
)

TARGET_LINK_LIBRARIES (${PROGRAM_NAME} libfusepp fusepp-pg
) 

SET_TARGET_PROPERTIES(${PROGRAM_NAME} PROPERTIES PROJECT_LABEL "sample - ${SAMPLE_NAME}")
