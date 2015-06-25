MESSAGE(STATUS "Using bundled Findlibrt.cmake...")
  FIND_PATH(
  LIBRT_INCLUDE_DIR
  time.h 
  /usr/include/ 
  /usr/local/include/ 
  )

FIND_LIBRARY(
  LIBRT_LIBRARIES NAMES rt 
  PATHS /usr/lib/ /usr/local/lib/
  )
