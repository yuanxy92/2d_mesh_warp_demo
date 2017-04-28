# CAMKE file of gigapixel from video
#
# Author: Shane Yuan
# Date: Sep 22, 2015
#

cmake_minimum_required(VERSION 2.8)
project( GigaStitcher)

if ( WIN32 )
	set(OpenCV_DIR "E:/ShaneYuan/lib/opencv/x64/vc12/lib")
	set(OpenCV_CONFIG_PATH "E:/ShaneYuan/lib/opencv/")
	set(WIN_HEADER_PATH "E:/ShaneYuan/Project/gigavideo/winheadfile")
    include_directories(${WIN_HEADER_PATH})
endif()

# OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})


file(GLOB HEADERS  *.h *.hpp)
set(SRC_LIST "main.cpp" "CPURender.cpp" ${HEADERS})

# OpenMP
if (WIN32)
	set(CMAKE_CXX_FLAGS_RELEASE "/openmp")
elseif( UNIX )
    find_package(OpenMP)
    if(OPENMP_FOUND)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
    endif()
endif()

#c++11
if (UNIX)
    SET(CMAKE_CXX_FLAGS "-std=c++11")
endif()

add_executable( GigaStitcher ${SRC_LIST} )
target_link_libraries( GigaStitcher ${OpenCV_LIBS} )
