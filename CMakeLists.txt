if(NOT WIN32)
  string(ASCII 27 Esc)
  set(ColourReset "${Esc}[m")
  set(Yellow      "${Esc}[33m")
  set(Blue        "${Esc}[34m")
  set(Cyan        "${Esc}[36m")
  set(Green       "${Esc}[32m")
  set(Red         "${Esc}[31m")
endif()

# ------ Create library makefile  -------------------------------------------------
message("${Yellow} libmpeg7 library - Krzysztof Kucharski ${ColourReset}")

# ------ Set project -------------------------------------------------
project (LIBMPEG7_PROJECT)
cmake_minimum_required (VERSION 2.8)
# ------ Set compilation flags -------------------------------------------------
set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG")
set(CMAKE_CXX_FLAGS "-DBUILD_SHARED_LIBS=OFF")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# ------ Load OpenCV library----------------------------------------------------
message("${Cyan}-- Loading OpenCV library ${ColourReset}")

find_package(OpenCV REQUIRED)

if(OpenCV_FOUND)
	message("${Green}   Package OpenCV found!")
	include_directories(${OpenCV_INCLUDE_DIRS})
endif()

# ------ Load Java and JNI --------------------------------------------------------------
message("${Cyan}-- Loading Java and JNI ${ColourReset}")

find_package(Java REQUIRED)
find_package(JNI REQUIRED)

if(JNI_FOUND)
	message("${Green}   JNI found! ${ColourReset}")
	include_directories(${JAVA_AWT_INCLUDE_PATH} ${JAVA_INCLUDE_PATH2})
endif()

# ------ Scan for all .cpp files -----------------------------------------------
file(GLOB_RECURSE SRC_FILES ${PROJECT_SOURCE_DIR}/sources/*.cpp)

SET(CMAKE_DEBUG_POSTFIX _d)

add_library(mpeg7 SHARED ${SRC_FILES})
add_library(mpeg7_s STATIC ${SRC_FILES})

target_link_libraries(mpeg7 ${OpenCV_LIBS})

