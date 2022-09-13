include(BuildUtils)

add_library(server_backend SHARED arg_parser.c server_backend.c)
target_link_libraries(server_backend PUBLIC utils thread_pool header_parser)
set_project_properties(server_backend ${PROJECT_SOURCE_DIR}/include)

add_executable(server server.c)
target_link_libraries(server PUBLIC server_backend)
set_project_properties(server ${PROJECT_SOURCE_DIR}/include)
