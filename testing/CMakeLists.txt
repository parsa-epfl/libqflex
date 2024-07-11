cmake_minimum_required(VERSION 3.14)
project(libqflex)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Find GLib package
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB REQUIRED glib-2.0)

# Include GLib directories
include_directories(${GLIB_INCLUDE_DIRS})

add_executable(
  compare-and-swap-pair-test ../libqflex/plugins/trace/memory-decoder.c  testing-utils.cc compare-and-swap-pair-test.cc
)
add_executable(
  load-register-literal-test ../libqflex/plugins/trace/memory-decoder.c testing-utils.cc load-register-literal-test.cc
)
add_executable(
  load-store-register-pair-test ../libqflex/plugins/trace/memory-decoder.c testing-utils.cc load-store-register-pair-test.cc
)

target_link_libraries(
  compare-and-swap-pair-test
  GTest::gtest_main
  ${GLIB_LIBRARIES}
)
target_link_libraries(
  load-register-literal-test
  GTest::gtest_main
  ${GLIB_LIBRARIES}
)
target_link_libraries(
  load-store-register-pair-test
  GTest::gtest_main
  ${GLIB_LIBRARIES}
)

include(GoogleTest)
gtest_discover_tests(compare-and-swap-pair-test)
gtest_discover_tests(load-register-literal-test)
gtest_discover_tests(load-store-register-pair-test)
