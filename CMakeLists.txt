set(COMPONENT_ADD_INCLUDEDIRS
    src
    src/lgfx
    src/lgfx/platforms
    src/lgfx/utility
    src/Fonts
    )
file(GLOB SRCS src/lgfx/*.cpp src/lgfx/platforms/*.cpp src/lgfx/utility/*.c src/Fonts/*.cpp)
set(COMPONENT_SRCS ${SRCS})

register_component()
