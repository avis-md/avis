set(DIR src/scene/comp)

set(SOURCES
	${SOURCES}
	${DIR}/camera.cpp
	${DIR}/component.cpp
	PARENT_SCOPE
)
set(HEADERS
	${HEADERS}
	${DIR}/camera.h
	${DIR}/component.h
	PARENT_SCOPE
)
