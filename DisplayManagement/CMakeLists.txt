add_library(DisplayManagement INTERFACE)

target_sources(DisplayManagement INTERFACE DisplayManagement.cpp DisplayManagement.h)

target_include_directories(DisplayManagement INTERFACE ../Adafruit_ILI9341 ../magloop ../arduino_includes ../DDS ../SWR ../AutoTune ../EEPROM)

