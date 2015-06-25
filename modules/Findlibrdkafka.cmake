MESSAGE(STATUS "Using bundled Findlibrdkafka.cmake...")
  FIND_PATH(
  LIBRDKAFKA_INCLUDE_DIR
  librdkafka/rdkafka.h 
  /usr/include/ 
  /usr/local/include/ 
  )

FIND_LIBRARY(
  LIBRDKAFKA_LIBRARIES NAMES librdkafka.a rdkafka
  PATHS /usr/lib/ /usr/local/lib/
  )
