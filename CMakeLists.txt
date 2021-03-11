cmake_minimum_required(VERSION 3.14)

project(tsp)

include_directories(include)

add_executable(${PROJECT_NAME} src/main.c)

set(CMAKE_C_STANDARD 11)


if (APPLE)

    set(CPLEX_PATH /Applications/CPLEX_Studio201/cplex)
    set(CPLEX_LIB ${CPLEX_PATH}/lib/x86-64_osx/static_pic)

elseif (UNIX AND NOT APPLE)

    set(CPLEX_PATH /opt/ibm/ILOG/CPLEX_Studio201/cplex)
    set(CPLEX_LIB ${CPLEX_PATH}/lib/x86-64_linux/static_pic)

elseif (WIN32)

    set(CPLEX_PATH "C:/Program Files/IBM/ILOG/CPLEX_Studio201/cplex")
    set(CPLEX_LIB ${CPLEX_PATH}/lib/x64_windows_msvc14/stat_mda)
    
endif()

include_directories(${CPLEX_PATH}/include/ilcplex)


add_definitions(-DIL_STD)
set (CPLEX_LINKER_FLAGS -lcplex -lm -lpthread -ldl)

target_link_libraries(${PROJECT_NAME} ${CPLEX_LINKER_FLAGS})
target_link_libraries(${PROJECT_NAME} -L${CPLEX_LIB})


enable_testing()
#add_subdirectory(test)


add_executable(tsp_test test/tsp_test.c)

target_link_libraries(tsp_test ${CPLEX_LINKER_FLAGS})
target_link_libraries(tsp_test -L${CPLEX_LIB})

add_test(NAME no_input_test COMMAND tsp_test)
set_tests_properties(no_input_test PROPERTIES WILL_FAIL TRUE)

add_test(NAME help_input_test COMMAND tsp_test --help)

add_test(NAME version_input_test COMMAND tsp_test --v)

add_test(NAME wrong_input_file_test COMMAND tsp_test -f hello.txt)
set_tests_properties(wrong_input_file_test PROPERTIES WILL_FAIL TRUE)

add_test(NAME shuffled_prop_input_file_test COMMAND tsp_test -f ../test/data/shuffled_prop_att48.tsp -verbose 3)

add_test(NAME fail_input_file_test COMMAND tsp_test -f ../test/data/fail_att48.tsp -verbose 3)
set_tests_properties(fail_input_file_test PROPERTIES WILL_FAIL TRUE)