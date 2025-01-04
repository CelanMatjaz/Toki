CONFIG=release_linux_wayland
COMPILER=clang

premake5 export-compile-commands --cc="$COMPILER" --os=linux
cp "compile_commands/$CONFIG.json" compile_commands.json
premake5 gmake2 "--cc=$COMPILER" --os=linux
make "config=$CONFIG"
