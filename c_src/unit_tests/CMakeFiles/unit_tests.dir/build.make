# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

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

# The program to use to edit the cache.
CMAKE_EDIT_COMMAND = /usr/bin/cmake-gui

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /mnt/Storage/Linux/Programming/NES_EMU/c_src

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /mnt/Storage/Linux/Programming/NES_EMU/c_src

# Include any dependencies generated for this target.
include unit_tests/CMakeFiles/unit_tests.dir/depend.make

# Include the progress variables for this target.
include unit_tests/CMakeFiles/unit_tests.dir/progress.make

# Include the compile flags for this target's objects.
include unit_tests/CMakeFiles/unit_tests.dir/flags.make

unit_tests/CMakeFiles/unit_tests.dir/common_unit_test.c.o: unit_tests/CMakeFiles/unit_tests.dir/flags.make
unit_tests/CMakeFiles/unit_tests.dir/common_unit_test.c.o: unit_tests/common_unit_test.c
	$(CMAKE_COMMAND) -E cmake_progress_report /mnt/Storage/Linux/Programming/NES_EMU/c_src/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object unit_tests/CMakeFiles/unit_tests.dir/common_unit_test.c.o"
	cd /mnt/Storage/Linux/Programming/NES_EMU/c_src/unit_tests && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/unit_tests.dir/common_unit_test.c.o   -c /mnt/Storage/Linux/Programming/NES_EMU/c_src/unit_tests/common_unit_test.c

unit_tests/CMakeFiles/unit_tests.dir/common_unit_test.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/unit_tests.dir/common_unit_test.c.i"
	cd /mnt/Storage/Linux/Programming/NES_EMU/c_src/unit_tests && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /mnt/Storage/Linux/Programming/NES_EMU/c_src/unit_tests/common_unit_test.c > CMakeFiles/unit_tests.dir/common_unit_test.c.i

unit_tests/CMakeFiles/unit_tests.dir/common_unit_test.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/unit_tests.dir/common_unit_test.c.s"
	cd /mnt/Storage/Linux/Programming/NES_EMU/c_src/unit_tests && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /mnt/Storage/Linux/Programming/NES_EMU/c_src/unit_tests/common_unit_test.c -o CMakeFiles/unit_tests.dir/common_unit_test.c.s

unit_tests/CMakeFiles/unit_tests.dir/common_unit_test.c.o.requires:
.PHONY : unit_tests/CMakeFiles/unit_tests.dir/common_unit_test.c.o.requires

unit_tests/CMakeFiles/unit_tests.dir/common_unit_test.c.o.provides: unit_tests/CMakeFiles/unit_tests.dir/common_unit_test.c.o.requires
	$(MAKE) -f unit_tests/CMakeFiles/unit_tests.dir/build.make unit_tests/CMakeFiles/unit_tests.dir/common_unit_test.c.o.provides.build
.PHONY : unit_tests/CMakeFiles/unit_tests.dir/common_unit_test.c.o.provides

unit_tests/CMakeFiles/unit_tests.dir/common_unit_test.c.o.provides.build: unit_tests/CMakeFiles/unit_tests.dir/common_unit_test.c.o

unit_tests/CMakeFiles/unit_tests.dir/cpu_basic_operations_unit_tests.c.o: unit_tests/CMakeFiles/unit_tests.dir/flags.make
unit_tests/CMakeFiles/unit_tests.dir/cpu_basic_operations_unit_tests.c.o: unit_tests/cpu_basic_operations_unit_tests.c
	$(CMAKE_COMMAND) -E cmake_progress_report /mnt/Storage/Linux/Programming/NES_EMU/c_src/CMakeFiles $(CMAKE_PROGRESS_2)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object unit_tests/CMakeFiles/unit_tests.dir/cpu_basic_operations_unit_tests.c.o"
	cd /mnt/Storage/Linux/Programming/NES_EMU/c_src/unit_tests && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/unit_tests.dir/cpu_basic_operations_unit_tests.c.o   -c /mnt/Storage/Linux/Programming/NES_EMU/c_src/unit_tests/cpu_basic_operations_unit_tests.c

unit_tests/CMakeFiles/unit_tests.dir/cpu_basic_operations_unit_tests.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/unit_tests.dir/cpu_basic_operations_unit_tests.c.i"
	cd /mnt/Storage/Linux/Programming/NES_EMU/c_src/unit_tests && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /mnt/Storage/Linux/Programming/NES_EMU/c_src/unit_tests/cpu_basic_operations_unit_tests.c > CMakeFiles/unit_tests.dir/cpu_basic_operations_unit_tests.c.i

unit_tests/CMakeFiles/unit_tests.dir/cpu_basic_operations_unit_tests.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/unit_tests.dir/cpu_basic_operations_unit_tests.c.s"
	cd /mnt/Storage/Linux/Programming/NES_EMU/c_src/unit_tests && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /mnt/Storage/Linux/Programming/NES_EMU/c_src/unit_tests/cpu_basic_operations_unit_tests.c -o CMakeFiles/unit_tests.dir/cpu_basic_operations_unit_tests.c.s

