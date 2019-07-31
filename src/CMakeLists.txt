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

