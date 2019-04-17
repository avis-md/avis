set(DIR src/web/py)

set(SOURCES
	${SOURCES}
	${DIR}/pyscript.cpp
	${DIR}/pynode.cpp
	${DIR}/pyarr.cpp
	${DIR}/pyreader.cpp
	PARENT_SCOPE
)
set(HEADERS
	${HEADERS}
	${DIR}/pyscript.h
	${DIR}/pynode.h
	${DIR}/pyarr.h
	${DIR}/pyreader.h
	PARENT_SCOPE
)
