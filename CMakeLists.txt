cmake_minimum_required(VERSION 3.16)

project(yet-another-survey CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})  # Should be before add_executable
# target_include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_executable(app main.cpp multiple-question.cpp single_choice_question.cpp text_question.cpp) 