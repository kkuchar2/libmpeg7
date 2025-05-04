if(NOT WIN32)
  string(ASCII 27 Esc)
  set(ColourReset "${Esc}[m")
  set(Yellow      "${Esc}[33m")
  set(Blue        "${Esc}[34m")
  set(Cyan        "${Esc}[36m")
  set(Green       "${Esc}[32m")
  set(Red         "${Esc}[31m")
endif()

message("${Yellow}libmpeg7 library - Krzysztof Kucharski ${ColourReset}")
cmake_minimum_required(VERSION 3.5)
project(LIBMPEG7_PROJECT)

# Improved debug flags
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_C_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS "-DBUILD_SHARED_LIBS=OFF")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_DEBUG_POSTFIX _d)

# Ensure this is a debug build when built through CLion
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

find_package(Java REQUIRED)
find_package(JNI REQUIRED)

if(JNI_FOUND)
	message("${Green}JNI found! ${ColourReset}")
    set(JAVA_AWT_INCLUDE_PATH "/usr/lib/jvm/java-21-openjdk-amd64/include")
    set(JAVA_INCLUDE_PATH2 "/usr/lib/jvm/java-21-openjdk-amd64/include/linux")
	include_directories(${JAVA_AWT_INCLUDE_PATH} ${JAVA_INCLUDE_PATH2})
endif()

file(GLOB_RECURSE LIB_SRC_FILES ${PROJECT_SOURCE_DIR}/sources/*.cpp)
# Exclude main.cpp from the library sources
list(FILTER LIB_SRC_FILES EXCLUDE REGEX ".*main\\.cpp$")

add_library(mpeg7 SHARED ${LIB_SRC_FILES})
add_library(mpeg7_s STATIC ${LIB_SRC_FILES})

# Add executable target using main.cpp
add_executable(mpeg7_app ${PROJECT_SOURCE_DIR}/sources/main.cpp)
target_link_libraries(mpeg7_app mpeg7_s)

target_link_libraries(mpeg7)