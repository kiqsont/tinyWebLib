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
CMAKE_SOURCE_DIR = /home/kiqsont/program/logger/asyncLogger

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/kiqsont/program/logger/asyncLogger/build

# Include any dependencies generated for this target.
include example/CMakeFiles/test.out.dir/depend.make

# Include the progress variables for this target.
include example/CMakeFiles/test.out.dir/progress.make

# Include the compile flags for this target's objects.
include example/CMakeFiles/test.out.dir/flags.make

example/CMakeFiles/test.out.dir/test.cc.o: example/CMakeFiles/test.out.dir/flags.make
example/CMakeFiles/test.out.dir/test.cc.o: ../example/test.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kiqsont/program/logger/asyncLogger/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object example/CMakeFiles/test.out.dir/test.cc.o"
	cd /home/kiqsont/program/logger/asyncLogger/build/example && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/test.out.dir/test.cc.o -c /home/kiqsont/program/logger/asyncLogger/example/test.cc

example/CMakeFiles/test.out.dir/test.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test.out.dir/test.cc.i"
	cd /home/kiqsont/program/logger/asyncLogger/build/example && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kiqsont/program/logger/asyncLogger/example/test.cc > CMakeFiles/test.out.dir/test.cc.i

example/CMakeFiles/test.out.dir/test.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test.out.dir/test.cc.s"
	cd /home/kiqsont/program/logger/asyncLogger/build/example && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kiqsont/program/logger/asyncLogger/example/test.cc -o CMakeFiles/test.out.dir/test.cc.s

# Object files for target test.out
test_out_OBJECTS = \
"CMakeFiles/test.out.dir/test.cc.o"

# External object files for target test.out
test_out_EXTERNAL_OBJECTS =

bin/test.out: example/CMakeFiles/test.out.dir/test.cc.o
bin/test.out: example/CMakeFiles/test.out.dir/build.make
bin/test.out: ../lib/libasyncLogger.so
bin/test.out: example/CMakeFiles/test.out.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/kiqsont/program/logger/asyncLogger/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../bin/test.out"
	cd /home/kiqsont/program/logger/asyncLogger/build/example && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test.out.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
example/CMakeFiles/test.out.dir/build: bin/test.out

.PHONY : example/CMakeFiles/test.out.dir/build

example/CMakeFiles/test.out.dir/clean:
	cd /home/kiqsont/program/logger/asyncLogger/build/example && $(CMAKE_COMMAND) -P CMakeFiles/test.out.dir/cmake_clean.cmake
.PHONY : example/CMakeFiles/test.out.dir/clean

example/CMakeFiles/test.out.dir/depend:
	cd /home/kiqsont/program/logger/asyncLogger/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/kiqsont/program/logger/asyncLogger /home/kiqsont/program/logger/asyncLogger/example /home/kiqsont/program/logger/asyncLogger/build /home/kiqsont/program/logger/asyncLogger/build/example /home/kiqsont/program/logger/asyncLogger/build/example/CMakeFiles/test.out.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : example/CMakeFiles/test.out.dir/depend
