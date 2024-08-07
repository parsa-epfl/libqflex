cmake_minimum_required(VERSION 3.14)
project(libqflex)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
  DOWNLOAD_EXTRACT_TIMESTAMP CMP0135
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

add_definitions(-DTESTING)

include(GoogleTest)

set(EXECUTABLES
  compare-and-swap-pair
  load-register-literal
  load-store-register-pair-post-indexed
  load-store-register-unscaled-immediate
  load-store-register-immediate-post-indexed
)

foreach(EXEC ${EXECUTABLES})
  add_executable(${EXEC}
    ../libqflex/plugins/trace/memory-decoder.c
    ../libqflex/plugins/trace/branch-decoder.c
    testing-utils.cpp
    ${EXEC}.cpp
  )

  target_link_libraries(${EXEC}
    GTest::gtest_main
    ${GLIB_LIBRARIES}
  )

  gtest_discover_tests(${EXEC})
endforeach()
