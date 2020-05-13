set(COMPONENT_ADD_INCLUDEDIRS
    src
    src/lgfx
    src/lgfx/platforms
    src/lgfx/utility
    src/Fonts
    )
file(GLOB UTIL_SRCS src/lgfx/utility/*.c src/Fonts/*.cpp)
set(COMPONENT_SRCS ${UTIL_SRCS})

register_component()
