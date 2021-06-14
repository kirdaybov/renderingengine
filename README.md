# Rendering engine
Experimental Vulkan renderer/engine. At the moment it is capable of rendering a simple model and ImGui interface. Tracks if the shader sources were changed and reloads them on the fly.

# Compiling
At the moment it's not meant to be compiled by someone other than me but it is possible! If you try to run it though it will crash as I haven't hosted the data for it.

## Requirements
cmake to generate a solution

Vulkan SDK installed and VULKAN_SDK environment variable set to the corresponding path

FBX SDK installed and FBX_SDK environment variable set to the corresponding path

Built DirectX compiler and GLFW

## How to build
Clone the repository recursively or init submodules after cloning

Download content (currently not possible)

Download DirectX compiler and GLFW. Place both in the root/experimental (consult CMakeLists.txt to see the exact location expected) 

Execute gen_solution.bat

Open intermediate/cmake/game.sln

Compile!
