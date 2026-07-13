add_library(gtest_all STATIC
    ${CMAKE_CURRENT_LIST_DIR}/googletest/googletest/src/gtest_main.cc
    ${CMAKE_CURRENT_LIST_DIR}/googletest/googletest/src/gtest-all.cc
    ${CMAKE_CURRENT_LIST_DIR}/googletest/googlemock/src/gmock-all.cc
)

target_include_directories(gtest_all PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/googletest/googletest
    ${CMAKE_CURRENT_LIST_DIR}/googletest/googletest/include
    ${CMAKE_CURRENT_LIST_DIR}/googletest/googlemock
    ${CMAKE_CURRENT_LIST_DIR}/googletest/googlemock/include
)

target_include_directories(gtest_all SYSTEM INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/googletest/googletest/include
    ${CMAKE_CURRENT_LIST_DIR}/googletest/googlemock/include
)
