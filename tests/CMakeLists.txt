cmake_minimum_required(VERSION 3.10)

include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(auction_app_tests
  test_app.cpp
  ../src/Auth.cpp
  ../src/Database.cpp
  ../src/Farm.cpp
  ../src/Lot.cpp
  ../src/Auction.cpp
)

target_include_directories(auction_app_tests PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/../includes"
)

target_link_libraries(auction_app_tests PRIVATE
  gtest_main
)

if (TARGET SQLite::SQLite3)
  target_link_libraries(auction_app_tests PRIVATE SQLite::SQLite3)
else()
  target_link_libraries(auction_app_tests PRIVATE ${SQLITE3_LIBRARIES})
endif()

include(GoogleTest)
gtest_discover_tests(auction_app_tests)