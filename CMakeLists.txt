cmake_minimum_required(VERSION 2.6)

project(game)

if(CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_CONFIGURATION_TYPES Debug Release)
endif()

file(GLOB_RECURSE game_src "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE game_h "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")
file(GLOB imgui_src "${CMAKE_CURRENT_SOURCE_DIR}/submodules/imgui/*.cpp")
file(GLOB imgui_h "${CMAKE_CURRENT_SOURCE_DIR}/submodules/imgui/*.h")
message("${game_src}")

add_executable(game WIN32 ${game_src} ${game_h} ${imgui_src} ${imgui_h})

target_include_directories(game PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(game PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/submodules/glm)
target_include_directories(game PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/submodules/imgui)
target_include_directories(game PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/submodules/stb)
target_include_directories(game PRIVATE "$ENV{VULKAN_SDK}/Include")
target_include_directories(game PRIVATE "$ENV{FBX_SDK}/include")
target_include_directories(game PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/external/dxc/inc)
target_include_directories(game PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/include)


target_link_directories(game PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/lib-vc2015)
target_link_directories(game PRIVATE "$ENV{VULKAN_SDK}/Lib")
target_link_directories(game PRIVATE "$ENV{FBX_SDK}/lib/vs2017/x64/debug")
target_link_directories(game PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/external/dxc/lib/x64)

target_link_libraries(game vulkan-1 libfbxsdk glfw3 dxcompiler)

target_compile_definitions(game PRIVATE GLFW_INCLUDE_VULKAN GLFW_EXPOSE_NATIVE_WIN32)

set_target_properties(game
  PROPERTIES
  OUTPUT_NAME_DEBUG game_d
  RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/bin"
  OUTPUT_NAME_RELEASE game_r
  RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/bin"
  CXX_STANDARD 17
)

add_custom_command(TARGET game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$ENV{FBX_SDK}/lib/vs2017/x64/debug/libfbxsdk.dll"
        "${CMAKE_CURRENT_SOURCE_DIR}/bin")
