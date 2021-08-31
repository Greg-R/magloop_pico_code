add_library(SWR INTERFACE)

target_sources(SWR INTERFACE SWR.cpp SWR.h)

target_include_directories(SWR INTERFACE ../arduino_includes ../magloop ../Adafruit_ILI9341)

