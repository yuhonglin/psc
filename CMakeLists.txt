cmake_minimum_required(VERSION 2.6)

set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/)
set(EXTERN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/)

project(segfit CXX Fortran)

AUX_SOURCE_DIRECTORY(${SRC_DIR} LIB_SRC)
AUX_SOURCE_DIRECTORY(${EXTERN_DIR} EXTERN_SRC)
AUX_SOURCE_DIRECTORY(${LIB_DIR} SRC_SRC)

add_subdirectory(src)
add_subdirectory(extern)
