// LovyanGFX v1 uses functional hubs for *.inl implementations and standalone *.cpp
// files for LGFXBase, fonts, and implementations with large data tables.
// Older RISC-V toolchains (assembler and linker) can retain unused code throughout
// a translation unit when its last function is used. Keeping functional groups and large tables in separate
// translation units limits this effect while sharing header parsing within each hub.
//
// Maintenance notes:
// - Add a small implementation as *.inl to the matching hub: lgfx_v1.cpp for sprites,
//   buttons and misc, lgfx_v1_panel.cpp for panels, lgfx_v1_touch.cpp for touch, or
//   lgfx_v1_platforms.cpp for platform back ends. Keep implementations with large
//   tables in standalone *.cpp files, and update the CMake source lists and SourceCheck.
// - LovyanGFX and M5GFX share this source tree but keep their own hub include lists
//   because their supported panels and platforms differ; update both.
// - File-local names are visible to later includes within the same hub. Use names
//   identifying the file (or class members), and #undef helper macros at the end.
//   Dependencies across translation units need declarations in shared headers.
// - Platform directories are self-guarded. Desktop back ends follow the selection
//   in platforms/common.hpp (LGFX_PLATFORM_*). Keep the platform hub's "must precede"
//   includes in order: FrameBufferBase provides cache helpers and esp32/common
//   provides reg() / writereg().
// - C decoders in src/lgfx/utility and font tables in src/lgfx/Fonts stay separate
//   translation units (third-party file-local names collide; the font tables would
//   make a single 100+ MB translation unit).
// - .github/scripts/check_inl_sources.py checks reachability, guards, allowed sources,
//   and ownership by exactly one hub. It also compiles each direct *.inl include on
//   its own; run the commands in .github/workflows/SourceCheck.yml locally.

#define LGFX_V1_IMPLEMENTATION

#include "LGFX_Sprite.inl"
#include "LGFX_Button.inl"
#include "misc/DividedFrameBuffer.inl"
#include "misc/SpriteBuffer.inl"
#include "misc/common_function.inl"
#include "misc/pixelcopy.inl"

#undef LGFX_V1_IMPLEMENTATION
