cmake_minimum_required(VERSION 3.10)
project(FurAuction)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Требуемая структура (bin/build/includes/src)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

# Найти SQLite3
find_package(SQLite3 REQUIRED)

# Подключить директории заголовков
include_directories(includes ${SQLITE3_INCLUDE_DIRS})

# Исходные файлы (все .cpp)
set(SOURCES
    src/main.cpp
    src/Auth.cpp
    src/Database.cpp
    src/Farm.cpp
    src/Lot.cpp
    src/Auction.cpp
)

# Создание исполняемого файла
add_executable(auction_app ${SOURCES})

# Линковка с SQLite3
if (TARGET SQLite::SQLite3)
    target_link_libraries(auction_app SQLite::SQLite3)
else()
    target_link_libraries(auction_app ${SQLITE3_LIBRARIES})
endif()

# Опционально: настройка тестов (если есть папка tests)
enable_testing()
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
    add_subdirectory(tests)
endif()