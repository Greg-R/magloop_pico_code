add_library(Calibrate INTERFACE)

target_sources(Calibrate INTERFACE Calibrate.cpp Calibrate.h)

target_include_directories(Calibrate INTERFACE ../SWR ../AccelStepper ../DisplayManagement ../AutoTune)

target_include_directories(Calibrate INTERFACE ../Adafruit_ILI9341 ../magloop ../arduino_includes ../DDS)