include(BuildUtils)

add_library(thread_pool SHARED thread_pool.c)
target_link_libraries(thread_pool PUBLIC utils)
set_project_properties(thread_pool ${PROJECT_SOURCE_DIR}/include)
