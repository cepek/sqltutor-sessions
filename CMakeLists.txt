cmake_minimum_required(VERSION 3.7)

project (sqltutor-sessions)
find_library(PQXX_LIB pqxx)
set (FILES main.cpp sessions.cpp)
add_executable (${PROJECT_NAME} ${FILES})
target_link_libraries (${PROJECT_NAME} ${PQXX_LIB})
