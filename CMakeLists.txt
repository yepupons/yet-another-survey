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
find_package(mongocxx CONFIG REQUIRED)
find_package(bsoncxx CONFIG REQUIRED)

include(FetchContent)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

FetchContent_Declare(
    matplotplusplus
    GIT_REPOSITORY https://github.com/alandefreitas/matplotplusplus.git
    GIT_TAG master
)
FetchContent_MakeAvailable(matplotplusplus)

# Drogon Options
option(USE_POSTGRESQL "Enable PostgreSQL" OFF)
option(USE_SQLITE3    "Enable SQLite3"    OFF)
option(USE_MYSQL      "Enable MySQL"      OFF)

# Client
add_executable(client
    client/src/main.cpp
    client/src/main_window.cpp
    client/src/survey_window.cpp
    client/src/survey_taking.cpp
    client/src/multiple_choice_block.cpp
    client/src/single_choice_block.cpp
    client/src/text_block.cpp

    client/src/survey_builder_window.cpp
    client/src/section_editor.cpp
    client/src/text_block_editor.cpp
    client/src/single_choice_block_editor.cpp
    client/src/multiple_choice_block_editor.cpp

    client/src/session.cpp
    client/src/server_interaction.cpp

    client/src/view_passed_surveys.cpp
    client/src/test_results.cpp
    client/src/created_surveys_window.cpp

    # Because of Qt issues
    client/include/main_window.hpp
    client/include/survey_window.hpp
    client/include/survey_taking.hpp
    client/include/multiple_choice_block.hpp
    client/include/single_choice_block.hpp
    client/include/text_block.hpp
    client/include/abstract_block.hpp

    client/include/survey_builder_window.hpp
    client/include/section_editor.hpp
    client/include/text_block_editor.hpp
    client/include/single_choice_block_editor.hpp
    client/include/multiple_choice_block_editor.hpp
    client/include/abstract_block_editor.hpp

    client/include/view_passed_surveys.hpp
    client/include/test_results.hpp
    client/include/created_surveys_window.hpp
    
    client/resources/resources.qrc
)

target_include_directories(client PRIVATE
    ${PROJECT_SOURCE_DIR}/client/include/
)

target_link_libraries(client PRIVATE
    Qt6::Widgets
    CURL::libcurl
    nlohmann_json::nlohmann_json
    matplot
)

# Server
add_executable(server
    server/src/main.cpp
    server/src/database.cpp
)

target_include_directories(server PRIVATE
    ${PROJECT_SOURCE_DIR}/server/include/
)

target_link_libraries(server PRIVATE
    Drogon::Drogon
    nlohmann_json::nlohmann_json
    mongo::bsoncxx_shared
    mongo::mongocxx_shared
)
# Debug (enabling reading errors and recieved data on server)
option(YAZ_DEBUG OFF)
if (YAZ_DEBUG) 
    add_compile_definitions(YAZ_DEBUG)
endif()
