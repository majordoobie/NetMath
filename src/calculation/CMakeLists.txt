include(BuildUtils)

add_library(calculation SHARED calculation.c)
set_project_properties(calculation ${PROJECT_SOURCE_DIR}/include)