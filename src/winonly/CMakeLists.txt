set(DIR src/winonly)

set(SOURCES
	${SOURCES}
	PARENT_SCOPE
)
set(HEADERS
	${HEADERS}
	${DIR}/minimalwin.h
	${DIR}/unistd.h
	PARENT_SCOPE
)
