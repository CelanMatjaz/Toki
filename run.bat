@echo off
set config=release_windows
set compiler=clang

premake5 export-compile-commands --cc=clang --os=windows
cp build/compile_commands/%config%.json build/compile_commands.json
premake5 gmake2 --cc=%compiler% --os=windows
make config=%config%
REM make -j%NUMBER_OF_PROCESSORS% config=%config%
