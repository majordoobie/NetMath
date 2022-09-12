include(BuildUtils)

add_library(server SHARED arg_parser.c server.c)
target_link_libraries(server PUBLIC utils thread_pool)
set_project_properties(server ${PROJECT_SOURCE_DIR}/include)
