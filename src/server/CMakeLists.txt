include(BuildUtils)

add_library(server SHARED server.c)
target_link_libraries(server PUBLIC utils)
set_project_properties(server ${PROJECT_SOURCE_DIR}/include)
