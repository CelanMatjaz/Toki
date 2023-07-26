@echo off

set /P "=This script will run Deno with these flags: --allow-read --allow-net --allow-env --allow-write --allow-run, press any button to continue..."

deno run --allow-read --allow-net --allow-env --allow-write --allow-run ../scripts/generate_projects.ts

pause