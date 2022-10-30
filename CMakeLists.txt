cmake_minimum_required(VERSION 3.23)
project(ss_profile)

set(CMAKE_CXX_STANDARD 17)

add_executable(ss_profile main.cpp document.cpp document.h paginator.h read_input_functions.cpp read_input_functions.h request_queue.cpp request_queue.h search_server.cpp search_server.h string_processing.cpp string_processing.h log_duration.h remove_duplicates.h remove_duplicates.cpp test_example_functions.h test_example_functions.cpp)
