MESSAGE(STATUS "Using bundled Findlibpthread.cmake...")
  FIND_PATH(
  LIBPTHREAD_INCLUDE_DIR
  pthread.h 
  /usr/include/ 
  /usr/local/include/ 
  )

FIND_LIBRARY(
  LIBPTHREAD_LIBRARIES NAMES pthread 
  PATHS /usr/lib/ /usr/local/lib/
  )
