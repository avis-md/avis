set(DIR src/web)

set(SOURCES
	${SOURCES}
	${DIR}/anweb.cpp
	${DIR}/anscript.cpp
	${DIR}/annode.cpp
	${DIR}/annode_internal.cpp
	${DIR}/anbrowse.cpp
	${DIR}/anops.cpp
	${DIR}/errorview.cpp
	${DIR}/arrayview.cpp
	${DIR}/dmscript.cpp
	PARENT_SCOPE
)
set(HEADERS
	${HEADERS}
	${DIR}/anweb.h
	${DIR}/anscript.h
	${DIR}/annode.h
	${DIR}/annode_internal.h
	${DIR}/anbrowse.h
	${DIR}/anops.h
	${DIR}/errorview.h
	${DIR}/arrayview.h
	${DIR}/dmscript.h
	PARENT_SCOPE
)

