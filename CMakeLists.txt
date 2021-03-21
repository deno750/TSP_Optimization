cmake_minimum_required(VERSION 3.14)

project(tsp)

include_directories(include)

add_executable(${PROJECT_NAME} 
                src/main.c 
                src/utility.c
                src/solver.c
                src/distutil.c
                src/plot.c
                src/mtz.c
                src/gg.c
                )

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


#enable_testing()
#add_subdirectory(test)
