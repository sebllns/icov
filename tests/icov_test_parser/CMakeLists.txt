message("BUILD PARSER TEST PROJECT")

set(EXECUTABLE_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH}/parser_tests)


find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

if(UNIX)
add_compile_options("-Wno-write-strings")
endif()


# get source files
file(GLOB COMMON_SRC_FILES "*.cpp")

# get include files
file(GLOB COMMON_INC_FILES "*.h")


include_directories(. ${GLUT_INCLUDE_DIR})


file(GLOB_RECURSE ENCODER_SRC_FILES "encoder/*.cpp" )
file(GLOB_RECURSE ENCODER_INC_FILES "encoder/*.h" )
add_executable(test_encoder ${ENCODER_SRC_FILES} ${ENCODER_INC_FILES} ${COMMON_SRC_FILES} ${COMMON_INC_FILES})
target_link_libraries(test_encoder IcovRendererLib IcovNetworkLib IcovCoreLib)

file(GLOB_RECURSE PARSER_SRC_FILES "parser/*.cpp" )
file(GLOB_RECURSE PARSER_INC_FILES "parser/*.h" )
add_executable(test_parser ${PARSER_SRC_FILES} ${PARSER_INC_FILES} ${COMMON_SRC_FILES} ${COMMON_INC_FILES})
target_include_directories(test_parser PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries(test_parser IcovRendererLib IcovNetworkLib IcovCoreLib ${Boost_LIBRARIES})

file(GLOB_RECURSE VIEWER_SRC_FILES "viewer/*.cpp" )
file(GLOB_RECURSE VIEWER_INC_FILES "viewer/*.h" )
add_executable(test_viewer ${VIEWER_SRC_FILES} ${VIEWER_INC_FILES} ${COMMON_SRC_FILES} ${COMMON_INC_FILES})
target_link_libraries(test_viewer IcovRendererLib IcovNetworkLib IcovCoreLib ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})

#message("####### ${GLUT_LIBRARY}")



# set the folder where to place the projects
set_target_properties(test_encoder PROPERTIES FOLDER "protected/tests" )
set_target_properties(test_parser PROPERTIES FOLDER "protected/tests" )
set_target_properties(test_viewer PROPERTIES FOLDER "protected/tests" )