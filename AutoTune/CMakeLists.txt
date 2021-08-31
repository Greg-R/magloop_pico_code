add_library(AutoTune INTERFACE)

target_sources(AutoTune INTERFACE AutoTune.cpp AutoTune.h)

target_include_directories(AutoTune INTERFACE ../SWR ../AccelStepper)

target_include_directories(AutoTune INTERFACE ../Adafruit_ILI9341 ../magloop)