include(build_utils)

add_library(utils SHARED utils.c)
set_project_properties(utils ${PROJECT_SOURCE_DIR}/include)
