
add_executable(segfit main.cpp ${SRC_SRC} ${LIB_SRC} ${EXTERN_SRC})
include_directories(${LIB_DIR} ${EXTERN_DIR})
target_compile_features(segfit PRIVATE cxx_range_for)
target_link_libraries(segfit lbfgsb)


add_executable(batch main-batch.cpp ${SRC_SRC} ${LIB_SRC} ${EXTERN_SRC})
include_directories(${LIB_DIR} ${EXTERN_DIR})
target_compile_features(batch PRIVATE cxx_range_for)
target_link_libraries(batch lbfgsb)
