# Toki

Work in progress rendering engine writen with C++ and the Vulkan rendering API.

![Example](/assets/pic1.png)

## System dependencies

Submodule dependencies:
- GLFW
- GLM
- STB

Only a C++23 compatible compiler is needed, plus some extra window managment with GLFW for Linux.

## Running the engine

Clone the repo with the --recursive flag.

Toki uses [xmake](https://xmake.io/) as it's build system. You can install it [here](https://xmake.io/#/getting_started).

To generate project files for Visual Studio, run `./scripts/xmake/generate_vs.bat` from project root.

You can also generate a `compile_commands.json` file to be used by your LSP with `./scripts/xmake/generate_compile_commands` shell or batch scripts.

To run the program, run `xmake b && xmake r`, this will build and run the project respevtively.