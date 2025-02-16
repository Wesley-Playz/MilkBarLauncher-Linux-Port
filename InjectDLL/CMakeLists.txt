cmake_minimum_required(VERSION 3.16)
project(InjectDLL VERSION 1.0 LANGUAGES CXX)

# Detect platform
if(WIN32)
    set(PLATFORM_WINDOWS TRUE)
elseif(UNIX)
    set(PLATFORM_LINUX TRUE)
endif()

# Source and header files
file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS include/*.h)

# Ensure at least one source file exists
if(NOT SOURCES)
    message(FATAL_ERROR "No source files found in src/")
endif()

# Define the target as a shared library (DLL)
add_library(InjectDLL SHARED ${SOURCES} ${HEADERS})

# Set C++ standard
set_target_properties(InjectDLL PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

# Define preprocessor definitions
target_compile_definitions(InjectDLL PRIVATE INJECTDLL_EXPORTS)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(InjectDLL PRIVATE _DEBUG)
else()
    target_compile_definitions(InjectDLL PRIVATE NDEBUG)
endif()

# Include directories
target_include_directories(InjectDLL PRIVATE include)

# Link system libraries
if(PLATFORM_WINDOWS)
    target_compile_definitions(InjectDLL PRIVATE _WINDOWS _USRDLL)
    target_link_libraries(InjectDLL PRIVATE 
        kernel32 user32 gdi32 winspool comdlg32 
        advapi32 shell32 ole32 oleaut32 uuid odbc32 odbccp32 ws2_32
    )
elseif(PLATFORM_LINUX)
    target_link_libraries(InjectDLL PRIVATE pthread dl)
endif()

# Set output directories
set_target_properties(InjectDLL PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# Precompiled headers (optional)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/pch.h")
    target_precompile_headers(InjectDLL PRIVATE "pch.h")
endif()
