@echo off

@REM xmake project -k vsxmake -m "debug,release" -a "x64" --confirm=CONFIRM -D
xmake f --vs=2022
xmake project -k vsxmake -m "debug;release" 

pause