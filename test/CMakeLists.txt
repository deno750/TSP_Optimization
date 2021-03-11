add_executable(tsp_test tsp_test.c)
target_link_libraries(tsp_test 
                    ${cplex}
                    ${ilocplex}
                    ${cplexdistmip}
                    ${concert})
add_test(NAME no_input_test COMMAND tsp_test)
set_tests_properties(no_input_test PROPERTIES WILL_FAIL TRUE)

add_test(NAME help_input_test COMMAND tsp_test --help)
set_tests_properties(help_input_test PROPERTIES WILL_FAIL TRUE)

add_test(NAME version_input_test COMMAND tsp_test --v)

add_test(NAME no_input_file_test COMMAND tsp_test -f hello.txt)
set_tests_properties(no_input_file_test PROPERTIES WILL_FAIL TRUE)

add_test(NAME shuffled_prop_input_file_test COMMAND tsp_test -f ../test/data/shuffled_prop_att48.tsp -verbose 3)

add_test(NAME fail_input_file_test COMMAND tsp_test -f ../test/data/fail_att48.tsp -verbose 3)
set_tests_properties(fail_input_file_test PROPERTIES WILL_FAIL TRUE)