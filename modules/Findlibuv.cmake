MESSAGE(STATUS "Using bundled Findlibuv.cmake...")
  FIND_PATH(
  LIBUV_INCLUDE_DIR
  uv.h
  /usr/include/
  /usr/local/include/uv
  )

FIND_LIBRARY(
  LIBUV_LIBRARIES NAMES libuv.a uv
  PATHS /usr/lib /usr/local/lib
  )
