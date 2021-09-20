add_library(Calibrate INTERFACE)

target_sources(Calibrate INTERFACE Calibrate.cpp Calibrate.h)

target_include_directories(Calibrate INTERFACE ../SWR ../AccelStepper ../DisplayManagement ../AutoTune ../Adafruit_ILI9341 ../magloop ../arduino_includes ../DDS)

target_include_directories(Calibrate INTERFACE ../Adafruit-GFX-Library/Fonts)