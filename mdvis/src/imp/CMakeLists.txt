set(DIR src/imp)

set(SOURCES
	${SOURCES}
	${DIR}/parloader.cpp
	${DIR}/GenericSSV.cpp
	${DIR}/Gromacs.cpp
	${DIR}/pdb.cpp
	${DIR}/pdbx.cpp
	${DIR}/lammps.cpp
	${DIR}/CDV.cpp
	${DIR}/dlpoly.cpp
	PARENT_SCOPE
)
set(HEADERS
	${HEADERS}
	${DIR}/parloader.h
	${DIR}/GenericSSV.h
	${DIR}/Gromacs.h
	${DIR}/pdb.h
	${DIR}/pdbx.h
	${DIR}/lammps.h
	${DIR}/CDV.h
	${DIR}/dlpoly.h
	PARENT_SCOPE
)
