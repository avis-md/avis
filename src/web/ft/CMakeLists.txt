set(DIR src/web/ft)

set(SOURCES
	${SOURCES}
	${DIR}/fscript.cpp
	${DIR}/fnode.cpp
	${DIR}/freader.cpp
	PARENT_SCOPE
)
set(HEADERS
	${HEADERS}
	${DIR}/fscript.h
	${DIR}/fnode.h
	${DIR}/freader.h
	PARENT_SCOPE
)
