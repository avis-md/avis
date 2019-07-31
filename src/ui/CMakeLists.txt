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
