MESSAGE(STATUS "Using bundled Findlibpcre2.cmake...")
  FIND_PATH(
  LIBPCRE2_INCLUDE_DIR
  pcre2.h
  /usr/include/
  /usr/local/include
  /usr/local/include/pcre2
  )

FIND_LIBRARY(
  LIBPCRE2_LIBRARIES NAMES libpcre2-8.a pcre2
  PATHS /usr/lib /usr/local/lib
  )
