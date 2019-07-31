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

set(DIR src/core)

set(SOURCES
	${SOURCES}
	${DIR}/bbox.cpp
	${DIR}/debug.cpp
	${DIR}/display.cpp
	${DIR}/font.cpp
	${DIR}/input.cpp
	${DIR}/io.cpp
	${DIR}/math.cpp
	${DIR}/mvp.cpp
	${DIR}/time.cpp
	${DIR}/ui.cpp
	${DIR}/ui3.cpp
	PARENT_SCOPE
)
set(HEADERS
	${HEADERS}
	${DIR}/bbox.h
	${DIR}/debug.h
	${DIR}/display.h
	${DIR}/font.h
	${DIR}/input.h
	${DIR}/io.h
	${DIR}/math.h
	${DIR}/mvp.h
	${DIR}/time.h
	${DIR}/ui.h
	${DIR}/ui3.h
	PARENT_SCOPE
)
