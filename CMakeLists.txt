set(COMPONENT_ADD_INCLUDEDIRS
    .
    platforms
    Fonts
    )
file(GLOB FONT_SRCS Fonts/*.c)
set(COMPONENT_SRCS ${FONT_SRCS})

register_component()
