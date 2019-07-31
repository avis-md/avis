# Copyright (C) 2019 Pua Kai
# 
# This file is part of AViS.
# 
# AViS is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# AViS is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with AViS.  If not, see <http://www.gnu.org/licenses/>.

set(DIR src/web)

set(SOURCES
	${SOURCES}
	${DIR}/anweb.cpp
	${DIR}/anscript.cpp
	${DIR}/annode.cpp
	${DIR}/annode_internal.cpp
	${DIR}/anbrowse.cpp
	${DIR}/anops.cpp
	${DIR}/errorview.cpp
	${DIR}/arrayview.cpp
	${DIR}/dmscript.cpp
	PARENT_SCOPE
)
set(HEADERS
	${HEADERS}
	${DIR}/anweb.h
	${DIR}/anscript.h
	${DIR}/annode.h
	${DIR}/annode_internal.h
	${DIR}/anbrowse.h
	${DIR}/anops.h
	${DIR}/errorview.h
	${DIR}/arrayview.h
	${DIR}/dmscript.h
	PARENT_SCOPE
)

