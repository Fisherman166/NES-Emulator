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
include cpu/CMakeFiles/cpu.dir/depend.make

# Include the progress variables for this target.
include cpu/CMakeFiles/cpu.dir/progress.make

# Include the compile flags for this target's objects.
include cpu/CMakeFiles/cpu.dir/flags.make

cpu/CMakeFiles/cpu.dir/cpu_basic_operations.c.o: cpu/CMakeFiles/cpu.dir/flags.make
cpu/CMakeFiles/cpu.dir/cpu_basic_operations.c.o: cpu/cpu_basic_operations.c
	$(CMAKE_COMMAND) -E cmake_progress_report /mnt/Storage/Linux/Programming/NES_EMU/c_src/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object cpu/CMakeFiles/cpu.dir/cpu_basic_operations.c.o"
	cd /mnt/Storage/Linux/Programming/NES_EMU/c_src/cpu && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/cpu.dir/cpu_basic_operations.c.o   -c /mnt/Storage/Linux/Programming/NES_EMU/c_src/cpu/cpu_basic_operations.c

cpu/CMakeFiles/cpu.dir/cpu_basic_operations.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/cpu.dir/cpu_basic_operations.c.i"
	cd /mnt/Storage/Linux/Programming/NES_EMU/c_src/cpu && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /mnt/Storage/Linux/Programming/NES_EMU/c_src/cpu/cpu_basic_operations.c > CMakeFiles/cpu.dir/cpu_basic_operations.c.i

cpu/CMakeFiles/cpu.dir/cpu_basic_operations.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/cpu.dir/cpu_basic_operations.c.s"
	cd /mnt/Storage/Linux/Programming/NES_EMU/c_src/cpu && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /mnt/Storage/Linux/Programming/NES_EMU/c_src/cpu/cpu_basic_operations.c -o CMakeFiles/cpu.dir/cpu_basic_operations.c.s

cpu/CMakeFiles/cpu.dir/cpu_basic_operations.c.o.requires:
.PHONY : cpu/CMakeFiles/cpu.dir/cpu_basic_operations.c.o.requires

cpu/CMakeFiles/cpu.dir/cpu_basic_operations.c.o.provides: cpu/CMakeFiles/cpu.dir/cpu_basic_operations.c.o.requires
	$(MAKE) -f cpu/CMakeFiles/cpu.dir/build.make cpu/CMakeFiles/cpu.dir/cpu_basic_operations.c.o.provides.build
.PHONY : cpu/CMakeFiles/cpu.dir/cpu_basic_operations.c.o.provides

cpu/CMakeFiles/cpu.dir/cpu_basic_operations.c.o.provides.build: cpu/CMakeFiles/cpu.dir/cpu_basic_operations.c.o

cpu/CMakeFiles/cpu.dir/cpu.c.o: cpu/CMakeFiles/cpu.dir/flags.make
cpu/CMakeFiles/cpu.dir/cpu.c.o: cpu/cpu.c
	$(CMAKE_COMMAND) -E cmake_progress_report /mnt/Storage/Linux/Programming/NES_EMU/c_src/CMakeFiles $(CMAKE_PROGRESS_2)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object cpu/CMakeFiles/cpu.dir/cpu.c.o"
	cd /mnt/Storage/Linux/Programming/NES_EMU/c_src/cpu && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/cpu.dir/cpu.c.o   -c /mnt/Storage/Linux/Programming/NES_EMU/c_src/cpu/cpu.c

cpu/CMakeFiles/cpu.dir/cpu.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/cpu.dir/cpu.c.i"
	cd /mnt/Storage/Linux/Programming/NES_EMU/c_src/cpu && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /mnt/Storage/Linux/Programming/NES_EMU/c_src/cpu/cpu.c > CMakeFiles/cpu.dir/cpu.c.i

cpu/CMakeFiles/cpu.dir/cpu.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/cpu.dir/cpu.c.s"
	cd /mnt/Storage/Linux/Programming/NES_EMU/c_src/cpu && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /mnt/Storage/Linux/Programming/NES_EMU/c_src/cpu/cpu.c -o CMakeFiles/cpu.dir/cpu.c.s

cpu/CMakeFiles/cpu.dir/cpu.c.o.requires:
.PHONY : cpu/CMakeFiles/cpu.dir/cpu.c.o.requires

