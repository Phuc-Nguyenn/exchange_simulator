cmake_minimum_required(VERSION 3.10)
project(ExchangeSimulator)
set(CMAKE_CXX_STANDARD 14)

add_executable(exchange_server 
    src/Exchange.cpp
    src/OrderBook.cpp
    src/main.cpp
)

add_executable(unit_tests 
    UnitTest/UtilsTest.cpp
    UnitTest/OrderBookTest.cpp
    src/OrderBook.cpp  
)

# Include GTest and GMock
include_directories(
    ${CMAKE_SOURCE_DIR}/3rdParty/googletest/googletest/include
    ${CMAKE_SOURCE_DIR}/include
)

# Link with GTest and GMock libraries
link_directories(
    ${CMAKE_SOURCE_DIR}/3rdParty/googletest/build/lib
)

target_link_libraries(exchange_server gtest pthread)
target_link_libraries(unit_tests 
    ${CMAKE_SOURCE_DIR}/3rdParty/googletest/build/lib/libgtest.a
    ${CMAKE_SOURCE_DIR}/3rdParty/googletest/build/lib/libgtest_main.a 
    pthread
)