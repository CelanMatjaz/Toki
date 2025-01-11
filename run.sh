# One of: debug/release/dist 
OPTIMIZE=debug
# One of linux_wayland/linux_x11/windows
PLATFORM=linux_wayland

CONFIG="${OPTIMIZE}_${PLATFORM}"
COMPILER=clang

premake5 export-compile-commands --cc="$COMPILER" --os=linux > /dev/null
cp "compile_commands/$CONFIG.json" compile_commands.json > /dev/null
premake5 gmake2 "--cc=$COMPILER" --os=linux
make "config=$CONFIG" && ./bin/${PLATFORM}-x86_64/${OPTIMIZE}/Sandbox ./toki_sandbox
