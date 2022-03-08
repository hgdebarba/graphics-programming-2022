## set target project
file(GLOB target_src "*.h" "*.cpp") # look for source files
file(GLOB target_shaders "shaders/*.vert" "shaders/*.frag") # look for shaders
add_executable(${subdir} ${target_src} ${target_shaders})

## set link libraries
target_link_libraries(${subdir} ${libraries})

## add local source directory to include paths
target_include_directories(${subdir} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

## copy shaders folder to build folder
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

## copy models
file(GLOB car_geom "${CMAKE_SOURCE_DIR}/common/models/car/*.obj")
file(COPY ${car_geom} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/car)
file(COPY ${CMAKE_SOURCE_DIR}/common/models/floor/floor.obj DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/floor)

## copy again at the time the current target gets compiled
add_custom_command(
        TARGET ${subdir} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders ${CMAKE_CURRENT_BINARY_DIR}/shaders
        COMMENT "Copying shaders" VERBATIM
)