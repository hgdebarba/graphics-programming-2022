
SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_LIST_DIR})

# ---------------------------------------------------------------------------------
# Executable and target include/link libraries
# ---------------------------------------------------------------------------------

set(libraries glad glfw)

if(APPLE)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(OPENGL_LIBRARY OpenGL)
    find_library(COREVIDEO_LIBRARY CoreVideo)

    list(APPEND libraries
            ${OPENGL_LIBRARY}
            ${COCOA_LIBRARY}
            ${IOKIT_LIBRARY}
            ${COREVIDEO_LIBRARY}
            )
endif()


FOREACH(subdir ${SUBDIRS})
    add_executable(${subdir} ${subdir}/main.cpp)
    target_link_libraries(${subdir} ${libraries})
ENDFOREACH()