cmake_minimum_required(VERSION 3.10)

project(VlcUnityWrapper LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

#Include functions
message(VERBOSE "Includes")
include(ExternalProject)

message("BUILD UNITY VLC PROJECT")

# library
set( LIB_NAME VlcUnityWrapper )

#---------------------------------------------------------------------
# Link with the "Threads library (required to link with AFF3CT after)
message(STATUS "[INFO] Checking Threads support...")
set(CMAKE_THREAD_PREFER_PTHREAD ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
option(CMAKE_USE_WIN32_THREADS_INIT "using WIN32 threads" ON)
message("\n")

#find_package(GLEW REQUIRED)
#include_directories(${GLEW_INCLUDE_DIRS})
#link_libraries(${GLEW_LIBRARIES})
#message(${GLEW_INCLUDE_DIRS})

# get source files
file(GLOB_RECURSE SRC_FILES "*.cpp" ${C_FILES})

# get include files
file(GLOB_RECURSE INC_FILES "*.h" )

# library
add_library(${LIB_NAME} SHARED ${SRC_FILES} ${INC_FILES})

set(vlc_PREFIX "${CMAKE_BINARY_DIR}/third-party/libvlc")
set(vlc_INSTALL_DIR "${vlc_PREFIX}")

if(WIN32)

ExternalProject_Add(libvlc
    PREFIX ${vlc_PREFIX}
    GIT_REPOSITORY https://code.videolan.org/videolan/vlc.git
    #https://github.com/videolan/vlc.git
    GIT_TAG f7803af508b977c4680143039da44974d0d423f4
    #origin/release/3.0.17.4
    BUILD_IN_SOURCE TRUE
    INSTALL_DIR ${vlc_INSTALL_DIR}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    )

    add_dependencies(${LIB_NAME} libvlc)
    #target_link_directories(${LIB_NAME} PUBLIC . ${vlc_INSTALL_DIR}/lib)
    target_include_directories(${LIB_NAME} SYSTEM PUBLIC ${vlc_INSTALL_DIR}/src/libvlc/include)
    #target_link_libraries(${LIB_NAME} vlc)

elseif(UNIX)
ExternalProject_Add(libvlc
    PREFIX ${vlc_PREFIX}
    GIT_REPOSITORY https://code.videolan.org/videolan/vlc.git
    #https://github.com/videolan/vlc.git
    GIT_TAG f7803af508b977c4680143039da44974d0d423f4
    #origin/release/3.0.17.4
    BUILD_IN_SOURCE TRUE
    INSTALL_DIR ${vlc_INSTALL_DIR}
    CONFIGURE_COMMAND ./bootstrap
    COMMAND ./configure --config-cache --enable-debug --disable-silent-rules --disable-vlc --without-kde-solid --disable-opencv --disable-qt --disable-lua --prefix=<INSTALL_DIR>
    )

    add_dependencies(${LIB_NAME} libvlc)
    target_link_directories(${LIB_NAME} PUBLIC . ${vlc_INSTALL_DIR}/lib)
    target_include_directories(${LIB_NAME} SYSTEM PUBLIC ${vlc_INSTALL_DIR}/include)
    target_link_libraries(${LIB_NAME} vlc)
endif()


target_include_directories(${LIB_NAME} PUBLIC .)
#third-party/libvlc/src/libvlc-build/lib/.libs

set_source_files_properties( ${SRC_FILES} PROPERTIES LANGUAGE "CXX" )

target_link_libraries(${LIB_NAME} ${OPENGL_gl_LIBRARY} glfw ${OPENGL_LIBRARIES} ${OPENGL_gl_LIBRARY} Threads::Threads)

include_directories(.)

# set the folder where to place the projects
set_target_properties(${LIB_NAME} PROPERTIES FOLDER "protected/unity" )
