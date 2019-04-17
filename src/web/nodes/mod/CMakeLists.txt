set(DIR src/web/nodes/mod)

set(SOURCES
	${SOURCES}
	${DIR}/node_addbond.cpp
	${DIR}/node_addmesh.cpp
	${DIR}/node_addsurface.cpp
	PARENT_SCOPE
)
set(HEADERS
	${HEADERS}
	${DIR}/node_addbond.h
	${DIR}/node_addmesh.h
	${DIR}/node_addsurface.h
	PARENT_SCOPE
)
