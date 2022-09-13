include(BuildUtils)

add_library(header_parser SHARED header_parser.c)
target_link_libraries(header_parser PUBLIC utils calculation)
set_project_properties(header_parser ${PROJECT_SOURCE_DIR}/include)
