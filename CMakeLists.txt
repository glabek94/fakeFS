cmake_minimum_required(VERSION 3.9)
project(fakeFS C)

set(CMAKE_C_STANDARD 99)
find_package(PkgConfig)
pkg_check_modules(FUSE "fuse")

add_definitions(${FUSE_CFLAGS})
add_executable(fakeFS main.c structs.h fuseFcns.c fuseFcns.h diskFcns.c diskFcns.h)
target_link_libraries(fakeFS ${FUSE_LDFLAGS})