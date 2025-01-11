@echo off
set config=release_windows
set compiler=clang

premake5 export-compile-commands --cc=%compiler% --os=windows
cp compile_commands/%config%.json compile_commands.json
premake5 gmake2 --cc=%compiler% --os=windows
make config=%config%
