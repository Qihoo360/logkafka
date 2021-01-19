MESSAGE(STATUS "Using bundled Findlibz.cmake...")
  FIND_PATH(
  LIBZ_INCLUDE_DIR
  zlib.h 
  /usr/include/ 
  /usr/local/include/ 
  /usr/local/opt/zlib/include/
  )

FIND_LIBRARY(
  LIBZ_LIBRARIES NAMES z 
  PATHS /usr/lib/ /usr/local/lib/ /usr/local/opt/zlib/lib
  )
