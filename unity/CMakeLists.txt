cmake_minimum_required(VERSION 3.2)

# Select platform
if(ICOV_PLATFORM STREQUAL "ANDROID")
  message(STATUS "[INFO] Building for Android")
  # find_package(OpenGL) find_package(Vulkan) SET(CMAKE_CXX_FLAGS
  # "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++" )
  add_compile_options(
    -DSUPPORT_OPENGL_ES=1 -DSUPPORT_VULKAN=0 -DUNITY_ANDROID=1 -D__ANDROID__=1
    -DANDROID_STL=c++_shared)
  # link_libraries("-stdlib=libc++")
elseif(ICOV_PLATFORM STREQUAL "LINUX")
  message(STATUS "[INFO] Building for Linux")
  find_package(OpenGL REQUIRED)
  find_package(Vulkan REQUIRED)
  add_compile_options(-DSUPPORT_OPENGL_UNIFIED=1 -DSUPPORT_VULKAN=1
                      -DUNITY_LINUX=1)
  add_compile_options("-Wno-switch")
elseif(ICOV_PLATFORM STREQUAL "WIN")
  message(STATUS "[INFO] Building for Windows")
  find_package(OpenGL REQUIRED)
  add_compile_options(-DUNITY_WIN -DGLEW_STATIC)
  set(C_FILES "*.c")
elseif(ICOV_PLATFORM STREQUAL "MAC")
  message(STATUS "[INFO] Building for Mac")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -stdlib=libc++")
  find_package(OpenGL REQUIRED OpenGL)
  add_compile_options(-DSUPPORT_VULKAN=0 -DUNITY_OSX=1)
  # link_libraries("-stdlib=libc++")
endif()

unset(missingComponents)
message(STATUS "[INFO] Build Unity plugins")
message(CHECK_START "Finding dependencies")
list(APPEND CMAKE_MESSAGE_INDENT "  ")

message(CHECK_START "Finding gl3w")

set(gl3w_PREFIX "${CMAKE_BINARY_DIR}/third-party/gl3w")
set(gl3w_INSTALL_DIR "${gl3w_PREFIX}")
set(gl3w_CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${gl3w_INSTALL_DIR})

message("gl3w_PREFIX='${gl3w_PREFIX}'")
message("gl3w_INSTALL_DIR='${gl3w_INSTALL_DIR}'")
message("gl3w_CMAKE_ARGS='${gl3w_CMAKE_ARGS}'")
ExternalProject_Add(
  gl3w
  PREFIX ${gl3w_PREFIX}
  GIT_REPOSITORY https://github.com/skaslev/gl3w.git
  INSTALL_DIR ${gl3w_INSTALL_DIR}
  CMAKE_ARGS ${gl3w_CMAKE_ARGS})

message(CHECK_PASS "found")

# message(CHECK_START "Finding partB") # ... do check, assume we don't find B
# list(APPEND missingComponents B) message(CHECK_FAIL "not found")

list(POP_BACK CMAKE_MESSAGE_INDENT)

if(missingComponents)
  message(CHECK_FAIL
          "Missing components required for Unity plugins: ${missingComponents}")
else()
  message(CHECK_PASS "all components found")
endif()
add_subdirectory(icov_demonstrator/icov_unity_plugin)
add_subdirectory(vlc/vlc_unity_plugin)

if(ICOV_RESTART_UNITY)
  # set(ENV{UNITY_RESTART_COMMAND} ${ICOV_UNITY_COMMAND}) message("UNITY COMD IS
  # $ENV{UNITY_RESTART_COMMAND}")
  install(CODE "set(ICOV_UNITY_COMMAND \"${ICOV_UNITY_COMMAND}\")")
  install(
    CODE "set(UNITY_PROJECT_PATH \"${CMAKE_SOURCE_DIR}/client_application/UnityProject\")"
  )
  install(SCRIPT ${CMAKE_SOURCE_DIR}/.cmake/RestartUnity.cmake)
endif()
