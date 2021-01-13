cmake_minimum_required(VERSION 3.16)

############################ project setup ############################

project(lc3-vm LANGUAGES CXX)
include(cmake/standard_project_settings.cmake)
include(cmake/prevent_in_source_builds.cmake)


############################ Options ############################

# Link this 'library' to set the c++ standard / compile-time options requested
add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_20)
target_include_directories(project_options 
  INTERFACE ${CMAKE_SOURCE_DIR}/external
            ${CMAKE_SOURCE_DIR}/src

)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


############################ warnings, sanitizers ############################

# Link this 'library' to use the warnings specified in compiler_warnings.cmake
add_library(project_warnings INTERFACE)

# standard compiler warnings
include(cmake/compiler_warnings.cmake)
set_project_warnings(project_warnings)

# sanitizer options if supported by compiler
include(cmake/sanitizers.cmake)
enable_sanitizers(project_options)

# static analyzers
include(cmake/static_analyzers.cmake)


############################ extra libraries ############################

# add installed libraries
find_package(fmt REQUIRED)


############################ project files ############################

set(sources src/main.cpp 
            src/vm.cpp
            src/utils.cpp
)

add_executable(vm ${sources})

target_link_libraries(
  vm
  PRIVATE project_warnings
          project_options
          fmt::fmt-header-only
)
