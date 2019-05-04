cmake_minimum_required(VERSION 3.5)
project(tun-open VERSION 1.0 LANGUAGES C)

option(TUN_OPEN_ENABLE_VERBOSE "Enable verbose error output")
option(TUN_OPEN_ENABLE_TEST "Build the test executable")


add_library(tun-open src/tun-open.c)
target_compile_features(tun-open PRIVATE c_std_99)
target_compile_options(tun-open PRIVATE -Wall -Wextra)
if (TUN_OPEN_ENABLE_TEST)
	target_compile_definitions(tun-open PRIVATE TUN_OPEN_VERBOSE=1)
endif()
target_include_directories(tun-open PUBLIC
	$<INSTALL_INTERFACE:inc>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>)


if (TUN_OPEN_ENABLE_TEST)
	add_executable(test-tun-open test/test-tun-open.c)
	target_link_libraries(test-tun-open PRIVATE tun-open)
	target_compile_features(test-tun-open PRIVATE c_std_99)
	target_compile_options(test-tun-open PRIVATE -Wall -Wextra)
endif()

