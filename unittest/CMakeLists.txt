# Options. Turn on with 'cmake -Dmyvarname=ON'.
OPTION(test "Build all tests." OFF) # Makes boolean 'test' available.

################################
# Testing
################################
IF (test)
  # Set compile flags
  SET(GCC_COVERAGE_COMPILE_FLAGS "-fprofile-arcs -ftest-coverage")
  SET(GCC_COVERAGE_LINK_FLAGS    "-lgcov")
  SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )
  SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}" )

  # This adds another subdirectory, which has 'project(gtest)'.
  ADD_SUBDIRECTORY(lib/gtest-1.6.0)

  ENABLE_TESTING()

  # Include the gtest library. gtest_SOURCE_DIR is available due to
  # 'project(gtest)' above.
  INCLUDE_DIRECTORIES(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
  INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src)
  INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/third_party)
  INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/third_party/confuse/src)
  INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/third_party/rapidjson/include)
  INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/third_party/tclap/include)

  ##############
  # Unit Tests
  ##############
  FILE(GLOB DIR_SRCS ${PROJECT_SOURCE_DIR}/src/base/*
      ${PROJECT_SOURCE_DIR}/src/logkafka/*)
  LIST(REMOVE_ITEM DIR_SRCS ${PROJECT_SOURCE_DIR}/src/logkafka/main.cc) # remove "main.cc" from "*.cc" file list
  AUX_SOURCE_DIRECTORY(./src DIR_TEST_SRCS)
  ADD_EXECUTABLE(runUnitTests ${DIR_TEST_SRCS} ${DIR_SRCS})

  # Standard linking to gtest stuff.
  TARGET_LINK_LIBRARIES(runUnitTests gtest gtest_main)

  # Extra linking for the project.
  #TARGET_LINK_LIBRARIES(runUnitTests logkafka_lib)
  TARGET_LINK_LIBRARIES(runUnitTests confuse)
  
  # Extra linking for the gcov.
  SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/modules)
  IF(CMAKE_COMPILER_IS_GNUCXX)
      INCLUDE(CodeCoverage)
      setup_target_for_coverage(${PROJECT_NAME}_coverage runUnitTests coverage)
      # TARGET_LINK_LIBRARIES(${PROJECT_TEST_NAME} gcov)
  ENDIF()


  # This is so you can do 'make test' to see all your tests run, instead of
  # manually running the executable runUnitTests to see those specific tests.
  ADD_TEST(NAME that-test-I-made COMMAND runUnitTests)

  # You can also omit NAME and COMMAND. The second argument could be some other
  # test executable.
  ADD_TEST(that-other-test-I-made runUnitTests)

  ##############
  # Modules
  ##############
  SET(CMAKE_MODULE_PATH ${CMAKE_ROOT}/Modules ${PROJECT_SOURCE_DIR}/modules) 

  # gcov 
  FIND_PACKAGE( libgcov REQUIRED)
  MARK_AS_ADVANCED(
  LIBGCOV_INCLUDE_DIR
  LIBGCOV_LIBRARIES
  )
  IF (LIBGCOV_INCLUDE_DIR AND LIBGCOV_LIBRARIES)
  MESSAGE(STATUS "Found libgcov libraries")
     INCLUDE_DIRECTORIES(${LIBGCOV_INCLUDE_DIR})
      MESSAGE( ${LIBGCOV_LIBRARIES} )
      TARGET_LINK_LIBRARIES(runUnitTests ${LIBGCOV_LIBRARIES} )
  ENDIF (LIBGCOV_INCLUDE_DIR AND LIBGCOV_LIBRARIES)

   
  # rdkafka
  IF (NOT INSTALL_LIBRDKAFKA)
  
      FIND_PACKAGE( librdkafka REQUIRED)
      MARK_AS_ADVANCED(
      LIBRDKAFKA_INCLUDE_DIR
      LIBRDKAFKA_LIBRARIES
      )
      IF (LIBRDKAFKA_INCLUDE_DIR AND LIBRDKAFKA_LIBRARIES)
      MESSAGE(STATUS "Found librdkafka libraries")
         INCLUDE_DIRECTORIES(${LIBRDKAFKA_INCLUDE_DIR})
          MESSAGE( ${LIBRDKAFKA_LIBRARIES} )
          TARGET_LINK_LIBRARIES(runUnitTests ${LIBRDKAFKA_LIBRARIES} )
      ELSE (LIBRDKAFKA_INCLUDE_DIR AND LIBRDKAFKA_LIBRARIES)
          MESSAGE(FATAL_ERROR "Failed to find librdkafka libraries")
      ENDIF (LIBRDKAFKA_INCLUDE_DIR AND LIBRDKAFKA_LIBRARIES)
  
  ENDIF (NOT INSTALL_LIBRDKAFKA)
  
  # zookeeper
  IF (NOT INSTALL_LIBZOOKEEPER_MT)
  
      FIND_PACKAGE( libzookeeper_mt REQUIRED)
      MARK_AS_ADVANCED(
      LIBZOOKEEPER_MT_INCLUDE_DIR
      LIBZOOKEEPER_MT_LIBRARIES
      )
      IF (LIBZOOKEEPER_MT_INCLUDE_DIR AND LIBZOOKEEPER_MT_LIBRARIES)
      MESSAGE(STATUS "Found libzookeeper_mt libraries")
         INCLUDE_DIRECTORIES(${LIBZOOKEEPER_MT_INCLUDE_DIR})
          MESSAGE( ${LIBZOOKEEPER_MT_LIBRARIES} )
          TARGET_LINK_LIBRARIES(runUnitTests ${LIBZOOKEEPER_MT_LIBRARIES} )
      ELSE (LIBZOOKEEPER_MT_INCLUDE_DIR AND LIBZOOKEEPER_MT_LIBRARIES)
          MESSAGE(FATAL_ERROR "Failed to find libzookeeper_mt libraries")
      ENDIF (LIBZOOKEEPER_MT_INCLUDE_DIR AND LIBZOOKEEPER_MT_LIBRARIES)
  
  ENDIF (NOT INSTALL_LIBZOOKEEPER_MT)

  # uv
  IF (NOT INSTALL_LIBUV)
  
      FIND_PACKAGE( libuv REQUIRED)
      MARK_AS_ADVANCED(
      LIBUV_INCLUDE_DIR
      LIBUV_LIBRARIES
      )
      IF (LIBUV_INCLUDE_DIR AND LIBUV_LIBRARIES)
      MESSAGE(STATUS "Found libuv libraries")
         INCLUDE_DIRECTORIES(${LIBUV_INCLUDE_DIR})
          MESSAGE( ${LIBUV_LIBRARIES} )
          MESSAGE( ${LIBUV_INCLUDE_DIR} )
          TARGET_LINK_LIBRARIES(runUnitTests ${LIBUV_LIBRARIES} )
      ELSE (LIBUV_INCLUDE_DIR AND LIBUV_LIBRARIES)
          MESSAGE(FATAL_ERROR "Failed to find libuv libraries")
      ENDIF (LIBUV_INCLUDE_DIR AND LIBUV_LIBRARIES)
  
  ENDIF (NOT INSTALL_LIBUV)
  
  # pthread
  FIND_PACKAGE( libpthread REQUIRED)
  MARK_AS_ADVANCED(
  LIBPTHREAD_INCLUDE_DIR
  LIBPTHREAD_LIBRARIES
  )
  IF (LIBPTHREAD_INCLUDE_DIR AND LIBPTHREAD_LIBRARIES)
  MESSAGE(STATUS "Found libpthread libraries")
     INCLUDE_DIRECTORIES(${LIBPTHREAD_INCLUDE_DIR})
      MESSAGE( ${LIBPTHREAD_LIBRARIES} )
      TARGET_LINK_LIBRARIES(runUnitTests ${LIBPTHREAD_LIBRARIES} )
  ELSE (LIBPTHREAD_INCLUDE_DIR AND LIBPTHREAD_LIBRARIES)
      MESSAGE(FATAL_ERROR "Failed to find libpthread libraries")
  ENDIF (LIBPTHREAD_INCLUDE_DIR AND LIBPTHREAD_LIBRARIES)
  
  # rt ( libkafka )
  FIND_PACKAGE( librt REQUIRED)
  MARK_AS_ADVANCED(
  LIBRT_INCLUDE_DIR
  LIBRT_LIBRARIES
  )
  IF (LIBRT_INCLUDE_DIR AND LIBRT_LIBRARIES)
  MESSAGE(STATUS "Found librt libraries")
     INCLUDE_DIRECTORIES(${LIBRT_INCLUDE_DIR})
      MESSAGE( ${LIBRT_LIBRARIES} )
      TARGET_LINK_LIBRARIES(runUnitTests ${LIBRT_LIBRARIES} )
  ELSE (LIBRT_INCLUDE_DIR AND LIBRT_LIBRARIES)
      MESSAGE(FATAL_ERROR "Failed to find librt libraries")
  ENDIF (LIBRT_INCLUDE_DIR AND LIBRT_LIBRARIES)
  
  # z ( libkafka )
  FIND_PACKAGE( libz REQUIRED)
  MARK_AS_ADVANCED(
  LIBZ_INCLUDE_DIR
  LIBZ_LIBRARIES
  )
  IF (LIBZ_INCLUDE_DIR AND LIBZ_LIBRARIES)
  MESSAGE(STATUS "Found libz libraries")
     INCLUDE_DIRECTORIES(${LIBZ_INCLUDE_DIR})
      MESSAGE( ${LIBZ_LIBRARIES} )
      TARGET_LINK_LIBRARIES(runUnitTests ${LIBZ_LIBRARIES} )
  ELSE (LIBZ_INCLUDE_DIR AND LIBZ_LIBRARIES)
      MESSAGE(${LIBZ_LIBRARIES})
      MESSAGE(${LIBZ_INCLUDE_DIR})
      MESSAGE(FATAL_ERROR "Failed to find libz libraries")
  ENDIF (LIBZ_INCLUDE_DIR AND LIBZ_LIBRARIES)

endif()
