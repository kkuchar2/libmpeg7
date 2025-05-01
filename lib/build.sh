#!/bin/bash

COLOR_CYAN="\e[96m"
COLOR_GREEN="\e[92m"
COLOR_RED="\e[91m"
COLOR_RESET="\e[0m"
readonly COLOR_CYAN COLOR_GREEN COLOR_RED COLOR_RESET

BUILD_PATH="lib/build"
readonly BUILD_PATH

MACHINE_TYPE=$(uname -m)
readonly MACHINE_TYPE

if [[ "${MACHINE_TYPE}" == "x86_64" ]]; then
  DEBUG_PATH="${BUILD_PATH}/x64/Debug"
  RELEASE_PATH="${BUILD_PATH}/x64/Release"
else
  DEBUG_PATH="${BUILD_PATH}/x32/Debug"
  RELEASE_PATH="${BUILD_PATH}/x32/Release"
fi
readonly DEBUG_PATH RELEASE_PATH

build_debug() {
    echo
    echo -e "${COLOR_CYAN}BUILDING DEBUG VERSION${COLOR_RESET}"
    echo

    mkdir -p "${DEBUG_PATH}"
    pushd "${DEBUG_PATH}" > /dev/null || {
      echo -e "${COLOR_RED}Failed to change to debug directory${COLOR_RESET}"
      return 1
    }

    echo -e "${COLOR_CYAN}Generating Makefile...${COLOR_RESET}"
    if ! cmake -DCMAKE_BUILD_TYPE=Debug "../../.."; then
      echo -e "${COLOR_RED}CMake configuration failed${COLOR_RESET}"
      popd > /dev/null || true
      return 1
    fi

    echo -e "${COLOR_CYAN}Building shared and static library (Debug)...${COLOR_RESET}"
    if ! make -j "$(nproc)"; then
      echo -e "${COLOR_RED}Build failed${COLOR_RESET}"
      popd > /dev/null || true
      return 1
    fi

    popd > /dev/null || true
    echo -e "${COLOR_GREEN}Debug build completed successfully${COLOR_RESET}"
    return 0
}

build_release() {
    echo
    echo -e "${COLOR_CYAN}BUILDING RELEASE VERSION${COLOR_RESET}"
    echo

    mkdir -p "${RELEASE_PATH}"
    pushd "${RELEASE_PATH}" > /dev/null || {
      echo -e "${COLOR_RED}Failed to change to release directory${COLOR_RESET}"
      return 1
    }

    echo -e "${COLOR_CYAN}Generating Makefile...${COLOR_RESET}"
    if ! cmake -DCMAKE_BUILD_TYPE=Release "../../.."; then
      echo -e "${COLOR_RED}CMake configuration failed${COLOR_RESET}"
      popd > /dev/null || true
      return 1
    fi

    echo -e "${COLOR_CYAN}Building shared and static library (Release)...${COLOR_RESET}"
    if ! make -j "$(nproc)"; then
      echo -e "${COLOR_RED}Build failed${COLOR_RESET}"
      popd > /dev/null || true
      return 1
    fi

    popd > /dev/null || true
    echo -e "${COLOR_GREEN}Release build completed successfully${COLOR_RESET}"
    return 0
}

build_all() {
    local result=0

    if ! build_debug; then
        result=1
    fi

    if ! build_release; then
        result=1
    fi

    return $result
}

clean_project() {
  echo -e "${COLOR_CYAN}Cleaning project...${COLOR_RESET}"

  if [[ -d "${DEBUG_PATH}" ]]; then
    rm -rf "${DEBUG_PATH}"
  fi

  if [[ -d "${RELEASE_PATH}" ]]; then
    rm -rf "${RELEASE_PATH}"
  fi

  echo -e "${COLOR_GREEN}Project has been cleaned.${COLOR_RESET}"
  return 0
}

show_help() {
    echo
    echo "------------- HELP -------------"
    echo "-c   - Clean project"
    echo "-dbg - Build debug configuration"
    echo "-rel - Build release configuration"
    echo "-all - Build both debug and release configurations"
    echo "-h   - Show this help message"
    echo
}

unknown_command() {
   echo -e "${COLOR_RED}Unknown command (forgot to set a flag?)${COLOR_RESET}"
   echo -e "Type '$(basename "$0") -h' to show help."
   return 1
}

main() {
    local exit_code=0

    case "$1" in
        -c)
            clean_project
            exit_code=$?
            ;;
        -dbg)
            build_debug
            exit_code=$?
            ;;
        -rel)
            build_release
            exit_code=$?
            ;;
        -all)
            build_all
            exit_code=$?
            ;;
        -h|--help)
            show_help
            ;;
        *)
            unknown_command
            exit_code=$?
            ;;
    esac

    return $exit_code
}

main "$@"
exit $?