set(DIR src/web/cc)

set(SOURCES
	${SOURCES}
	${DIR}/cscript.cpp
	${DIR}/cnode.cpp
	${DIR}/creader.cpp
	PARENT_SCOPE
)
set(HEADERS
	${HEADERS}
	${DIR}/cscript.h
	${DIR}/cnode.h
	${DIR}/creader.h
	PARENT_SCOPE
)
