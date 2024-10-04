# Espressif ESP* based boards
if (DEFINED ESP_PLATFORM)
include(${CMAKE_CURRENT_LIST_DIR}/boards.cmake/esp-idf.cmake)

# Raspberry Pi RP* based boards
elseif ((DEFINED PICO_PLATFORM) OR (DEFINED PICO_BOARD))
include(${CMAKE_CURRENT_LIST_DIR}/boards.cmake/pico-sdk.cmake)

# Cannot determine boards type
else()
message("Error: No platform specified")
endif()
