add_library(Presets INTERFACE)

target_sources(Presets INTERFACE Presets.cpp Presets.h)

target_include_directories(Presets INTERFACE ../arduino_includes ../magloop ../Adafruit_ILI9341 ../SWR ../AccelStepper ../DDS ../DisplayManagement)

