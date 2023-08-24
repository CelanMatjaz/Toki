# Toki

## Project setup

Toki uses [XMake](https://xmake.io/#/) for it's build system. To build/generate projects, you first need to install it. If you want to use VS Code, then the [XMake extention](https://marketplace.visualstudio.com/items?itemName=tboox.xmake-vscode) is also recommended for debugging and configuration setup.

You will also need the [VulkanSDK](https://vulkan.lunarg.com/sdk/home) installed.

It only supports C++23 and up.

### Run with VS Code: 

Run `xmake config -m debug` to set the build config to build debug. Or `xmake config -m release` for release.

### Run with Visual Studio:

Toki comes with `generate_vs.bat` batch script which will generate a `vsxmake2022` folder by default which will contain the solution file. If you want to change Visual Studio version then you need to edit the `--vs=2022` section in the batch script with `--vs=20XX` where `XX` is the version you want to run. The script will in turn instead create a `vsxmake20XX` folder with the provided VS version.