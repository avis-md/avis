set(DIR src/web/nodes/set)

set(SOURCES
	${SOURCES}
	${DIR}/node_setradscl.cpp
	${DIR}/node_showrange.cpp
	${DIR}/node_setattribute.cpp
	${DIR}/node_setbboxcenter.cpp
	PARENT_SCOPE
)
set(HEADERS
	${HEADERS}
	${DIR}/node_setradscl.h
	${DIR}/node_showrange.h
	${DIR}/node_setattribute.h
	${DIR}/node_setbboxcenter.h
	PARENT_SCOPE
)
