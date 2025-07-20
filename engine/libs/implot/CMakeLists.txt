list(
    APPEND LIBRARY_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/implot_demo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/implot_internal.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/implot_items.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/implot.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/implot.h"
)

set(LIBRARY_NAME "Implot")
add_library(${LIBRARY_NAME} ${LIBRARY_SOURCES})