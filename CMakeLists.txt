cmake_minimum_required(VERSION 3.14)

project(tsp)

include_directories(include)

file(GLOB tsp_SRC CONFIGURE_DEPENDS "src/*.c" )

add_executable(${PROJECT_NAME} ${tsp_SRC})

add_definitions(-DVERSION="0.9.5") #Software's current version
add_definitions(-DDEBUG) #Whether debug version is needed

set(CMAKE_C_STANDARD 11)

set(CONCORDE_PATH ${PROJECT_SOURCE_DIR}/libs/concorde)

if (APPLE)

    set(CPLEX_PATH /Applications/CPLEX_Studio201/cplex)
    set(CPLEX_LIB ${CPLEX_PATH}/lib/x86-64_osx/static_pic)

    set(CONCORDE_LIB ${CONCORDE_PATH}/concorde.a)

elseif (UNIX AND NOT APPLE)

    set(CPLEX_PATH /opt/ibm/ILOG/CPLEX_Studio201/cplex)
    set(CPLEX_LIB ${CPLEX_PATH}/lib/x86-64_linux/static_pic)

    set(CONCORDE_LIB ${CONCORDE_PATH}/concorde.a)

elseif (WIN32)

    set(CPLEX_PATH "C:/Program Files/IBM/ILOG/CPLEX_Studio201/cplex")
    set(CPLEX_LIB ${CPLEX_PATH}/lib/x64_windows_msvc14/stat_mda)

    set(CONCORDE_LIB ${CONCORDE_PATH}/concorde.lib)
    
endif()

include_directories(${CPLEX_PATH}/include/ilcplex)
include_directories(${CONCORDE_PATH} ${CONCORDE_PATH}/INCLUDE)


add_definitions(-DIL_STD)
set (CPLEX_LINKER_FLAGS -lcplex -lm -lpthread -ldl)

target_link_libraries(${PROJECT_NAME} ${CPLEX_LINKER_FLAGS})
target_link_libraries(${PROJECT_NAME} -L${CPLEX_LIB})
target_link_libraries(${PROJECT_NAME} ${CONCORDE_LIB})
