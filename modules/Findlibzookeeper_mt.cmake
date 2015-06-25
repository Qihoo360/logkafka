MESSAGE(STATUS "Using bundled Findlibzookeeper_mt.cmake...")
  FIND_PATH(
  LIBZOOKEEPER_MT_INCLUDE_DIR
  zookeeper.h 
  $ENV{LIBZOOKEEPER_INCLUDE_PATH}
  /usr/include/zookeeper
  /usr/include/ 
  /usr/local/include/ 
  /usr/local/include/zookeeper
  /usr/local/zookeeper/include/zookeeper
  )

FIND_LIBRARY(
  LIBZOOKEEPER_MT_LIBRARIES NAMES libzookeeper_mt.a zookeeper_mt
  PATHS $ENV{LIBZOOKEEPER_LIB_PATH} /usr/lib/ /usr/local/lib/ 
  )
