
add_executable(segfit main.cpp ${SRC_SRC} ${LIB_SRC} ${EXTERN_SRC})
include_directories(${LIB_DIR} ${EXTERN_DIR})
target_compile_features(segfit PRIVATE cxx_range_for)
target_link_libraries(segfit lbfgsb)

# for a bug in g++4.8 reported here: http://stackoverflow.com/questions/19463602/compiling-multithread-code-with-g
if(CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL "4.8")
  SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wl,--no-as-needed" )
  SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wl,--no-as-needed" )
endif()
find_package(Threads)
add_executable(batch main-batch.cpp ${SRC_SRC} ${LIB_SRC} ${EXTERN_SRC})
include_directories(${LIB_DIR} ${EXTERN_DIR})
target_compile_features(batch PRIVATE cxx_range_for)
target_link_libraries(batch lbfgsb) 
target_link_libraries(batch ${CMAKE_THREAD_LIBS_INIT})
