add_library(EEPROM INTERFACE)

target_sources(EEPROM INTERFACE EEPROM.cpp EEPROM.h)

#target_include_directories(AutoTune INTERFACE ../SWR ../AccelStepper)
