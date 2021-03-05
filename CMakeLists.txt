cmake_minimum_required(VERSION 3.14)

project(tsp)

add_executable(tsp src/main.c)

set(CMAKE_C_STANDARD 11)

if (APPLE)
    include_directories("/Applications/CPLEX_Studio201/cplex/include")
    include_directories("/Applications/CPLEX_Studio201/cplex/include/ilcplex")
    include_directories("/Applications/CPLEX_Studio201/concert/include")
    include_directories("/Applications/CPLEX_Studio201/concert/include/ilconcert")

    find_library(cplex NAMES libcplex.a PATHS "/Applications/CPLEX_Studio201/cplex/lib/x86-64_osx/static_pic")
    find_library(ilocplex NAMES libilocplex.a PATHS "/Applications/CPLEX_Studio201/cplex/lib/x86-64_osx/static_pic")
    find_library(cplexdistmip NAMES libcplexdistmip.a PATHS "/Applications/CPLEX_Studio201/cplex/lib/x86-64_osx/static_pic")
    find_library(concert NAMES libconcert.a PATHS "/Applications/CPLEX_Studio201/concert/lib/x86-64_osx/static_pic")
endif()

if (UNIX AND NOT APPLE)
    include_directories("/opt/ibm/ILOG/CPLEX_Studio201/cplex/include")
    include_directories("/opt/ibm/ILOG/CPLEX_Studio201/cplex/include/ilcplex")
    include_directories("/opt/ibm/ILOG/CPLEX_Studio201/concert/include")
    include_directories("/opt/ibm/ILOG/CPLEX_Studio201/concert/include/ilconcert")
    find_library(cplex NAMES libcplex.a PATHS "/opt/ibm/ILOG/CPLEX_Studio201/cplex/lib/x86-64_linux/static_pic")
    find_library(ilocplex NAMES libilocplex.a PATHS "/opt/ibm/ILOG/CPLEX_Studio201/cplex/lib/x86-64_linux/static_pic")
    find_library(cplexdistmip NAMES libcplexdistmip.a PATHS "/opt/ibm/ILOG/CPLEX_Studio201/cplex/lib/x86-64_linux/static_pic")
    find_library(concert NAMES libconcert.a PATHS "/opt/ibm/ILOG/CPLEX_Studio201/concert/lib/x86-64_linux/static_pic")
endif()

add_definitions(-DIL_STD)

set (CMAKE_SHARED_LINKER_FLAGS "-lcplex -lm -lpthread")

target_link_libraries(tsp PUBLIC ${cplex})
target_link_libraries(tsp PUBLIC ${ilocplex})
target_link_libraries(tsp PUBLIC ${cplexdistmip})
target_link_libraries(tsp PUBLIC ${concert})
