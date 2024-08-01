# This file is to add source files and include directories
# into variables so that it can be reused from different repositories
# in their Cmake based build system by including this file.

# RTCP library source files.
file( GLOB RTCP_SOURCES
     "${CMAKE_CURRENT_LIST_DIR}/source/*.c" )

# RTCP library Public Include directories.
set( RTCP_INCLUDE_PUBLIC_DIRS
     "${CMAKE_CURRENT_LIST_DIR}/source/include" )

# RTCP library public include header files.
file( GLOB RTCP_INCLUDE_PUBLIC_FILES
     "${CMAKE_CURRENT_LIST_DIR}/source/include/*.h" )