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

set(DIR src/web/nodes/set)

set(SOURCES
	${SOURCES}
	${DIR}/node_setradscl.cpp
	${DIR}/node_showrange.cpp
	${DIR}/node_setattribute.cpp
	${DIR}/node_setbboxcenter.cpp
	PARENT_SCOPE
)
set(HEADERS
	${HEADERS}
	${DIR}/node_setradscl.h
	${DIR}/node_showrange.h
	${DIR}/node_setattribute.h
	${DIR}/node_setbboxcenter.h
	PARENT_SCOPE
)
