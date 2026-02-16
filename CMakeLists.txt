cmake_minimum_required(VERSION 3.16)

project(yet-another-survey CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include)

add_executable(app src/main.cpp src/multiple-question.cpp src/single_choice_question.cpp src/text_question.cpp)
