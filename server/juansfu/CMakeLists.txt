cmake_minimum_required(VERSION 3.14)

project(juansfu)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/)

set(CMAKE_CXX_STANDARD 17)

add_definitions(-DWEBRTC_POSIX)

# SET(CMAKE_C_FLAGS "-std=c11 -O2 -W -w")
# SET(CMAKE_CXX_FLAGS "-std=c++11 -W -w")
# set(CMAKE_CXX_FLAGS_DEBUG "-D_DEBUG -g -ggdb")
# set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

if (CMAKE_BUILD_TYPE STREQUAL Release)
	set(CMAKE_CXX_FLAGS "-W -w -DNDEBUG -O2")
else()
	set(CMAKE_CXX_FLAGS "-W -w -D_DEBUG -g")
endif()

find_package(PkgConfig REQUIRED)
if(NOT PKG_CONFIG_FOUND)
    message(FATAL_ERROR "pkg-config not found")
endif()

pkg_search_module(LIBUV REQUIRED libuv)
if(NOT LIBUV_FOUND)
    message(FATAL_ERROR "libuv not found")
endif()
include_directories(${LIBUV_INCLUDE_DIRS})
link_directories(${LIBUV_LIBRARY_DIRS})

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

include_directories(/usr/local/include)
include_directories(${CMAKE_SOURCE_DIR}
include_directories(${CMAKE_SOURCE_DIR}/..)
include_directories(${CMAKE_SOURCE_DIR}/../../..)
include_directories(${CMAKE_SOURCE_DIR}/../../../uvnet)

# add_subdirectory(${CMAKE_SOURCE_DIR}/pActor)
# add_subdirectory(${CMAKE_SOURCE_DIR}/genernal)
# add_subdirectory(${CMAKE_SOURCE_DIR}/UvNetBase)

# file(GLOB LIBS "*.a" "pActor/libs" "genernal/libs" "UvNetBase/libs")
SET(EXTRA_LIBS uvnet)
SET(LIBS uv ssl crypto pthread jsoncpp)

aux_source_directory(. SRCS)
#aux_source_directory(../backward SRCS)
aux_source_directory(./api SRCS)
aux_source_directory(./rtc_base SRCS)
aux_source_directory(./rtc_base/base64 SRCS)
aux_source_directory(./rtc_base/deprecated SRCS)
aux_source_directory(./rtc_base/internal SRCS)
aux_source_directory(./rtc_base/numerics SRCS)
aux_source_directory(./rtc_base/sigslot SRCS)
aux_source_directory(./rtc_base/strings SRCS)
aux_source_directory(./rtc_base/synchronization SRCS)
aux_source_directory(./rtc_base/system SRCS)
aux_source_directory(./rtc_base/task_utils SRCS)
aux_source_directory(./sdp SRCS)
aux_source_directory(./signaling SRCS)
aux_source_directory(./system_wrappers/source SRCS)
aux_source_directory(./udp SRCS)
aux_source_directory(./utils SRCS)

# message(STATUS ${SRCS})

link_directories(${CMAKE_SOURCE_DIR}/libs)
link_directories(/usr/local/lib)
link_directories(/usr/local/lib64)

# export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
add_executable(${PROJECT_NAME} ${SRCS})
target_link_libraries(${PROJECT_NAME} ${EXTRA_LIBS} ${LIBS})
