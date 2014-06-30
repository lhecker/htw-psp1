pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

include_directories(${GTK3_INCLUDE_DIRS} ../)
link_directories(${GTK3_LIBRARY_DIRS})
add_definitions(${GTK3_CFLAGS_OTHER})

add_executable(psp1beleg_gtk gbase-list-private.c main.c)
target_link_libraries(psp1beleg_gtk base ${GTK3_LIBRARIES})
