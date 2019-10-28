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

cmake_minimum_required(VERSION 3.3)
project (avis)

if (UNIX AND NOT APPLE)
	set(LINUX TRUE)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
set(CMAKE_SKIP_RPATH TRUE)

find_package(PythonLibs 3.7 REQUIRED)
find_package(NumPy REQUIRED)
find_package(OpenGL REQUIRED)
find_package(OpenCL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(GLEW REQUIRED)
if (UNIX)
    find_package(LibSSH2UNIX REQUIRED)
else()
    find_package(Libssh2 REQUIRED)
endif()
find_package(JPEG REQUIRED)
find_package(Freetype REQUIRED)
find_package(Threads REQUIRED)

if (MSVC)
	add_definitions(-DPLATFORM_WIN -DWIN32)
elseif (LINUX)
	add_definitions(-DPLATFORM_LNX)
elseif (APPLE)
	add_definitions(-DPLATFORM_OSX -DGL_SILENCE_DEPRECATION -DCL_SILENCE_DEPRECATION)
else()
	message(FATAL_ERROR "Platform not supported!")
endif()

add_definitions(-DUSE_OPENCL -DRR_EMBED_KERNELS)

if (MSVC)
	SET(CMAKE_CXX_FLAGS "/EHsc /W3")
else()
	SET(CMAKE_CXX_FLAGS "-g -Wall -Wextra -Wno-unknown-pragmas -Wno-sign-compare -Wno-unused-parameter -Wno-ignored-attributes")
	add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-std=c++11> $<$<COMPILE_LANGUAGE:C>:-std=c99>)
if (LINUX)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,-rpath,$ORIGIN -static-libstdc++ -fopenmp")
else()
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-narrowing")
endif()
endif()

set(INC_DIRS
	${PROJECT_SOURCE_DIR}/src
	${PYTHON_INCLUDE_DIR}
	${PYTHON_NUMPY_INCLUDE_DIR}
	${GLFW3_INCLUDE_DIR}
	${GLM_INCLUDE_DIR}
	${GLEW_INCLUDE_DIR}
	${LIBSSH2_INCLUDE_DIR}
	${JPEG_INCLUDE_DIR}
	${FREETYPE_INCLUDE_DIRS}
	${OpenCL_INCLUDE_DIR}
	${PROJECT_SOURCE_DIR}/radeonrays/RadeonRays/include
	${PROJECT_SOURCE_DIR}/radeonrays/Calc/inc
	${PROJECT_SOURCE_DIR}/radeonrays/CLW
	${PROJECT_SOURCE_DIR}/libgwavi/inc
	${PROJECT_SOURCE_DIR}/oidn/include
)
if (MSVC)
    set(INC_DIRS
        ${INC_DIRS}
        ${PROJECT_SOURCE_DIR}/src/winonly
    )
endif()

if (MSVC)
    set(LINK_DIRS
        ${PROJECT_SOURCE_DIR}/radeonrays/build/RadeonRays/Release
        ${PROJECT_SOURCE_DIR}/radeonrays/build/Calc/Release
        ${PROJECT_SOURCE_DIR}/radeonrays/build/CLW/Release
        ${PROJECT_SOURCE_DIR}/libgwavi/build/Release
        ${PROJECT_SOURCE_DIR}/oidn/build/Release
    )
else()
    set(LINK_DIRS
        ${PROJECT_SOURCE_DIR}/radeonrays/build/bin
        ${PROJECT_SOURCE_DIR}/radeonrays/build/Calc
        ${PROJECT_SOURCE_DIR}/radeonrays/build/CLW
        ${PROJECT_SOURCE_DIR}/libgwavi/build/
        ${PROJECT_SOURCE_DIR}/oidn/build
    )
endif()

if (APPLE)
set(WEAK -weak_library)
else()
set(WEAK)
endif()

set(LIBS
	${WEAK} ${PYTHON_LIBRARIES}
	${OPENGL_gl_LIBRARY}
	glfw
	${GLM_LIBRARY}
	${GLEW_LIBRARY}
	${JPEG_LIBRARY}
	${FREETYPE_LIBRARIES}
	${OpenCL_LIBRARIES}
	RadeonRays
	Calc
	CLW
	gwavi
	OpenImageDenoise
)
if(MSVC)
    set(LIBS
        ${LIBS}
        Libssh2::libssh2
    )
elseif(APPLE)
    set(LIBS
        ${LIBS}
        ${LIBSSH2_LIBRARY}
        dl
        pthread
        util
    )
else()
    set(LIBS
        ${LIBS}
        ${LIBSSH2_LIBRARY}
        dl
        pthread
        rt
        util
    )
endif()

if (MSVC)
	set(DLLS
        ${TBB_ROOT}/bin/intel64/vc14/tbb.dll
        ${PROJECT_SOURCE_DIR}/radeonrays/build/bin/Release/RadeonRays.dll
        ${PROJECT_SOURCE_DIR}/oidn/build/Release/OpenImageDenoise.dll
	)
