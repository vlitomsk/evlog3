project(evlog3)
cmake_minimum_required(VERSION 3.1)
set (CMAKE_CXX_STANDARD 11)
add_executable(${PROJECT_NAME}
main.cpp
restrictions.hpp # *.hpp is for QtCreator's project tree.
consumer.hpp
entry.cpp entry.hpp
entryreader.cpp entryreader.hpp
entrywriter.cpp entrywriter.hpp
orderedentrysource.cpp orderedentrysource.hpp
)

