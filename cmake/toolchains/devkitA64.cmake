if(NOT DEFINED ENV{DEVKITPRO})
	message(FATAL_ERROR "DEVKITPRO environment variable is not set.")
endif()

set(DEVKITPRO $ENV{DEVKITPRO})
set(CMAKE_SYSTEM_NAME NintendoSwitch)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Avoid link checks that require running binaries on the build host.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

if(EXISTS "${DEVKITPRO}/cmake/Switch.cmake")
	include("${DEVKITPRO}/cmake/Switch.cmake")
else()
	message(FATAL_ERROR "devkitPro Switch CMake toolchain not found at ${DEVKITPRO}/cmake/Switch.cmake")
endif()
