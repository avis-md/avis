set(DIR src/web/nodes/get)

set(SOURCES
	${SOURCES}
	${DIR}/node_inputs.cpp
	${DIR}/node_inputs_v.cpp
	${DIR}/node_info.cpp
	${DIR}/node_getattribute.cpp
	PARENT_SCOPE
)
set(HEADERS
	${HEADERS}
	${DIR}/node_inputs.h
	${DIR}/node_inputs_v.h
	${DIR}/node_info.h
	${DIR}/node_getattribute.h
	PARENT_SCOPE
)
