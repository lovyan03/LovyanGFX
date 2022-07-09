cmake_minimum_required (VERSION 3.8)
project(LGFX_SDL)

set( LovyanGFX_DIR "../../../LovyanGFX/src" )
# set( SDL2_DIR  "C:/path/to/SDL2" )

add_definitions(-DLGFX_SDL)

file(GLOB Target_Files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} CONFIGURE_DEPENDS
    *.cpp
    ${LovyanGFX_DIR}/lgfx/Fonts/efont/*.c
    ${LovyanGFX_DIR}/lgfx/Fonts/IPA/*.c
    ${LovyanGFX_DIR}/lgfx/utility/*.c
    ${LovyanGFX_DIR}/lgfx/v1/*.cpp
    ${LovyanGFX_DIR}/lgfx/v1/misc/*.cpp
    ${LovyanGFX_DIR}/lgfx/v1/panel/Panel_Device.cpp
    ${LovyanGFX_DIR}/lgfx/v1/platforms/sdl/*.cpp
    )
add_executable (${PROJECT_NAME} ${Target_Files})

find_package(SDL2 REQUIRED)

message(STATUS "LGFX_SDL status:")
message(STATUS "    LovyanGFX dir     = ${LovyanGFX_DIR}")
message(STATUS "    SDL2 include path = ${SDL2_INCLUDE_DIRS}")
message(STATUS "    SDL2 libraries    = ${SDL2_LIBRARIES}")

target_include_directories(${PROJECT_NAME} PUBLIC ${LovyanGFX_DIR} ${SDL2_INCLUDE_DIRS})

if(UNIX)
  target_link_libraries(${PROJECT_NAME} -lpthread ${SDL2_LIBRARIES})
endif()

if(WIN32)
  set(CMAKE_CXX_COMPILER "clang++")
  target_link_libraries(${PROJECT_NAME} PUBLIC ${SDL2_LIBRARIES})
  # Copy the SDL .dll file into the application binary directory
  add_custom_command(
      TARGET ${PROJECT_NAME} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
      $<TARGET_FILE:SDL2::SDL2>
      $<TARGET_FILE_DIR:${PROJECT_NAME}>
      VERBATIM
  )
endif()


target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

