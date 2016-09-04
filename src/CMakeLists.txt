

add_executable(segfit ${SRC_SRC} ${LIB_SRC} ${EXTERN_SRC})
TARGET_LINK_LIBRARIES(segfit lbfgsb)
include_directories(${LIB_DIR})

target_compile_features(segfit PRIVATE cxx_range_for)
