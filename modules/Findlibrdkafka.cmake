MESSAGE(STATUS "Using bundled Findlibrdkafka.cmake...")
  FIND_PATH(
  LIBRDKAFKA_INCLUDE_DIR
  librdkafka/rdkafka.h 
  /usr/include/ 
  /usr/local/include/ 
  /usr/local/opt/librdkafka/include/
  )

FIND_LIBRARY(
  LIBRDKAFKA_LIBRARIES NAMES librdkafka.a rdkafka
  PATHS /usr/lib/ /usr/local/lib/ /usr/local/opt/librdkafka/lib/
  )
