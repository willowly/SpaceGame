# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.29

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/Cellar/cmake/3.29.2/bin/cmake

# The command to remove a file.
RM = /usr/local/Cellar/cmake/3.29.2/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/willow/Documents/GitHub/SpaceGame

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/willow/Documents/GitHub/SpaceGame/build

# Include any dependencies generated for this target.
include CMakeFiles/STB.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/STB.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/STB.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/STB.dir/flags.make

CMakeFiles/STB.dir/include/stb_image.cpp.o: CMakeFiles/STB.dir/flags.make
CMakeFiles/STB.dir/include/stb_image.cpp.o: /Users/willow/Documents/GitHub/SpaceGame/include/stb_image.cpp
CMakeFiles/STB.dir/include/stb_image.cpp.o: CMakeFiles/STB.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/willow/Documents/GitHub/SpaceGame/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/STB.dir/include/stb_image.cpp.o"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/STB.dir/include/stb_image.cpp.o -MF CMakeFiles/STB.dir/include/stb_image.cpp.o.d -o CMakeFiles/STB.dir/include/stb_image.cpp.o -c /Users/willow/Documents/GitHub/SpaceGame/include/stb_image.cpp

CMakeFiles/STB.dir/include/stb_image.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/STB.dir/include/stb_image.cpp.i"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/willow/Documents/GitHub/SpaceGame/include/stb_image.cpp > CMakeFiles/STB.dir/include/stb_image.cpp.i

CMakeFiles/STB.dir/include/stb_image.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/STB.dir/include/stb_image.cpp.s"
	/usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/willow/Documents/GitHub/SpaceGame/include/stb_image.cpp -o CMakeFiles/STB.dir/include/stb_image.cpp.s

# Object files for target STB
STB_OBJECTS = \
"CMakeFiles/STB.dir/include/stb_image.cpp.o"

# External object files for target STB
STB_EXTERNAL_OBJECTS =

libSTB.a: CMakeFiles/STB.dir/include/stb_image.cpp.o
libSTB.a: CMakeFiles/STB.dir/build.make
libSTB.a: CMakeFiles/STB.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/Users/willow/Documents/GitHub/SpaceGame/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library libSTB.a"
	$(CMAKE_COMMAND) -P CMakeFiles/STB.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/STB.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/STB.dir/build: libSTB.a
.PHONY : CMakeFiles/STB.dir/build

CMakeFiles/STB.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/STB.dir/cmake_clean.cmake
.PHONY : CMakeFiles/STB.dir/clean

CMakeFiles/STB.dir/depend:
	cd /Users/willow/Documents/GitHub/SpaceGame/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/willow/Documents/GitHub/SpaceGame /Users/willow/Documents/GitHub/SpaceGame /Users/willow/Documents/GitHub/SpaceGame/build /Users/willow/Documents/GitHub/SpaceGame/build /Users/willow/Documents/GitHub/SpaceGame/build/CMakeFiles/STB.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/STB.dir/depend

