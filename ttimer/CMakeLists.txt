add_executable(timer timer.h timer.c os.h tdengine_old.c)

add_executable(tdengine_old tdengine_old.c)
target_link_libraries(tdengine_old pthread)