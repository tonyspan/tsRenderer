# TsVKRenderer [![License: GPL v3](https://img.shields.io/badge/License-GPLv3-green.svg)](https://github.com/tonyspan/tsRenderer/blob/master/LICENSE)

This project is my implementation/abstraction of a Vulkan renderer. Everything is subject to change as I learn more about Vulkan API.

## Some sources
* https://vulkan-tutorial.com/
* https://vkguide.dev/
* https://github.com/SaschaWillems/Vulkan
* https://github.com/David-DiGioia/vulkan-diagrams
* https://gpuopen.com/learn/understanding-vulkan-objects/
* Various videos, blogs and repos

## Dependencies
* ~[`SDL2`](https://www.libsdl.org/)~ [`GLFW`](https://www.glfw.org/)
* [`glm`](https://github.com/g-truc/glm)
* [`imgui`](https://github.com/ocornut/imgui)
* [`stb`](https://github.com/nothings/stb)
* [`tinyobjloader`](https://github.com/tinyobjloader/tinyobjloader)
* [`volk`](https://github.com/zeux/volk)
* [`Vulkan-Headers`](https://github.com/KhronosGroup/Vulkan-Headers)
* [`Glslang`](https://github.com/KhronosGroup/glslang) using the [`release binaries`](https://github.com/KhronosGroup/glslang/releases/tag/main-tot)
* [`SPIRV-Reflect`](https://github.com/KhronosGroup/SPIRV-Reflect)
* [`Vulkan-Utility-Libraries`](https://github.com/KhronosGroup/Vulkan-Utility-Libraries) for `vk_enum_string_helper.h`

# Prerequisites
* Python 3 (Required)

## Getting Started
* Clone recursively this repository.
* Run `WinGenProject.bat` file found in `scripts` folder.
* Launch the `.sln`.

## Notes
* Currently works/tested ONLY on Windows.