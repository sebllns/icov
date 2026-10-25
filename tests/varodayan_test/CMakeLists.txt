message("BUILD VARODAYAN TEST PROJECT")

# executable
set( EXE_NAME varodayan_test )

# get source files
file(GLOB SRC_FILES "*.cpp")

# get include files
file(GLOB INC_FILES "*.h")


add_executable(${EXE_NAME} main.cpp ${SRC_FILES} ${INC_FILES})


include_directories(. /usr/local/include/aff3ct-2.3.5/aff3ct)
#include_directories(${CMAKE_CURRENT_BINARY_DIR})


target_link_libraries(${EXE_NAME}  IcovRendererLib IcovNetworkLib IcovCoreLib)

target_compile_options(${EXE_NAME} PRIVATE -w)

# set the folder where to place the projects
set_target_properties( ${EXE_NAME} PROPERTIES FOLDER "protected/tests" )