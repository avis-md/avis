set(DIR src/vis)

set(SOURCES
	${SOURCES}
	${DIR}/pargraphics.cpp
	${DIR}/system.cpp
	${DIR}/shadows.cpp
	${DIR}/renderer.cpp
	${DIR}/cubemarcher.cpp
	${DIR}/selection.cpp
	${DIR}/changelog.cpp
	${DIR}/preferences.cpp
	PARENT_SCOPE
)
set(HEADERS
	${HEADERS}
	${DIR}/pargraphics.h
	${DIR}/system.h
	${DIR}/shadows.h
	${DIR}/renderer.h
	${DIR}/cubemarcher.h
	${DIR}/selection.h
	${DIR}/changelog.h
	${DIR}/preferences.h
	PARENT_SCOPE
)
