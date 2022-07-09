include(CMake/get_repository.cmake)
include(Cmake/create_plugin.cmake)
get_repository(JUCE https://github.com/juce-framework/JUCE.git 545e9f353a6a336c5d1616796024b30d4bbed04b)
add_subdirectory(${juce_SOURCE_DIR})