unit_tests/CMakeFiles/unit_tests.dir/cpu_basic_operations_unit_tests.c.o.requires:
.PHONY : unit_tests/CMakeFiles/unit_tests.dir/cpu_basic_operations_unit_tests.c.o.requires

unit_tests/CMakeFiles/unit_tests.dir/cpu_basic_operations_unit_tests.c.o.provides: unit_tests/CMakeFiles/unit_tests.dir/cpu_basic_operations_unit_tests.c.o.requires
	$(MAKE) -f unit_tests/CMakeFiles/unit_tests.dir/build.make unit_tests/CMakeFiles/unit_tests.dir/cpu_basic_operations_unit_tests.c.o.provides.build
.PHONY : unit_tests/CMakeFiles/unit_tests.dir/cpu_basic_operations_unit_tests.c.o.provides

unit_tests/CMakeFiles/unit_tests.dir/cpu_basic_operations_unit_tests.c.o.provides.build: unit_tests/CMakeFiles/unit_tests.dir/cpu_basic_operations_unit_tests.c.o

unit_tests/CMakeFiles/unit_tests.dir/cpu_decode_logic_unit_tests.c.o: unit_tests/CMakeFiles/unit_tests.dir/flags.make
unit_tests/CMakeFiles/unit_tests.dir/cpu_decode_logic_unit_tests.c.o: unit_tests/cpu_decode_logic_unit_tests.c
	$(CMAKE_COMMAND) -E cmake_progress_report /mnt/Storage/Linux/Programming/NES_EMU/c_src/CMakeFiles $(CMAKE_PROGRESS_3)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object unit_tests/CMakeFiles/unit_tests.dir/cpu_decode_logic_unit_tests.c.o"
	cd /mnt/Storage/Linux/Programming/NES_EMU/c_src/unit_tests && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/unit_tests.dir/cpu_decode_logic_unit_tests.c.o   -c /mnt/Storage/Linux/Programming/NES_EMU/c_src/unit_tests/cpu_decode_logic_unit_tests.c

unit_tests/CMakeFiles/unit_tests.dir/cpu_decode_logic_unit_tests.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/unit_tests.dir/cpu_decode_logic_unit_tests.c.i"
	cd /mnt/Storage/Linux/Programming/NES_EMU/c_src/unit_tests && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /mnt/Storage/Linux/Programming/NES_EMU/c_src/unit_tests/cpu_decode_logic_unit_tests.c > CMakeFiles/unit_tests.dir/cpu_decode_logic_unit_tests.c.i

unit_tests/CMakeFiles/unit_tests.dir/cpu_decode_logic_unit_tests.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/unit_tests.dir/cpu_decode_logic_unit_tests.c.s"
	cd /mnt/Storage/Linux/Programming/NES_EMU/c_src/unit_tests && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /mnt/Storage/Linux/Programming/NES_EMU/c_src/unit_tests/cpu_decode_logic_unit_tests.c -o CMakeFiles/unit_tests.dir/cpu_decode_logic_unit_tests.c.s

unit_tests/CMakeFiles/unit_tests.dir/cpu_decode_logic_unit_tests.c.o.requires:
.PHONY : unit_tests/CMakeFiles/unit_tests.dir/cpu_decode_logic_unit_tests.c.o.requires

unit_tests/CMakeFiles/unit_tests.dir/cpu_decode_logic_unit_tests.c.o.provides: unit_tests/CMakeFiles/unit_tests.dir/cpu_decode_logic_unit_tests.c.o.requires
	$(MAKE) -f unit_tests/CMakeFiles/unit_tests.dir/build.make unit_tests/CMakeFiles/unit_tests.dir/cpu_decode_logic_unit_tests.c.o.provides.build
.PHONY : unit_tests/CMakeFiles/unit_tests.dir/cpu_decode_logic_unit_tests.c.o.provides

unit_tests/CMakeFiles/unit_tests.dir/cpu_decode_logic_unit_tests.c.o.provides.build: unit_tests/CMakeFiles/unit_tests.dir/cpu_decode_logic_unit_tests.c.o

unit_tests/CMakeFiles/unit_tests.dir/cpu_unit_tests.c.o: unit_tests/CMakeFiles/unit_tests.dir/flags.make
unit_tests/CMakeFiles/unit_tests.dir/cpu_unit_tests.c.o: unit_tests/cpu_unit_tests.c
	$(CMAKE_COMMAND) -E cmake_progress_report /mnt/Storage/Linux/Programming/NES_EMU/c_src/CMakeFiles $(CMAKE_PROGRESS_4)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object unit_tests/CMakeFiles/unit_tests.dir/cpu_unit_tests.c.o"
	cd /mnt/Storage/Linux/Programming/NES_EMU/c_src/unit_tests && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/unit_tests.dir/cpu_unit_tests.c.o   -c /mnt/Storage/Linux/Programming/NES_EMU/c_src/unit_tests/cpu_unit_tests.c

unit_tests/CMakeFiles/unit_tests.dir/cpu_unit_tests.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/unit_tests.dir/cpu_unit_tests.c.i"
	cd /mnt/Storage/Linux/Programming/NES_EMU/c_src/unit_tests && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /mnt/Storage/Linux/Programming/NES_EMU/c_src/unit_tests/cpu_unit_tests.c > CMakeFiles/unit_tests.dir/cpu_unit_tests.c.i

