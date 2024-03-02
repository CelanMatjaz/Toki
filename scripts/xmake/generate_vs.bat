@echo off

xmake f --vs=2022
xmake project -k vsxmake -m "debug;release" 