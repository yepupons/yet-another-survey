cmake_minimum_required(VERSION 3.16)

project(yet-another-survey CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# ускоряет сборку
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  set(CMAKE_C_COMPILER_LAUNCHER   ${CCACHE_PROGRAM})
  set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
endif()

include_directories(include)
add_subdirectory(third_party/drogon)
add_subdirectory(third_party/jsoncpp)

add_library(core
    src/multiple-question.cpp
    src/single_choice_question.cpp
    src/text_question.cpp
)
target_include_directories(core PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}
)

target_include_directories(core PUBLIC)

target_link_libraries(core PUBLIC jsoncpp_lib)

# опросник
add_executable(cli
    src/main.cpp
)
target_link_libraries(cli PRIVATE core)

# Drogon)
add_executable(server
    src/server_main.cpp
)
target_link_libraries(server PRIVATE core drogon)
find_package(jsoncpp CONFIG REQUIRED)

find_package(CURL REQUIRED)
target_link_libraries(cli PRIVATE CURL::libcurl)

