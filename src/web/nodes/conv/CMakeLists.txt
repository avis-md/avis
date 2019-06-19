set(DIR src/web/nodes/conv)

set(SOURCES
	${SOURCES}
	${DIR}/node_adjlist.cpp
	${DIR}/node_downsample.cpp
	PARENT_SCOPE
)
set(HEADERS
	${HEADERS}
	${DIR}/node_adjlist.h
	${DIR}/node_downsample.h
	PARENT_SCOPE
)
