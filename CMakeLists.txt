cmake_minimum_required(VERSION 3.22)

#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME at32-keyboard-firmware)

# Include toolchain file
include("cmake/gcc-arm-none-eabi.cmake")

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add at32_workbench generated sources
add_subdirectory(cmake/at32_workbench)

set(LIBAMP_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/project/inc)
add_subdirectory(libraries/libamp)
# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

file(GLOB_RECURSE MY_SOURCES
    "${CMAKE_SOURCE_DIR}/project/src/*.c"
    "${CMAKE_SOURCE_DIR}/libraries/CherryUSB/core/usbd_core.c"
    "${CMAKE_SOURCE_DIR}/libraries/CherryUSB/class/hid/usbd_hid.c"
    "${CMAKE_SOURCE_DIR}/libraries/CherryUSB/port/dwc2/usb_glue_at.c"
    "${CMAKE_SOURCE_DIR}/libraries/CherryUSB/port/dwc2/usb_dc_dwc2.c"
    "${CMAKE_SOURCE_DIR}/libraries/libamp/usb/template/cherryusb/usbd_user.c"
    "${CMAKE_SOURCE_DIR}/libraries/libamp/usb/template/cherryusb/mtp/*.c"
    "${CMAKE_SOURCE_DIR}/libraries/ws2812/*.c"
)
# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    ${MY_SOURCES}
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths

    ${CMAKE_SOURCE_DIR}/libraries/CherryUSB/core
    ${CMAKE_SOURCE_DIR}/libraries/CherryUSB/common
    ${CMAKE_SOURCE_DIR}/libraries/CherryUSB/class/hid
    ${CMAKE_SOURCE_DIR}/libraries/CherryUSB/class/midi
    ${CMAKE_SOURCE_DIR}/libraries/CherryUSB/class/hub
    ${CMAKE_SOURCE_DIR}/libraries/CherryUSB/port/dwc2
    ${CMAKE_SOURCE_DIR}/libraries/libamp/usb/template/cherryusb
    ${CMAKE_SOURCE_DIR}/libraries/libamp/usb/template/cherryusb/mtp
    ${CMAKE_SOURCE_DIR}/libraries/ws2812
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    at32_workbench

    # Add user defined libraries
    libamp
    m
)

set(HEX_FILE "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.hex")
set(BIN_FILE "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.bin")

set(OPENOCD_PATH      "${CMAKE_SOURCE_DIR}/tools/OpenOCD_Linux_x86-64_V2.0.6")

set(OPENOCD_SCRIPT    ${CMAKE_SOURCE_DIR}/at32.cfg)
set(OPENOCD           "${OPENOCD_PATH}/bin/openocd")

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${HEX_FILE}
        COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${BIN_FILE}
        COMMENT "Building ${HEX_FILE}\nBuilding ${BIN_FILE}")

# Add a custom target that flash hex file to AT32 using openocd
add_custom_target(flash-hex
        DEPENDS ${CMAKE_PROJECT_NAME}
        COMMAND ${OPENOCD} -f ${OPENOCD_SCRIPT} -c init -c "reset halt" -c "flash write_image erase ${HEX_FILE}" -c reset -c shutdown
        COMMENT "Flashing ${HEX_FILE} to AT32")

# Add a custom target that flash bin file to AT32 using openocd
add_custom_target(flash-bin
        DEPENDS ${CMAKE_PROJECT_NAME}
        COMMAND ${OPENOCD} -f ${OPENOCD_SCRIPT} -c init -c "reset halt" -c "flash write_image erase ${BIN_FILE} 0x08000000" -c reset -c shutdown
        COMMENT "Flashing ${HEX_FILE} to AT32")

# Add a custom target that erase AT32 using openocd
add_custom_target(erase
        COMMAND ${OPENOCD} -f ${OPENOCD_SCRIPT} -c init -c "reset halt" -c "at32f405xx mass_erase 0" -c halt -c shutdown
        COMMENT "Erasing AT32")