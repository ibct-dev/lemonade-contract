# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.19

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
CMAKE_COMMAND = /usr/local/Cellar/cmake/3.19.1/bin/cmake

# The command to remove a file.
RM = /usr/local/Cellar/cmake/3.19.1/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/ibct/workspace/develop/led.public.contracts/contracts

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/ibct/workspace/develop/led.public.contracts/build/contracts

# Include any dependencies generated for this target.
include nftmarket/CMakeFiles/nftmarket.dir/depend.make

# Include the progress variables for this target.
include nftmarket/CMakeFiles/nftmarket.dir/progress.make

# Include the compile flags for this target's objects.
include nftmarket/CMakeFiles/nftmarket.dir/flags.make

nftmarket/CMakeFiles/nftmarket.dir/src/nftmarket.cpp.obj: nftmarket/CMakeFiles/nftmarket.dir/flags.make
nftmarket/CMakeFiles/nftmarket.dir/src/nftmarket.cpp.obj: /Users/ibct/workspace/develop/led.public.contracts/contracts/nftmarket/src/nftmarket.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/ibct/workspace/develop/led.public.contracts/build/contracts/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object nftmarket/CMakeFiles/nftmarket.dir/src/nftmarket.cpp.obj"
	cd /Users/ibct/workspace/develop/led.public.contracts/build/contracts/nftmarket && //usr/local/bin/eosio-cpp $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/nftmarket.dir/src/nftmarket.cpp.obj -c /Users/ibct/workspace/develop/led.public.contracts/contracts/nftmarket/src/nftmarket.cpp

nftmarket/CMakeFiles/nftmarket.dir/src/nftmarket.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/nftmarket.dir/src/nftmarket.cpp.i"
	cd /Users/ibct/workspace/develop/led.public.contracts/build/contracts/nftmarket && //usr/local/bin/eosio-cpp $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/ibct/workspace/develop/led.public.contracts/contracts/nftmarket/src/nftmarket.cpp > CMakeFiles/nftmarket.dir/src/nftmarket.cpp.i

nftmarket/CMakeFiles/nftmarket.dir/src/nftmarket.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/nftmarket.dir/src/nftmarket.cpp.s"
	cd /Users/ibct/workspace/develop/led.public.contracts/build/contracts/nftmarket && //usr/local/bin/eosio-cpp $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/ibct/workspace/develop/led.public.contracts/contracts/nftmarket/src/nftmarket.cpp -o CMakeFiles/nftmarket.dir/src/nftmarket.cpp.s

# Object files for target nftmarket
led_token_OBJECTS = \
"CMakeFiles/nftmarket.dir/src/nftmarket.cpp.obj"

# External object files for target nftmarket
led_token_EXTERNAL_OBJECTS =

nftmarket/nftmarket.wasm: nftmarket/CMakeFiles/nftmarket.dir/src/nftmarket.cpp.obj
nftmarket/nftmarket.wasm: nftmarket/CMakeFiles/nftmarket.dir/build.make
nftmarket/nftmarket.wasm: nftmarket/CMakeFiles/nftmarket.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/ibct/workspace/develop/led.public.contracts/build/contracts/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable nftmarket.wasm"
	cd /Users/ibct/workspace/develop/led.public.contracts/build/contracts/nftmarket && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/nftmarket.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
nftmarket/CMakeFiles/nftmarket.dir/build: nftmarket/nftmarket.wasm

.PHONY : nftmarket/CMakeFiles/nftmarket.dir/build

nftmarket/CMakeFiles/nftmarket.dir/clean:
	cd /Users/ibct/workspace/develop/led.public.contracts/build/contracts/nftmarket && $(CMAKE_COMMAND) -P CMakeFiles/nftmarket.dir/cmake_clean.cmake
.PHONY : nftmarket/CMakeFiles/nftmarket.dir/clean

nftmarket/CMakeFiles/nftmarket.dir/depend:
	cd /Users/ibct/workspace/develop/led.public.contracts/build/contracts && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/ibct/workspace/develop/led.public.contracts/contracts /Users/ibct/workspace/develop/led.public.contracts/contracts/nftmarket /Users/ibct/workspace/develop/led.public.contracts/build/contracts /Users/ibct/workspace/develop/led.public.contracts/build/contracts/nftmarket /Users/ibct/workspace/develop/led.public.contracts/build/contracts/nftmarket/CMakeFiles/nftmarket.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : nftmarket/CMakeFiles/nftmarket.dir/depend

