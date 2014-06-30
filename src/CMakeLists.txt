find_package(Threads)

add_library(base list.c base.c)
target_link_libraries(base ${CMAKE_THREAD_LIBS_INIT})
