cmake_minimum_required(VERSION 3.6.3)
project(kvsrtcp C)

include(rtcpFilePaths.cmake)

add_library(kvsrtcp ${RTCP_SOURCES})

target_include_directories(kvsrtcp PUBLIC
                           ${RTCP_INCLUDE_PUBLIC_DIRS})

# install header files
install(
    FILES ${RTCP_INCLUDE_PUBLIC_FILES}
    DESTINATION include/kvsrtcp)

# install RTCP library
install(
    TARGETS kvsrtcp
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")