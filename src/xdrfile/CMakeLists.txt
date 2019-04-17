set(DIR src/xdrfile)

set(SOURCES
	${SOURCES}
	${DIR}/xdrfile.c
	${DIR}/xdrfile_trr.c
	${DIR}/xdrfile_xtc.c
	PARENT_SCOPE
)
set(HEADERS
	${HEADERS}
	${DIR}/xdrfile.h
	${DIR}/xdrfile_trr.h
	${DIR}/xdrfile_xtc.h
	PARENT_SCOPE
)