cpu/CMakeFiles/cpu.dir/cpu.c.o.provides: cpu/CMakeFiles/cpu.dir/cpu.c.o.requires
	$(MAKE) -f cpu/CMakeFiles/cpu.dir/build.make cpu/CMakeFiles/cpu.dir/cpu.c.o.provides.build
.PHONY : cpu/CMakeFiles/cpu.dir/cpu.c.o.provides

cpu/CMakeFiles/cpu.dir/cpu.c.o.provides.build: cpu/CMakeFiles/cpu.dir/cpu.c.o

cpu/CMakeFiles/cpu.dir/cpu_decode_logic.c.o: cpu/CMakeFiles/cpu.dir/flags.make
cpu/CMakeFiles/cpu.dir/cpu_decode_logic.c.o: cpu/cpu_decode_logic.c
	$(CMAKE_COMMAND) -E cmake_progress_report /mnt/Storage/Linux/Programming/NES_EMU/c_src/CMakeFiles $(CMAKE_PROGRESS_3)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object cpu/CMakeFiles/cpu.dir/cpu_decode_logic.c.o"
	cd /mnt/Storage/Linux/Programming/NES_EMU/c_src/cpu && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/cpu.dir/cpu_decode_logic.c.o   -c /mnt/Storage/Linux/Programming/NES_EMU/c_src/cpu/cpu_decode_logic.c

cpu/CMakeFiles/cpu.dir/cpu_decode_logic.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/cpu.dir/cpu_decode_logic.c.i"
	cd /mnt/Storage/Linux/Programming/NES_EMU/c_src/cpu && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /mnt/Storage/Linux/Programming/NES_EMU/c_src/cpu/cpu_decode_logic.c > CMakeFiles/cpu.dir/cpu_decode_logic.c.i

cpu/CMakeFiles/cpu.dir/cpu_decode_logic.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/cpu.dir/cpu_decode_logic.c.s"
	cd /mnt/Storage/Linux/Programming/NES_EMU/c_src/cpu && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /mnt/Storage/Linux/Programming/NES_EMU/c_src/cpu/cpu_decode_logic.c -o CMakeFiles/cpu.dir/cpu_decode_logic.c.s

cpu/CMakeFiles/cpu.dir/cpu_decode_logic.c.o.requires:
.PHONY : cpu/CMakeFiles/cpu.dir/cpu_decode_logic.c.o.requires

cpu/CMakeFiles/cpu.dir/cpu_decode_logic.c.o.provides: cpu/CMakeFiles/cpu.dir/cpu_decode_logic.c.o.requires
	$(MAKE) -f cpu/CMakeFiles/cpu.dir/build.make cpu/CMakeFiles/cpu.dir/cpu_decode_logic.c.o.provides.build
.PHONY : cpu/CMakeFiles/cpu.dir/cpu_decode_logic.c.o.provides

cpu/CMakeFiles/cpu.dir/cpu_decode_logic.c.o.provides.build: cpu/CMakeFiles/cpu.dir/cpu_decode_logic.c.o

cpu/CMakeFiles/cpu.dir/cpu_instructions.c.o: cpu/CMakeFiles/cpu.dir/flags.make
cpu/CMakeFiles/cpu.dir/cpu_instructions.c.o: cpu/cpu_instructions.c
	$(CMAKE_COMMAND) -E cmake_progress_report /mnt/Storage/Linux/Programming/NES_EMU/c_src/CMakeFiles $(CMAKE_PROGRESS_4)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object cpu/CMakeFiles/cpu.dir/cpu_instructions.c.o"
	cd /mnt/Storage/Linux/Programming/NES_EMU/c_src/cpu && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/cpu.dir/cpu_instructions.c.o   -c /mnt/Storage/Linux/Programming/NES_EMU/c_src/cpu/cpu_instructions.c

cpu/CMakeFiles/cpu.dir/cpu_instructions.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/cpu.dir/cpu_instructions.c.i"
	cd /mnt/Storage/Linux/Programming/NES_EMU/c_src/cpu && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /mnt/Storage/Linux/Programming/NES_EMU/c_src/cpu/cpu_instructions.c > CMakeFiles/cpu.dir/cpu_instructions.c.i

