cmake_minimum_required(VERSION 3.17)

set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Universal Build" FORCE)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.14 CACHE STRING "minimum macos target" FORCE)
set(ENV{CC} "clang")
set(ENV{CXX} "clang++")
set(CMAKE_CXX_STANDARD 20)

project(Modelizer VERSION 1.0.0)

include(CMake/get_juce.cmake)
include(CMake/create_plugin.cmake)
include(CMake/add_sources.cmake)

include_directories(Source)

# create_plugin(MyPlugin "My plugin" "Company Name" COMPANY_ID PLUGIN_ID)
create_plugin(${PROJECT_NAME} ${PROJECT_NAME} "NNModels" Nnmd Mdlz 1.0.0)

add_target_source(${PROJECT_NAME})
