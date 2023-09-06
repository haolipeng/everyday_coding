add_executable(thread main.c ../networks/reuse_addr_option.c)
target_link_libraries(thread pthread)