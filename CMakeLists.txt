set(COMPONENT_ADD_INCLUDEDIRS
    src
    )
file(GLOB SRCS
     src/lgfx/Fonts/efont/*.c
     src/lgfx/Fonts/IPA/*.c
     src/lgfx/utility/*.c
     src/lgfx/v0/*.cpp
     src/lgfx/v0/panel/*.cpp
     src/lgfx/v0/platforms/*.cpp
     src/lgfx/v0/touch/*.cpp
     )
set(COMPONENT_SRCS ${SRCS})

register_component()
