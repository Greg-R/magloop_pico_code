# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/pi/projects_pico/magloop_pico/Adafruit_ILI9341

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/pi/projects_pico/magloop_pico/Adafruit_ILI9341/build

# Include any dependencies generated for this target.
include CMakeFiles/ili9341.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/ili9341.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/ili9341.dir/flags.make

CMakeFiles/ili9341.dir/Adafruit_ILI9341.o: CMakeFiles/ili9341.dir/flags.make
CMakeFiles/ili9341.dir/Adafruit_ILI9341.o: ../Adafruit_ILI9341.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pi/projects_pico/magloop_pico/Adafruit_ILI9341/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/ili9341.dir/Adafruit_ILI9341.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/ili9341.dir/Adafruit_ILI9341.o -c /home/pi/projects_pico/magloop_pico/Adafruit_ILI9341/Adafruit_ILI9341.cpp

CMakeFiles/ili9341.dir/Adafruit_ILI9341.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ili9341.dir/Adafruit_ILI9341.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/pi/projects_pico/magloop_pico/Adafruit_ILI9341/Adafruit_ILI9341.cpp > CMakeFiles/ili9341.dir/Adafruit_ILI9341.i

CMakeFiles/ili9341.dir/Adafruit_ILI9341.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ili9341.dir/Adafruit_ILI9341.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/pi/projects_pico/magloop_pico/Adafruit_ILI9341/Adafruit_ILI9341.cpp -o CMakeFiles/ili9341.dir/Adafruit_ILI9341.s

# Object files for target ili9341
ili9341_OBJECTS = \
"CMakeFiles/ili9341.dir/Adafruit_ILI9341.o"

# External object files for target ili9341
ili9341_EXTERNAL_OBJECTS =

libili9341.a: CMakeFiles/ili9341.dir/Adafruit_ILI9341.o
libili9341.a: CMakeFiles/ili9341.dir/build.make
libili9341.a: CMakeFiles/ili9341.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/pi/projects_pico/magloop_pico/Adafruit_ILI9341/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library libili9341.a"
	$(CMAKE_COMMAND) -P CMakeFiles/ili9341.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/ili9341.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/ili9341.dir/build: libili9341.a

.PHONY : CMakeFiles/ili9341.dir/build

CMakeFiles/ili9341.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/ili9341.dir/cmake_clean.cmake
.PHONY : CMakeFiles/ili9341.dir/clean

CMakeFiles/ili9341.dir/depend:
	cd /home/pi/projects_pico/magloop_pico/Adafruit_ILI9341/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/pi/projects_pico/magloop_pico/Adafruit_ILI9341 /home/pi/projects_pico/magloop_pico/Adafruit_ILI9341 /home/pi/projects_pico/magloop_pico/Adafruit_ILI9341/build /home/pi/projects_pico/magloop_pico/Adafruit_ILI9341/build /home/pi/projects_pico/magloop_pico/Adafruit_ILI9341/build/CMakeFiles/ili9341.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/ili9341.dir/depend

