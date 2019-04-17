set(DIR src/md)

set(SOURCES
	${SOURCES}
	${DIR}/parmenu.cpp
	${DIR}/particles.cpp
	${DIR}/particles_animdata.cpp
	${DIR}/particles_attrdata.cpp
	${DIR}/Protein.cpp
	PARENT_SCOPE
)
set(HEADERS
	${HEADERS}
	${DIR}/parmenu.h
	${DIR}/particles.h
	${DIR}/Protein.h
	PARENT_SCOPE
)
