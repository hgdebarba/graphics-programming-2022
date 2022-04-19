## set target project
file(GLOB target_src "*.h" "*.cpp" "rasterizer/*.h" "rasterizer/*.cpp" "renderer/*.h" "renderer/*.cpp") # look for source files

add_executable(${subdir} ${target_src} renderer/rt_renderer.h renderer/rt_types.h)

## set link libraries
target_link_libraries(${subdir} ${libraries})

## add local source directory to include paths
target_include_directories(${subdir} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/rasterizer ${CMAKE_CURRENT_SOURCE_DIR}/renderer)

