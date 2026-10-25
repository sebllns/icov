message("BUILD NETWORK LIB PROJECT")

# library
set( LIB_NAME IcovNetworkLib )

# get source files
file(GLOB_RECURSE SRC_FILES "*.cpp" )

# get include files
file(GLOB_RECURSE INC_FILES "*.h" )

# library
add_library(${LIB_NAME} ${SRC_FILES} ${INC_FILES})
target_include_directories(${LIB_NAME} PUBLIC .)
target_link_libraries(${LIB_NAME} IcovCoreLib)
set_target_properties(${LIB_NAME} PROPERTIES FOLDER "protected/lib" )
