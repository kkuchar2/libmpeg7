#!/bin/bash

COLOR_CYAN="\e[96m"
COLOR_GREEN="\e[92m"
COLOR_RED="\e[91m"
COLOR_YELLOW="\e[93m"
COLOR_RESET="\e[0m"

BUILD_PATH="build"

MACHINE_TYPE=`uname -m`

if [ ${MACHINE_TYPE} == "x86_64" ]; then
  DEBUG_PATH="$BUILD_PATH/x64/Debug"
  RELEASE_PATH="$BUILD_PATH/x64/Release"
else
  DEBUG_PATH="$BUILD_PATH/x32/Debug"
  RELEASE_PATH="$BUILD_PATH/x32/Release"
fi

build_debug() {
    mkdir -p ${DEBUG_PATH}
    
    echo
    echo -e "$COLOR_CYAN ---------- GENERATING MAKEFILE ----------"
    echo

    cd $DEBUG_PATH

    cmake -DCMAKE_BUILD_TYPE=Debug "../../.."

    echo
    echo -e "$COLOR_CYAN ---------- BUILDING SHARED AND STATIC LIBRARY IN DEBUG CONFIG ----------"
    echo
    
    make -j 4

    cd "../../.."
}

build_release() { 
    mkdir -p ${RELEASE_PATH}
    
    echo
    echo -e "$COLOR_CYAN ---------- GENERATING MAKEFILE ----------"
    echo

    cd $RELEASE_PATH
    cmake -DCMAKE_BUILD_TYPE=Release "../../.."

    echo
    echo -e "$COLOR_CYAN ---------- BUILDING SHARED AND STATIC LIBRARY IN RELEASE CONFIG ----------"
    echo

    make -j 4

    cd "../../.."
}

build_all() {
    build_debug
    build_release
}

clean_project() {
  echo -e "$COLOR_CYAN Cleaning project..."

  if [ -d "${DEBUG_PATH}" ]; then
    rm -r ${DEBUG_PATH}
  fi
  
  if [ -d "${RELEASE_PATH}" ]; then
    rm -r ${RELEASE_PATH}
  fi
    echo -e "$COLOR_GREEN Project has been cleaned."
}

unknown_command() {
   echo -e "$COLOR_RED Unknown command (forgot to set a flag?)"
   echo -e "$COLOR_RESET Type 'build.sh -h' to show help."
}

ascii_art() {
   cat << "EOF"
  _ _ _                               ______             
 | (_) |                             |____  |            
 | |_| |__  _ __ ___  _ __   ___  __ _   / / ____      __
 | | | '_ \| '_ ` _ \| '_ \ / _ \/ _` | / / '_ \ \ /\ / /
 | | | |_) | | | | | | |_) |  __/ (_| |/ /| |_) \ V  V / 
 |_|_|_.__/|_| |_| |_| .__/ \___|\__, /_/ | .__/ \_/\_/  
                     | |          __/ |   | |            
                     |_|         |___/    |_| BUILD
EOF
}

if [ "$1" == "-c" ]; then
    clean_project
elif [ "$1" == "-dbg" ]; then
    ascii_art
    build_debug
elif [ "$1" == "-rel" ]; then
    ascii_art
    build_release
elif [ "$1" == "-all" ]; then
    ascii_art
    build_all
elif [ "$1" == "-h" ]; then
    echo 
    echo "------------- HELP -------------"
    echo "-c   - Clear project."
    echo "-dbg - Debug configuration build."
    echo "-rel - Release configuration build."
    echo "-all - Debug and Release configuration build"
else
    unknown_command
fi

echo -e $COLOR_RESET

