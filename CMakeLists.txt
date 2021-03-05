set(COMPONENT_ADD_INCLUDEDIRS
    src
    )
file(GLOB SRCS
     src/Fonts/*.cpp)
     src/lgfx/*.cpp
     src/lgfx/panel/*.cpp
     src/lgfx/platforms/*.cpp
     src/lgfx/touch/*.cpp
     src/utility/*.c
set(COMPONENT_SRCS ${SRCS})

register_component()
