cmake_minimum_required(VERSION 3.20)
project(GardenGuardians VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

option(USE_LOCAL_RAYLIB "Use vendored local raylib source" OFF)
set(LOCAL_RAYLIB_DIR "${CMAKE_SOURCE_DIR}/third_party/raylib" CACHE PATH "Path to local raylib source")

if(USE_LOCAL_RAYLIB AND EXISTS "${LOCAL_RAYLIB_DIR}/src/raylib.h")
  add_subdirectory("${LOCAL_RAYLIB_DIR}" raylib_local_build)
else()
  FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
  )
  FetchContent_MakeAvailable(raylib)
endif()

add_executable(GardenGuardians
  src/main.cpp
  src/Game.cpp
  src/Board.cpp
  src/Plant.cpp
  src/Zombie.cpp
  src/WaveManager.cpp
  src/GameConfig.cpp
  src/SaveSystem.cpp
  src/ProceduralSprites.cpp
)

target_include_directories(GardenGuardians PRIVATE include)

if(APPLE)
  target_link_libraries(GardenGuardians PRIVATE raylib "-framework IOKit" "-framework Cocoa" "-framework OpenGL")
elseif(UNIX)
  target_link_libraries(GardenGuardians PRIVATE raylib m pthread dl)
else()
  target_link_libraries(GardenGuardians PRIVATE raylib winmm)
endif()

target_compile_options(GardenGuardians PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

# Zip-ready folder next to the build directory (only the game + assets; no raylib SDK).
set(GG_PORTABLE_DIR "${CMAKE_BINARY_DIR}/GardenGuardians-portable" CACHE STRING "Output folder for portable_bundle target")
add_custom_target(portable_bundle
  DEPENDS GardenGuardians
  COMMAND ${CMAKE_COMMAND} -E rm -rf "${GG_PORTABLE_DIR}"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${GG_PORTABLE_DIR}"
  COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:GardenGuardians>" "${GG_PORTABLE_DIR}/"
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/assets" "${GG_PORTABLE_DIR}/assets"
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/packaging/PLAYER_README.txt" "${GG_PORTABLE_DIR}/README.txt"
  COMMENT "Creating portable bundle in ${GG_PORTABLE_DIR}"
  VERBATIM)
