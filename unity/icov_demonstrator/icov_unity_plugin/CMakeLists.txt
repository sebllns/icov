message("BUILD ICOV UNITY PLUGIN")

# library
set( LIB_NAME IcovUnityPlugin )


# get source files
file(GLOB_RECURSE SRC_FILES "*.cpp" ${C_FILES})

# get include files
file(GLOB_RECURSE INC_FILES "*.h" )


# library
add_library(${LIB_NAME} SHARED ${SRC_FILES} ${INC_FILES})
add_dependencies(${LIB_NAME} gl3w)

target_compile_options(${LIB_NAME} PRIVATE -DGL_SILENCE_DEPRECATION)
target_include_directories(${LIB_NAME} PUBLIC .)
set_source_files_properties( ${SRC_FILES} PROPERTIES LANGUAGE "CXX" )

install(TARGETS ${LIB_NAME} CONFIGURATIONS Debug LIBRARY DESTINATION ${ICOV_PLATFORM})


# # Select platform
# if(ICOV_PLATFORM STREQUAL "ANDROID")
#     message(STATUS "[INFO] Building Unity plugin for Android")
#     #find_package(OpenGL)
#     #find_package(Vulkan)
#     SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 " )
#     add_compile_options(-DSUPPORT_OPENGL_ES=1 -DSUPPORT_VULKAN=0 -DUNITY_ANDROID=1 -D__ANDROID__=1)
# elseif(ICOV_PLATFORM STREQUAL "LINUX")
#     message(STATUS "[INFO] Building Unity plugin for Linux")
#     find_package(OpenGL REQUIRED)
#     find_package(Vulkan REQUIRED)
#     add_compile_options(-DSUPPORT_OPENGL_UNIFIED=1 -DSUPPORT_VULKAN=1 -DUNITY_LINUX=1)
#     add_compile_options("-Wno-switch")
# elseif(ICOV_PLATFORM STREQUAL "WIN")
#     message(STATUS "[INFO] Building Unity plugin for Windows")
#     find_package(OpenGL REQUIRED)
#     add_compile_options(-DUNITY_WIN -DGLEW_STATIC)
#     set(C_FILES "*.c")
# elseif(ICOV_PLATFORM STREQUAL "MAC")
#     message(STATUS "[INFO] Building Unity plugin for Mac")
#     find_package(OpenGL REQUIRED OpenGL)
#     add_compile_options(-DSUPPORT_VULKAN=0 -DUNITY_OSX=1)
# endif()

if(ICOV_PLATFORM STREQUAL "ANDROID")
    target_link_libraries(${LIB_NAME}  IcovRendererLib IcovNetworkLib IcovCoreLib GLESv2)
elseif(ICOV_PLATFORM STREQUAL "MAC")
    target_link_libraries(${LIB_NAME}  IcovRendererLib IcovNetworkLib IcovCoreLib OpenGL::GL)
else()
    target_link_libraries(${LIB_NAME} IcovRendererLib IcovNetworkLib IcovCoreLib  ${OPENGL_gl_LIBRARY})
endif()

#include_directories(.)

# set the folder where to place the projects
set_target_properties(${LIB_NAME} PROPERTIES FOLDER "protected/unity" )