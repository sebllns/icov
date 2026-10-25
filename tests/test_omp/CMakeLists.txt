cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
project(omp_test LANGUAGES CXX C)

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR})

# use, i.e. don't skip the full RPATH for the build tree
set(CMAKE_SKIP_BUILD_RPATH FALSE)

# when building, don't use the install RPATH already (but later on when
# installing)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

# add the automatically determined parts of the RPATH which point to directories
# outside the build tree to the install RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# the RPATH to be used when installing, but only if it's not a system directory
list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES
     "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
if("${isSystemDir}" STREQUAL "-1")
  set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
endif("${isSystemDir}" STREQUAL "-1")

# sets RPATH for *installed* files set(CMAKE_INSTALL_RPATH "\$ORIGIN/lib") #
# Note CMake escaping aroung '$' sign. *built* files will also used RPATH which
# has been set before set(CMAKE_BUILD_WITH_INSTALL_RPATH on)

message("BUILD omp_test PROJECT")
find_package(OpenMP REQUIRED)
add_executable(omp_test main.cpp)
target_link_libraries(omp_test PUBLIC OpenMP::OpenMP_CXX)

install(TARGETS omp_test DESTINATION bin)
