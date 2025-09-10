cmake_minimum_required(VERSION 3.8)

# Enable hot reload for MSVC compilers if supported
if (POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project("Simple-LISP-Compiler"
        VERSION 0.1.0
        DESCRIPTION "A simple LISP compiler"
        LANGUAGES CXX)

# Collect all source files
file(GLOB_RECURSE SOURCES
     "src/*.cpp"
)

# Set include directories
include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/include"
)

# Create the executable
add_executable(slisp ${SOURCES})

# Set C++ standard
set_target_properties(slisp PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Set output directory
set_target_properties(slisp PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Installation instructions
install(TARGETS slisp
        RUNTIME DESTINATION bin)

# Copy test files to the binary directory for easy access during testing
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/tests/"
     DESTINATION "${CMAKE_BINARY_DIR}/bin/tests")
