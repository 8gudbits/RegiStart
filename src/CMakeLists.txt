# CMakeLists.txt for RegiStart
cmake_minimum_required(VERSION 3.10)
project(RegiStart VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /Zi")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -s")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
endif()

# Source files
set(SOURCES main.cpp)

# Create executable
add_executable(RegiStart ${SOURCES})

# Link libraries
target_link_libraries(RegiStart advapi32)

# Set properties for Windows
if(WIN32)
    target_compile_definitions(RegiStart PRIVATE _WIN32_WINNT=0x0600)
    set_target_properties(RegiStart PROPERTIES
        SUFFIX ".exe"
        WIN32_EXECUTABLE FALSE
    )
endif()

# Install target
install(TARGETS RegiStart
    RUNTIME DESTINATION bin
    COMPONENT applications
)

# Create a release package
set(CPACK_PACKAGE_NAME "RegiStart")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_VENDOR "8gudbits")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Windows Registry Auto-Start Program Manager")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "RegiStart")
set(CPACK_NSIS_DISPLAY_NAME "RegiStart")
set(CPACK_NSIS_PACKAGE_NAME "RegiStart")
include(CPack)

