set(DIR src/asset)

set(SOURCES
	${SOURCES}
	${DIR}/mesh.cpp
	${DIR}/tetrahedron.cpp
	${DIR}/tube.cpp
	${DIR}/texture.cpp
	${DIR}/texture3d.cpp
	PARENT_SCOPE
)
set(HEADERS
	${HEADERS}
	${DIR}/mesh.h
	${DIR}/tetrahedron.h
	${DIR}/tube.h
	${DIR}/texture.h
	${DIR}/texture3d.h
	PARENT_SCOPE
)
