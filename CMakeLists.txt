set(COMPONENT_ADD_INCLUDEDIRS
    src
    src/lgfx
    src/lgfx/platforms
    src/lgfx/Fonts
    )
file(GLOB FONT_SRCS Fonts/*.c)
set(COMPONENT_SRCS ${FONT_SRCS})

register_component()
