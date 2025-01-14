# Toki

A rendering framework/engine, writen in C++ using the Vulkan rendering API. Project is a work in progress library that I later plan to use to make games and visualizations.

<p align="center">
  <img src="images/screenshot.jpg" alt="Sublime's custom image" width=400/>
</p>

## Technologies

- C++23
- [Vulkan](https://www.vulkan.org/) 1.3.296.0
- [GLFW](https://github.com/glfw/glfw) 3.4
- [GLM](https://github.com/g-truc/glm) 1.0.1
- [yaml-cpp](https://github.com/jbeder/yaml-cpp) 0.8.0

## Supported platforms

Building and running tested on:

- **Linux - Wayland** (built on Arch Linux with Clang 18.1.8 and GCC 14.2.1)
- **Windows** (built with Clang 18.1.1 and MSVC 19.40 with Visual Studio 2022)
- ~~**Linux - X11**~~ - Not yet supported

## Project setup

To compile the project, you will need a compiler that supports C++23, preferably one of the above mentioned compilers with the same or more recent version.

### Repository setup

Clone the repo with `git clone --recursive https://github.com/CelanMatjaz/toki.git`.

### Prerequisites

- Download and install [Premake](https://premake.github.io/)
    - Linux: 
      - Install the `premake5` package for your distribution
    - Windows:
      - Download the zip file
      - Extract the file's contents and put the executable into a directory
      - Add the directory to your PATH or somewhere you can easily run the executable
- Download and install [VulkanSDK](https://vulkan.lunarg.com/) for your OS and make sure that the `VULKAN_SDK` environment variable is set and points to your Vulkan installation directory
    - Linux:
      - reference [VulkanSDK Linux help page](https://vulkan.lunarg.com/doc/sdk/1.3.296.0/linux/getting_started.html), search for your distribution and follow steps
      - reference [GLFW Linux compiling page](https://www.glfw.org/docs/latest/compile.html#compile_deps_wayland) for packages that are needed to compile GLFW on your distribution
    - Windows:
      - run the installer and everything should be setup, if you have any problems installing the SDK, then reference [VulkanSDK Windows help page](https://vulkan.lunarg.com/doc/sdk/1.3.296.0/windows/getting_started.html)

### Build file setup

#### Makefile (Windows and Linux)
- Generate makefiles files with Premake: `premake5 gmake2`
- Linux (Wayland) only: run `./scripts/generate_wayland_headers.sh` to generate necessary Wayland headers for GLFW
- Optional: you can run `premake5 export-compile-commands` to generate compile_commands.json files for all configurations in `./compile_commands/` directory


#### Visual Studio (Windows only)
- Generate Visual Studio project files with Premake: `premake5 vs2022`
- Open the `Toki.sln` file with Visual Studio