elseif (LINUX)
	set(DLLS
		"${PROJECT_SOURCE_DIR}/radeonrays/build/bin/libRadeonRays.so.2.0"
		"${PROJECT_SOURCE_DIR}/oidn/build/libOpenImageDenoise.so.0"
	)
else()
	set(DLLS
		
	)
endif()

if(MSVC)
set(SOURCES res.rc)
else()
set(SOURCES)
endif()
set(HEADERS)

include_directories(${INC_DIRS})
link_directories(${LINK_DIRS})

add_subdirectory("src")
add_subdirectory("src/asset")
add_subdirectory("src/core")
add_subdirectory("src/imp")
add_subdirectory("src/live")
add_subdirectory("src/md")
add_subdirectory("src/ocl")
add_subdirectory("src/scene")
add_subdirectory("src/scene/comp")
add_subdirectory("src/ui")
add_subdirectory("src/utils")
add_subdirectory("src/web")
add_subdirectory("src/web/cc")
add_subdirectory("src/web/ft")
add_subdirectory("src/web/py")
add_subdirectory("src/web/nodes")
add_subdirectory("src/web/nodes/get")
add_subdirectory("src/web/nodes/set")
add_subdirectory("src/web/nodes/ctrl")
add_subdirectory("src/web/nodes/conv")
add_subdirectory("src/web/nodes/mod")
add_subdirectory("src/web/nodes/misc")
add_subdirectory("src/vis")
add_subdirectory("src/xdrfile")
if (MSVC)
    add_subdirectory("src/winonly")
endif()

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin/)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(avis ${SOURCES} ${HEADERS})

target_link_libraries(avis ${LIBS})

file(
	COPY ${PROJECT_SOURCE_DIR}/resources/
	DESTINATION ${PROJECT_BINARY_DIR}/bin/
)

foreach (file_i ${DLLS})
	add_custom_command(
		TARGET avis
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${file_i}
		"${CMAKE_BINARY_DIR}/bin/"
	)
endforeach(file_i)

if (MSVC)
	set (CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/package/win/)
	set (CMAKE_INSTALL_LOCAL_ONLY TRUE)

	install(
		FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/avis.exe
		PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
		DESTINATION bin
	)
	install (DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
		DESTINATION bin
		PATTERN "avis.exe" EXCLUDE
		PATTERN "*.ilk" EXCLUDE
		PATTERN "*.pdb" EXCLUDE
	)
	
    set(CPACK_GENERATOR NSIS)
    set(CPACK_PACKAGE_NAME "AViS")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Analysis and Visualization Software")
    set(CPACK_PACKAGE_VERSION "0.12")
    set(CPACK_PACKAGE_VERSION_MAJOR "0")
    set(CPACK_PACKAGE_VERSION_MINOR "1")
    set(CPACK_PACKAGE_VERSION_PATCH "2")
	set(CPACK_PACKAGE_DIRECTORY ${CMAKE_INSTALL_PREFIX})
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "avis")
	set(CPACK_RESOURCE_FILE_LICENSE ${PROJECT_SOURCE_DIR}/LICENSE)
    set(CPACK_NSIS_MODIFY_PATH ON)
	set(CPACK_PACKAGE_EXECUTABLES avis;AViS)
	set(CPACK_CREATE_DESKTOP_LINKS avis)
	set(CPACK_NSIS_MODIFY_PATH "ON")
	set(CPACK_COMPONENTS_ALL "")
	set(CPACK_CMAKE_INSTALL_LOCAL_ONLY ON)

    INCLUDE(CPack)
elseif (LINUX)
	SET (CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/package/deb/pkg-debian/usr/bin)

	file(REMOVE_RECURSE ${CMAKE_INSTALL_PREFIX}/avis-md/)

	install(
		FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/avis
		PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
		DESTINATION ${CMAKE_INSTALL_PREFIX}/avis-md/
	)
	install(
		FILES ${PROJECT_SOURCE_DIR}/avis.desktop
		PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
		DESTINATION ${CMAKE_INSTALL_PREFIX}/avis-md/
	)
	install (DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
		DESTINATION ${CMAKE_INSTALL_PREFIX}/avis-md/
		PATTERN "avis" EXCLUDE
	)

	add_custom_target(
		debian
		COMMAND
		"${CMAKE_COMMAND}" --build . --target install
		COMMAND
		cd "${PROJECT_SOURCE_DIR}/package/deb/" && dpkg -b pkg-debian/ avis_amd64.deb
		WORKING_DIRECTORY
		"${CMAKE_BINARY_DIR}"
	)
else()
	SET (CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/package/mac/AViS.app/Contents)

	install(
		FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/avis
		PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
		DESTINATION ${CMAKE_INSTALL_PREFIX}/MacOS/
	)
	install (DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
		DESTINATION ${CMAKE_INSTALL_PREFIX}/MacOS/
		PATTERN "avis" EXCLUDE
	)

	add_custom_target(
		app
		COMMAND
		"${CMAKE_COMMAND}" --build . --target install
		COMMAND
		"${PYTHON_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/modify_rpath.py"
		WORKING_DIRECTORY
		"${CMAKE_BINARY_DIR}"
	)

endif()
