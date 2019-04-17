set(DIR src)

set(SOURCES
	${SOURCES}
	${DIR}/main.cpp
	${DIR}/hdr.cpp
	${DIR}/lodepng.cpp
	${DIR}/ChokoLait.cpp
	${DIR}/Engine.cpp
	${DIR}/RenderEngine.cpp
	${DIR}/unloader.cpp
	PARENT_SCOPE
)
set(HEADERS
	${HEADERS}
	${DIR}/hdr.h
	${DIR}/lodepng.h
	${DIR}/AssetObjects.h
	${DIR}/ChokoLait.h
	${DIR}/Engine.h
	${DIR}/SceneObjects.h
	${DIR}/unloader.h
	PARENT_SCOPE
)

