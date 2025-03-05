# One of: debug/release/dist 
OPTIMIZE=Debug
# One of linux/linux/windows
PLATFORM=Linux

CONFIG="${OPTIMIZE}_${PLATFORM}"
CONFIG_LOWER_CASE=$(echo "$CONFIG" | awk '{print tolower($0)}')
PLATFORM_LOWER_CASE=$(echo "$PLATFORM" | awk '{print tolower($0)}')
COMPILER=clang

premake5 export-compile-commands --cc="$COMPILER" --os=linux > /dev/null
cp "compile_commands/$CONFIG_LOWER_CASE.json" compile_commands.json > /dev/null
premake5 gmake2 "--cc=$COMPILER" "--os=$PLATFORM_LOWER_CASE"
# make Sandbox -j$(($(nproc) - 1)) "config=$CONFIG_LOWER_CASE"
make Sandbox2 "config=$CONFIG_LOWER_CASE"
