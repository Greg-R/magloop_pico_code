# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# initalize pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "/home/pi/pico/pico/pico-sdk")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(magloop_pico C CXX ASM)

set(PICO_CXX_ENABLE_EXCEPTIONS 1)

set(PICO_CXX_ENABLE_RTTI 1)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

#add_executable(magloop_pico magloop_pico.cpp magloop/StepperManagement.cpp)
add_executable(magloop_pico magloop_pico.cpp)

pico_set_program_name(magloop_pico "magloop_pico")
pico_set_program_version(magloop_pico "0.1")

pico_enable_stdio_uart(magloop_pico 1)
pico_enable_stdio_usb(magloop_pico 0)

add_subdirectory(Adafruit_ILI9341)
add_subdirectory(Adafruit-GFX-Library)
add_subdirectory(arduino_includes)
add_subdirectory(Adafruit_SPITFT)
add_subdirectory(AccelStepper)
add_subdirectory(StepperManagement)
add_subdirectory(DDS)
add_subdirectory(AutoTune)
add_subdirectory(SWR)
add_subdirectory(DisplayManagement)
#add_subdirectory(Calibrate)
#add_subdirectory(Buttons)
#add_subdirectory(Presets)
add_subdirectory(Rotary)
add_subdirectory(EEPROM)

#target_include_directories(magloop_pico INTERFACE  ./arduino_includes ./Adafruit_ILI9341 ./AccelStepper ./StepperManagement ./DDS ./AutoTune ./SWR ./DisplayManagement ./Calibrate ./Buttons ./Presets)
#target_include_directories(magloop_pico INTERFACE  ./arduino_includes ./Adafruit_ILI9341 ./AccelStepper ./StepperManagement ./DisplayManagement)
#target_include_directories(magloop_pico INTERFACE  ./arduino_includes ./AccelStepper ./Adafruit-GFX-Library ./Adafruit-GFX-Library/Fonts ./StepperManagement)
#target_include_directories(magloop_pico INTERFACE ./DDS)

# Add any user requested libraries
target_link_libraries(magloop_pico
        pico_stdlib
        hardware_spi
        hardware_timer
        hardware_clocks
        hardware_adc
        hardware_flash
        hardware_sync
        ili9341
        gfx
        spitft
        print
        stepper
        steppermanagement
        dds
        AutoTune
        SWR
        DisplayManagement
  #      Calibrate
  #      Buttons
  #      Presets
        Rotary
        EEPROM
        )

pico_add_extra_outputs(magloop_pico)