unit_tests/CMakeFiles/unit_tests.dir/cpu_unit_tests.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/unit_tests.dir/cpu_unit_tests.c.s"
	cd /mnt/Storage/Linux/Programming/NES_EMU/c_src/unit_tests && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /mnt/Storage/Linux/Programming/NES_EMU/c_src/unit_tests/cpu_unit_tests.c -o CMakeFiles/unit_tests.dir/cpu_unit_tests.c.s

unit_tests/CMakeFiles/unit_tests.dir/cpu_unit_tests.c.o.requires:
.PHONY : unit_tests/CMakeFiles/unit_tests.dir/cpu_unit_tests.c.o.requires

unit_tests/CMakeFiles/unit_tests.dir/cpu_unit_tests.c.o.provides: unit_tests/CMakeFiles/unit_tests.dir/cpu_unit_tests.c.o.requires
	$(MAKE) -f unit_tests/CMakeFiles/unit_tests.dir/build.make unit_tests/CMakeFiles/unit_tests.dir/cpu_unit_tests.c.o.provides.build
.PHONY : unit_tests/CMakeFiles/unit_tests.dir/cpu_unit_tests.c.o.provides

unit_tests/CMakeFiles/unit_tests.dir/cpu_unit_tests.c.o.provides.build: unit_tests/CMakeFiles/unit_tests.dir/cpu_unit_tests.c.o

# Object files for target unit_tests
unit_tests_OBJECTS = \
"CMakeFiles/unit_tests.dir/common_unit_test.c.o" \
"CMakeFiles/unit_tests.dir/cpu_basic_operations_unit_tests.c.o" \
"CMakeFiles/unit_tests.dir/cpu_decode_logic_unit_tests.c.o" \
"CMakeFiles/unit_tests.dir/cpu_unit_tests.c.o"

# External object files for target unit_tests
unit_tests_EXTERNAL_OBJECTS =

unit_tests/libunit_tests.a: unit_tests/CMakeFiles/unit_tests.dir/common_unit_test.c.o
unit_tests/libunit_tests.a: unit_tests/CMakeFiles/unit_tests.dir/cpu_basic_operations_unit_tests.c.o
unit_tests/libunit_tests.a: unit_tests/CMakeFiles/unit_tests.dir/cpu_decode_logic_unit_tests.c.o
unit_tests/libunit_tests.a: unit_tests/CMakeFiles/unit_tests.dir/cpu_unit_tests.c.o
unit_tests/libunit_tests.a: unit_tests/CMakeFiles/unit_tests.dir/build.make
unit_tests/libunit_tests.a: unit_tests/CMakeFiles/unit_tests.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C static library libunit_tests.a"
	cd /mnt/Storage/Linux/Programming/NES_EMU/c_src/unit_tests && $(CMAKE_COMMAND) -P CMakeFiles/unit_tests.dir/cmake_clean_target.cmake
	cd /mnt/Storage/Linux/Programming/NES_EMU/c_src/unit_tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/unit_tests.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
unit_tests/CMakeFiles/unit_tests.dir/build: unit_tests/libunit_tests.a
.PHONY : unit_tests/CMakeFiles/unit_tests.dir/build

unit_tests/CMakeFiles/unit_tests.dir/requires: unit_tests/CMakeFiles/unit_tests.dir/common_unit_test.c.o.requires
unit_tests/CMakeFiles/unit_tests.dir/requires: unit_tests/CMakeFiles/unit_tests.dir/cpu_basic_operations_unit_tests.c.o.requires
unit_tests/CMakeFiles/unit_tests.dir/requires: unit_tests/CMakeFiles/unit_tests.dir/cpu_decode_logic_unit_tests.c.o.requires
unit_tests/CMakeFiles/unit_tests.dir/requires: unit_tests/CMakeFiles/unit_tests.dir/cpu_unit_tests.c.o.requires
.PHONY : unit_tests/CMakeFiles/unit_tests.dir/requires

unit_tests/CMakeFiles/unit_tests.dir/clean:
	cd /mnt/Storage/Linux/Programming/NES_EMU/c_src/unit_tests && $(CMAKE_COMMAND) -P CMakeFiles/unit_tests.dir/cmake_clean.cmake
.PHONY : unit_tests/CMakeFiles/unit_tests.dir/clean

unit_tests/CMakeFiles/unit_tests.dir/depend:
	cd /mnt/Storage/Linux/Programming/NES_EMU/c_src && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /mnt/Storage/Linux/Programming/NES_EMU/c_src /mnt/Storage/Linux/Programming/NES_EMU/c_src/unit_tests /mnt/Storage/Linux/Programming/NES_EMU/c_src /mnt/Storage/Linux/Programming/NES_EMU/c_src/unit_tests /mnt/Storage/Linux/Programming/NES_EMU/c_src/unit_tests/CMakeFiles/unit_tests.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : unit_tests/CMakeFiles/unit_tests.dir/depend