cpu/CMakeFiles/cpu.dir/cpu_instructions.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/cpu.dir/cpu_instructions.c.s"
	cd /mnt/Storage/Linux/Programming/NES_EMU/c_src/cpu && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /mnt/Storage/Linux/Programming/NES_EMU/c_src/cpu/cpu_instructions.c -o CMakeFiles/cpu.dir/cpu_instructions.c.s

cpu/CMakeFiles/cpu.dir/cpu_instructions.c.o.requires:
.PHONY : cpu/CMakeFiles/cpu.dir/cpu_instructions.c.o.requires

cpu/CMakeFiles/cpu.dir/cpu_instructions.c.o.provides: cpu/CMakeFiles/cpu.dir/cpu_instructions.c.o.requires
	$(MAKE) -f cpu/CMakeFiles/cpu.dir/build.make cpu/CMakeFiles/cpu.dir/cpu_instructions.c.o.provides.build
.PHONY : cpu/CMakeFiles/cpu.dir/cpu_instructions.c.o.provides

cpu/CMakeFiles/cpu.dir/cpu_instructions.c.o.provides.build: cpu/CMakeFiles/cpu.dir/cpu_instructions.c.o

# Object files for target cpu
cpu_OBJECTS = \
"CMakeFiles/cpu.dir/cpu_basic_operations.c.o" \
"CMakeFiles/cpu.dir/cpu.c.o" \
"CMakeFiles/cpu.dir/cpu_decode_logic.c.o" \
"CMakeFiles/cpu.dir/cpu_instructions.c.o"

# External object files for target cpu
cpu_EXTERNAL_OBJECTS =

cpu/libcpu.a: cpu/CMakeFiles/cpu.dir/cpu_basic_operations.c.o
cpu/libcpu.a: cpu/CMakeFiles/cpu.dir/cpu.c.o
cpu/libcpu.a: cpu/CMakeFiles/cpu.dir/cpu_decode_logic.c.o
cpu/libcpu.a: cpu/CMakeFiles/cpu.dir/cpu_instructions.c.o
cpu/libcpu.a: cpu/CMakeFiles/cpu.dir/build.make
cpu/libcpu.a: cpu/CMakeFiles/cpu.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C static library libcpu.a"
	cd /mnt/Storage/Linux/Programming/NES_EMU/c_src/cpu && $(CMAKE_COMMAND) -P CMakeFiles/cpu.dir/cmake_clean_target.cmake
	cd /mnt/Storage/Linux/Programming/NES_EMU/c_src/cpu && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/cpu.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
cpu/CMakeFiles/cpu.dir/build: cpu/libcpu.a
.PHONY : cpu/CMakeFiles/cpu.dir/build

cpu/CMakeFiles/cpu.dir/requires: cpu/CMakeFiles/cpu.dir/cpu_basic_operations.c.o.requires
cpu/CMakeFiles/cpu.dir/requires: cpu/CMakeFiles/cpu.dir/cpu.c.o.requires
cpu/CMakeFiles/cpu.dir/requires: cpu/CMakeFiles/cpu.dir/cpu_decode_logic.c.o.requires
cpu/CMakeFiles/cpu.dir/requires: cpu/CMakeFiles/cpu.dir/cpu_instructions.c.o.requires
.PHONY : cpu/CMakeFiles/cpu.dir/requires

cpu/CMakeFiles/cpu.dir/clean:
	cd /mnt/Storage/Linux/Programming/NES_EMU/c_src/cpu && $(CMAKE_COMMAND) -P CMakeFiles/cpu.dir/cmake_clean.cmake
.PHONY : cpu/CMakeFiles/cpu.dir/clean

cpu/CMakeFiles/cpu.dir/depend:
	cd /mnt/Storage/Linux/Programming/NES_EMU/c_src && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /mnt/Storage/Linux/Programming/NES_EMU/c_src /mnt/Storage/Linux/Programming/NES_EMU/c_src/cpu /mnt/Storage/Linux/Programming/NES_EMU/c_src /mnt/Storage/Linux/Programming/NES_EMU/c_src/cpu /mnt/Storage/Linux/Programming/NES_EMU/c_src/cpu/CMakeFiles/cpu.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : cpu/CMakeFiles/cpu.dir/depend

