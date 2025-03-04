@echo off
set config=release_windows
set compiler=gcc

premake5 export-compile-commands --cc=clang --os=windows
cp compile_commands/%config%.json compile_commands.json
premake5 gmake2 --cc=%compiler% --os=windows
make -j%NUMBER_OF_PROCESSORS% config=%config%
