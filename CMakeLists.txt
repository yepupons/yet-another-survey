cmake_minimum_required(VERSION 3.16)

project(yet-another-survey LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# General Settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(EMSCRIPTEN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -gsource-map")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -gsource-map --source-map-base http://localhost:8054/")
endif()

# ccache
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_C_COMPILER_LAUNCHER   ${CCACHE_PROGRAM})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
endif()

if(EMSCRIPTEN)
    # Dependencies
    find_package(Qt6 REQUIRED COMPONENTS Widgets)
    find_package(Qt6 REQUIRED COMPONENTS Widgets Network)
    find_package(Qt6 REQUIRED COMPONENTS Widgets Svg)

    set(NLOHMANN_JSON_LOCAL_DIR "${PROJECT_SOURCE_DIR}/third_party/nlohmann_json")
    if (EXISTS "${NLOHMANN_JSON_LOCAL_DIR}/CMakeLists.txt")
        add_subdirectory("${NLOHMANN_JSON_LOCAL_DIR}" third_party/nlohmann_json)
    else()
        message(FATAL_ERROR "nlohmann_json submodule missing. Run: git submodule update --init --recursive")
    endif()

    set(QRCODEGENERATOR_LOCAL_DIR "${PROJECT_SOURCE_DIR}/third_party/Qt-QrCodeGenerator")
    if (EXISTS "${QRCODEGENERATOR_LOCAL_DIR}/src/QrCodeGenerator.cpp")
        add_library(qrcodegenerator
            "${QRCODEGENERATOR_LOCAL_DIR}/src/QrCodeGenerator.cpp"
            "${QRCODEGENERATOR_LOCAL_DIR}/src/qrcodegen/qrcodegen.cpp"
        )

        target_include_directories(qrcodegenerator PUBLIC
            "${QRCODEGENERATOR_LOCAL_DIR}/src"
        )

        target_link_libraries(qrcodegenerator PUBLIC
            Qt6::Widgets
            Qt6::Svg
        )
    else()
        message(FATAL_ERROR "Qt-QrCodeGenerator submodule missing. Run: git submodule update --init --recursive")
    endif()

    # Client
    qt_add_executable(client
        # Source
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
        client/src/login_window.cpp

        client/src/session.cpp
        client/src/server_interaction.cpp

        client/src/view_passed_surveys.cpp
        client/src/test_results.cpp
        client/src/created_surveys_window.cpp

        # Headers
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
        client/include/login_window.hpp
        client/include/abstract_block_editor.hpp

        client/include/server_interaction.hpp

        client/include/view_passed_surveys.hpp
        client/include/test_results.hpp
        client/include/created_surveys_window.hpp

        # Resources
        client/resources/resources.qrc
    )

    target_include_directories(client PRIVATE
        ${PROJECT_SOURCE_DIR}/client/include/
    )

    target_link_libraries(client PRIVATE
        Qt6::Widgets
        Qt6::Network
        nlohmann_json::nlohmann_json
        qrcodegenerator
    )

    target_link_options(client PRIVATE "--bind")
    set(CMAKE_AUTORCC_OPTIONS "--no-compress")

    add_custom_target(serve-wasm
        COMMAND python3 -m http.server 8054
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
else()
    # Dependencies
    find_package(Drogon CONFIG REQUIRED)
    find_package(mongocxx CONFIG REQUIRED)
    find_package(bsoncxx CONFIG REQUIRED)
    find_package(Boost 1.65.1 REQUIRED)
    find_package(CURL REQUIRED)
    find_package(OpenSSL REQUIRED)
    find_package(Qt6 REQUIRED COMPONENTS Widgets Svg)

    # Drogon Options
    option(USE_POSTGRESQL "Enable PostgreSQL" OFF)
    option(USE_SQLITE3    "Enable SQLite3"    OFF)
    option(USE_MYSQL      "Enable MySQL"      OFF)

    set(NLOHMANN_JSON_LOCAL_DIR "${PROJECT_SOURCE_DIR}/third_party/nlohmann_json")
    if (EXISTS "${NLOHMANN_JSON_LOCAL_DIR}/CMakeLists.txt")
        add_subdirectory("${NLOHMANN_JSON_LOCAL_DIR}" third_party/nlohmann_json)
    else()
        message(FATAL_ERROR "nlohmann_json submodule missing. Run: git submodule update --init --recursive")
    endif()

    set(MATPLOTPLUSPLUS_LOCAL_DIR "${PROJECT_SOURCE_DIR}/third_party/matplotplusplus")
    if (EXISTS "${MATPLOTPLUSPLUS_LOCAL_DIR}/CMakeLists.txt")
        add_subdirectory("${MATPLOTPLUSPLUS_LOCAL_DIR}" third_party/matplotplusplus)
    else()
        message(FATAL_ERROR "matplotplusplus submodule missing. Run: git submodule update --init --recursive")
    endif()

    set(QRCODEGENERATOR_LOCAL_DIR "${PROJECT_SOURCE_DIR}/third_party/Qt-QrCodeGenerator")
    if (EXISTS "${QRCODEGENERATOR_LOCAL_DIR}/src/QrCodeGenerator.cpp")
        add_library(qrcodegenerator
            "${QRCODEGENERATOR_LOCAL_DIR}/src/QrCodeGenerator.cpp"
            "${QRCODEGENERATOR_LOCAL_DIR}/src/qrcodegen/qrcodegen.cpp"
        )

        target_include_directories(qrcodegenerator PUBLIC
            "${QRCODEGENERATOR_LOCAL_DIR}/src"
        )

        target_link_libraries(qrcodegenerator PUBLIC
            Qt6::Widgets
            Qt6::Svg
        )
    else()
        message(FATAL_ERROR "Qt-QrCodeGenerator submodule missing. Run: git submodule update --init --recursive")
    endif()

    set(TGBOT_LOCAL_DIR "${PROJECT_SOURCE_DIR}/third_party/tgbot-cpp")
    if (EXISTS "${TGBOT_LOCAL_DIR}/CMakeLists.txt")
        set(ENABLE_TESTS OFF CACHE BOOL "" FORCE)
        set(BUILD_DOCUMENTATION OFF CACHE BOOL "" FORCE)
        add_subdirectory("${TGBOT_LOCAL_DIR}" third_party/tgbot-cpp)
    else()
        message(FATAL_ERROR "tgbot-cpp submodule missing. Run: git submodule update --init --recursive")
    endif()

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
        client/src/quiz_choice_block_editor.cpp
        client/src/login_window.cpp

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
        client/include/quiz_choice_block_editor.hpp
        client/include/login_window.hpp

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
        qrcodegenerator
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
        matplot
        OpenSSL::Crypto
    )

    # TG bot
    add_executable(telegram_bot
        server/telegram_bot/main.cpp
    )

    target_link_libraries(telegram_bot PRIVATE
        TgBot
        CURL::libcurl
        nlohmann_json::nlohmann_json
    )

    if (TARGET Boost::headers)
        target_link_libraries(telegram_bot PRIVATE Boost::headers)
    elseif (TARGET Boost::boost)
        target_link_libraries(telegram_bot PRIVATE Boost::boost)
    else()
        target_include_directories(telegram_bot PRIVATE ${Boost_INCLUDE_DIRS})
    endif()

    # Debug (enabling reading errors and recieved data on server)
    option(YAZ_DEBUG OFF)
    if (YAZ_DEBUG)
        add_compile_definitions(YAZ_DEBUG)
    endif()
endif()
