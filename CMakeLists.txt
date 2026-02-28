cmake_minimum_required(VERSION 3.16)

project(yet-another-survey LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# General Settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ccache
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_C_COMPILER_LAUNCHER   ${CCACHE_PROGRAM})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
endif()

# Dependencies
find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Drogon CONFIG REQUIRED)
find_package(CURL REQUIRED)

# Drogon Options
option(USE_POSTGRESQL "Enable PostgreSQL" OFF)
option(USE_SQLITE3    "Enable SQLite3"    OFF)
option(USE_MYSQL      "Enable MySQL"      OFF)

# Client
add_executable(client
    client/src/main.cpp
    client/src/main_window.cpp
    client/src/survey_window.cpp
    client/src/multiple_choice_question.cpp
    client/src/single_choice_question.cpp
    client/src/text_question.cpp
    # Because of Qt issues
    client/include/main_window.hpp
    client/include/survey_window.hpp
    client/include/multiple_choice_question.hpp
    client/include/single_choice_question.hpp
    client/include/text_question.hpp
    client/include/abstract_question.hpp
)

target_include_directories(client PRIVATE
    ${PROJECT_SOURCE_DIR}/client/include/
)

target_link_libraries(client PRIVATE
    Qt6::Widgets
    CURL::libcurl
)

# Server
add_executable(server
    server/src/main.cpp
)

target_include_directories(server PRIVATE
    ${PROJECT_SOURCE_DIR}/server/include/
)

target_link_libraries(server PRIVATE
    Drogon::Drogon
)
