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

set(DIR src/utils)

set(SOURCES
	${SOURCES}
	${DIR}/avi.cpp
	${DIR}/ssh.cpp
	${DIR}/color.cpp
	${DIR}/effects.cpp
	${DIR}/fft.cpp
	${DIR}/net.cpp
	${DIR}/plot.cpp
	${DIR}/random.cpp
	${DIR}/rect.cpp
	${DIR}/spline.cpp
	${DIR}/stream.cpp
	${DIR}/strext.cpp
	${DIR}/xml.cpp
	${DIR}/dialog.cpp
	${DIR}/dylib.cpp
	${DIR}/runcmd.cpp
	${DIR}/tinyfiledialogs.c
	${DIR}/shader.cpp
	${DIR}/glmext.cpp
	${DIR}/uniquecaller.cpp
	PARENT_SCOPE
)
set(HEADERS
	${HEADERS}
	${DIR}/avi.h
	${DIR}/ssh.h
	${DIR}/color.h
	${DIR}/effects.h
	${DIR}/fft.h
	${DIR}/net.h
	${DIR}/plot.h
	${DIR}/random.h
	${DIR}/rect.h
	${DIR}/spline.h
	${DIR}/stream.h
	${DIR}/strext.h
	${DIR}/xml.h
	${DIR}/dialog.h
	${DIR}/dylib.h
	${DIR}/runcmd.h
	${DIR}/tinyfiledialogs.h
	${DIR}/shader.h
	${DIR}/glmext.h
	${DIR}/uniquecaller.h
	${DIR}/linkedlist.h
	${DIR}/linkedlist.inl
	PARENT_SCOPE
)
