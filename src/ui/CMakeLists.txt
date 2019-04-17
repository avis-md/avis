set(DIR src/ui)

set(SOURCES
	${SOURCES}
	${DIR}/popups.cpp
	${DIR}/icons.cpp
	${DIR}/help.cpp
	${DIR}/ui_ext.cpp
	${DIR}/localizer.cpp
	${DIR}/browse.cpp
	${DIR}/localbrowsetarget.cpp
	${DIR}/remotebrowsetarget.cpp
	PARENT_SCOPE
)
set(HEADERS
	${HEADERS}
	${DIR}/popups.h
	${DIR}/icons.h
	${DIR}/help.h
	${DIR}/ui_ext.h
	${DIR}/localizer.h
	${DIR}/browse.h
	${DIR}/browsetarget.h
	PARENT_SCOPE
)
