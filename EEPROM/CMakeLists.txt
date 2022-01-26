add_library(EEPROM INTERFACE)

target_sources(EEPROM INTERFACE EEPROM.cpp EEPROM.h)

target_include_directories(EEPROM INTERFACE ../Adafruit_ILI9341)
