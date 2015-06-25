MESSAGE(STATUS "Using bundled Findlibgcov.cmake...")
  FIND_PATH(
  LIBGCOV_INCLUDE_DIR
  /usr/include/ 
  /usr/local/include/ 
  )

FIND_LIBRARY(
  LIBGCOV_LIBRARIES NAMES libgcov.a gcov 
  PATHS /usr/lib/ /usr/local/lib/
  )
