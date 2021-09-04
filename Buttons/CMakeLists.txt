add_library(Buttons INTERFACE)

target_sources(Buttons INTERFACE Buttons.cpp Buttons.h)

target_include_directories(Buttons INTERFACE ../arduino_includes ../magloop ../Adafruit_ILI9341 ../SWR ../Presets ../DDS ../Calibrate)

