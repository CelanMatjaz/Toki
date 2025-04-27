#!/bin/bash
set -e

clear
pushd build
cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build . -j$(nproc)
popd

# # One of: debug/release/dist 
# OPTIMIZE=Debug
# # One of linux/linux/windows
# PLATFORM=Linux
#
# CONFIG="${OPTIMIZE}_${PLATFORM}"
# CONFIG_LOWER_CASE=$(echo "$CONFIG" | awk '{print tolower($0)}')
# PLATFORM_LOWER_CASE=$(echo "$PLATFORM" | awk '{print tolower($0)}')
# COMPILER=gcc
#
# SCRIPT_DIR="$(dirname "$(realpath "$0")")"
# cd $SCRIPT_DIR
#
# premake5 export-compile-commands --cc="$COMPILER" --os=linux > /dev/null
# cp "build/compile_commands/$CONFIG_LOWER_CASE.json" build/compile_commands.json > /dev/null
# premake5 gmake2 "--cc=$COMPILER" "--os=$PLATFORM_LOWER_CASE"
# pushd build > /dev/null
# # make Sandbox -j$(($(nproc) - 1)) "config=$CONFIG_LOWER_CASE"
# make Sandbox "config=$CONFIG_LOWER_CASE"
# popd > /dev/null
