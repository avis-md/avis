cmake_minimum_required(VERSION 3.0)
project (avis)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

find_package(PythonLibs 3.7 REQUIRED)
find_package(NumPy REQUIRED)
find_package(OpenGL REQUIRED)
find_package(OpenCL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(GLEW REQUIRED)
find_package(LibSSH2 REQUIRED)
find_package(JPEG REQUIRED)
find_package(Freetype REQUIRED)
find_package(Threads REQUIRED)

add_definitions(-DPLATFORM_LNX -DUSE_OPENCL -DRR_EMBED_KERNELS)

SET(CMAKE_CXX_FLAGS "-std=c++11")

set(INC_DIRS
	"${PROJECT_SOURCE_DIR}/src"
	${PYTHON_INCLUDE_DIR}
	${PYTHON_NUMPY_INCLUDE_DIR}
	${GLFW3_INCLUDE_DIR}
	${GLM_INCLUDE_DIR}
	${GLEW_INCLUDE_DIR}
	${LIBSSH2_INCLUDE_DIR}
	${JPEG_INCLUDE_DIR}
	${FREETYPE_INCLUDE_DIRS}
	${OpenCL_INCLUDE_DIR}
	${PROJECT_SOURCE_DIR}/../radeonrays/RadeonRays/include
	${PROJECT_SOURCE_DIR}/../radeonrays/Calc/inc
	${PROJECT_SOURCE_DIR}/../radeonrays/CLW
	${PROJECT_SOURCE_DIR}/../libgwavi/inc
	${PROJECT_SOURCE_DIR}/../oidn/include
)
set(LINK_DIRS
	${PROJECT_SOURCE_DIR}/../radeonrays/build/bin
	${PROJECT_SOURCE_DIR}/../radeonrays/build/Calc
	${PROJECT_SOURCE_DIR}/../radeonrays/build/CLW
	${PROJECT_SOURCE_DIR}/../libgwavi/lib
	${PROJECT_SOURCE_DIR}/../oidn/build
)
set(LIBS
	${PYTHON_LIBRARIES}
	${OPENGL_gl_LIBRARY}
	glfw
	${GLM_LIBRARY}
	${GLEW_LIBRARY}
	${LIBSSH2_LIBRARY}
	${JPEG_LIBRARY}
	${FREETYPE_LIBRARIES}
	${OpenCL_LIBRARIES}
	dl
	rt
	pthread
	util
	RadeonRays
	Calc
	CLW
	gwavi
	OpenImageDenoise
)

set(DLLS
	"${PROJECT_SOURCE_DIR}/../radeonrays/build/bin/libRadeonRays.so.2.0"
	"${PROJECT_SOURCE_DIR}/../oidn/build/libOpenImageDenoise.so.0"
)

set(SOURCES)
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
