set(DIR src/web/nodes/misc)

set(SOURCES
	${SOURCES}
	${DIR}/node_plot.cpp
	${DIR}/node_accum.cpp
	PARENT_SCOPE
)
set(HEADERS
	${HEADERS}
	${DIR}/node_plot.h
	${DIR}/node_accum.h
	PARENT_SCOPE
)
