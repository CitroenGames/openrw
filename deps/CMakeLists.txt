if(ENABLE_PROFILING)
  add_subdirectory(microprofile)
endif()

add_subdirectory(imgui)
# disable bullet examples
set(BULLET_BUILD_EXTRAS OFF)
set(BULLET_BUILD_UNIT_TESTS OFF)
add_subdirectory(bullet)
add_subdirectory(openal-soft-1.23.1)
add_subdirectory(SDL-2.30.5)