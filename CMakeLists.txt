cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(icov_ADT LANGUAGES CXX C)

message("BUILD ICOV PROJECT")

# ##############################################################################
# PARAMETERS              #
# ##############################################################################
option(ICOV_BUILD_UNITY_PLUGINS "ENABLE UNITY PLUGINS BUILD" OFF)
option(ICOV_RESTART_UNITY "ENABLE UNITY RESTART AFTER INSTALLING UNITY PLUGINS"
       OFF)

set(ICOV_PLATFORM
    "LINUX"
    CACHE STRING "LINUX|ANDROID|MAC|WIN")
set(ICOV_UNITY_COMMAND
    "Unity"
    CACHE STRING "Unity command line")

# ##############################################################################
# SETUP PROJECT            #
# ##############################################################################
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
include(ExternalProject)
include(${CMAKE_SOURCE_DIR}/.cmake/AddGitSubmodule.cmake)

# use folders in IDEs for projects (e.g. lib sample app test)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)
link_directories(${LIBRARY_OUTPUT_PATH} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

set(C_FILES "")
add_compile_options(-DGL_SILENCE_DEPRECATION)

# ##############################################################################
# OS SPECIFIC             #
# ##############################################################################
if(ANDROID)
  message(STATUS "Configured for ANDROID")
  set(ICOV_PLATFORM
      "ANDROID"
      CACHE STRING "" FORCE)
elseif(APPLE OR OSX)
  message(STATUS "Configured for APPLE")
  set(ICOV_PLATFORM
      "MAC"
      CACHE STRING "" FORCE)
elseif(WIN32 OR WINDOWS)
  message(STATUS "Configured for WINDOWS")
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
  set(ICOV_PLATFORM
      "WIN"
      CACHE STRING "" FORCE)
elseif(UNIX)
  message(STATUS "Configured for UNIX")
  set(ICOV_PLATFORM
      "LINUX"
      CACHE STRING "" FORCE)
else()
  message(STATUS "Configured for UNKNOWN")
endif()

# Select platform if(ICOV_PLATFORM STREQUAL "ANDROID") message(STATUS "[INFO]
# Building for Android") #find_package(OpenGL) #find_package(Vulkan)
# #SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++" )
# add_compile_options(-DSUPPORT_OPENGL_ES=1 -DSUPPORT_VULKAN=0 -DUNITY_ANDROID=1
# -D__ANDROID__=1 -DANDROID_STL=c++_shared) #link_libraries("-stdlib=libc++")
# elseif(ICOV_PLATFORM STREQUAL "LINUX") message(STATUS "[INFO] Building for
# Linux") find_package(OpenGL REQUIRED) find_package(Vulkan REQUIRED)
# add_compile_options(-DSUPPORT_OPENGL_UNIFIED=1 -DSUPPORT_VULKAN=1
# -DUNITY_LINUX=1) add_compile_options("-Wno-switch") elseif(ICOV_PLATFORM
# STREQUAL "WIN") message(STATUS "[INFO] Building for Windows")
# find_package(OpenGL REQUIRED) add_compile_options(-DUNITY_WIN -DGLEW_STATIC)
# set(C_FILES "*.c") elseif(ICOV_PLATFORM STREQUAL "MAC") message(STATUS "[INFO]
# Building for Mac") SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17
# -stdlib=libc++" ) find_package(OpenGL REQUIRED OpenGL)
# add_compile_options(-DSUPPORT_VULKAN=0 -DUNITY_OSX=1)
# #link_libraries("-stdlib=libc++") endif()

# ##############################################################################
# ICOV                  #
# ##############################################################################
add_git_submodule(${CMAKE_SOURCE_DIR}/core)
add_subdirectory(core)
include_directories(SYSTEM ${Boost_INCLUDE_DIR})
include_directories(. core core/lib core/lib/icov)

# add_subdirectory(network) add_subdirectory(renderer)

if(ICOV_BUILD_UNITY_PLUGINS)
  add_subdirectory(unity)
endif()

# if(ICOV_BUILD_TESTS) message(STATUS "[INFO] Building ICOV private tests")
# add_subdirectory(tests/icov_mem_test) add_subdirectory(tests/icov_test_parser)
# add_subdirectory(tests/varodayan_test) endif()

# TODO CHECK install(CODE "MESSAGE(STATUS \"Install ICOV libs\")" CONFIGURATIONS
# Debug) install(TARGETS IcovCoreLib IcovNetworkLib IcovRendererLib
# CONFIGURATIONS Debug LIBRARY DESTINATION ${ICOV_PLATFORM})

if(GIT_PUSH)
  install(SCRIPT ${CMAKE_SOURCE_DIR}/.cmake/git_push.cmake)
endif()

if(ICOV_RESTART_UNITY)
  # set(ENV{UNITY_RESTART_COMMAND} ${ICOV_UNITY_COMMAND}) message("UNITY COMD IS
  # $ENV{UNITY_RESTART_COMMAND}")
  install(CODE "set(ICOV_UNITY_COMMAND \"${ICOV_UNITY_COMMAND}\")")
  install(
    CODE "set(UNITY_PROJECT_PATH \"${CMAKE_SOURCE_DIR}/client_application/UnityProject\")"
  )
  install(SCRIPT ${CMAKE_SOURCE_DIR}/.cmake/RestartUnity.cmake)
endif()

# if ... message("upload test") execute_process(COMMAND scp version.txt
# sbelleno@frs.sourceforge.net:/home/frs/project/icov/icov-demonstrator/version/)
