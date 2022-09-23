include(build_utils)

add_library(calculation SHARED calculation.c)
target_link_libraries(calculation PUBLIC utils)
set_project_properties(calculation ${PROJECT_SOURCE_DIR}/include)