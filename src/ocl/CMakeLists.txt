set(DIR src/ocl)

set(SOURCES
	${SOURCES}
	${DIR}/raytracer.cpp
	${DIR}/denoise.cpp
	PARENT_SCOPE
)
set(HEADERS
	${HEADERS}
	${DIR}/raytracer.h
	${DIR}/kernel.h
	${DIR}/denoise.h
	PARENT_SCOPE
)
