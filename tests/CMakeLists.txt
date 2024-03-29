# tests need source header
INCLUDE_DIRECTORIES(../src)
INCLUDE(CTest)

# This is for QtCreator which does not add single header to the project
SET(TEST_HEADER
    hashtable_test.h)

if(UNIX)
    setup_target_for_coverage_gcovr_html(
        NAME coverage
        EXECUTABLE ctest -j 4)
endif()

# Add your test to CMAKE
# to run all tests run 'make test'
MACRO(TEST_MACRO NAME LIBRARIES) # LIBRARIES
    ADD_EXECUTABLE(${NAME}_test ${NAME}_test.cpp ${TEST_HEADER})
    TARGET_LINK_LIBRARIES(${NAME}_test ${LIBRARIES} gtest -pthread)

    # gtest need to be compiled first
    ADD_DEPENDENCIES(${NAME}_test gtest)

    if(UNIX)
        ADD_DEPENDENCIES(coverage ${NAME}_test)
    endif()

    ADD_TEST(
        NAME ${NAME}_test
        WORKING_DIRECTORY .
        COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NAME}_test)
ENDMACRO(TEST_MACRO)

# add test here
# file_name_test.cpp ==> CBTEST_MACRO(file_name)

# User's Libraries
SET(project_libraries easyfs)

TEST_MACRO(easyfs ${project_libraries})
