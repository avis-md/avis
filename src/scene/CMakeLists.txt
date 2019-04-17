set(DIR src/scene)

set(SOURCES
	${SOURCES}
	${DIR}/object.cpp
	${DIR}/scene.cpp
	${DIR}/sceneobject.cpp
	${DIR}/transform.cpp
	PARENT_SCOPE
)
set(HEADERS
	${HEADERS}
	${DIR}/object.h
	${DIR}/scene.h
	${DIR}/sceneobject.h
	${DIR}/transform.h
	PARENT_SCOPE
)
