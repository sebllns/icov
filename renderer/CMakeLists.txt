message("BUILD RENDERER LIB PROJECT")

project(icov_ADT LANGUAGES CXX)

# library
set( LIB_NAME IcovRendererLib )

# get source files
file(GLOB_RECURSE SRC_FILES "*.cpp" )

# get include files
file(GLOB_RECURSE INC_FILES "*.h" )

add_library(${LIB_NAME} ${SRC_FILES} ${INC_FILES})
target_link_libraries(${LIB_NAME} IcovNetworkLib IcovCoreLib IcovOmnidirectionalLayer)
set_target_properties(${LIB_NAME} PROPERTIES FOLDER "protected/lib")

include_directories(.)
