# Options. Turn on with 'cmake -Dmyvarname=ON'.
OPTION(test "Build all tests." OFF) # Makes boolean 'test' available.

################################
# Testing
################################
IF (test)
  # Set compile flags
  SET(GCC_COVERAGE_COMPILE_FLAGS "-fprofile-arcs -ftest-coverage")
  SET(GCC_COVERAGE_LINK_FLAGS    "-lgcov")
  SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
  SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")
  ADD_DEFINITIONS(-DGTEST_HAS_TR1_TUPLE=0)

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

  # pcre2
  IF (NOT INSTALL_LIBPCRE2)
  
      FIND_PACKAGE( libpcre2 REQUIRED)
      MARK_AS_ADVANCED(
      LIBPCRE2_INCLUDE_DIR
      LIBPCRE2_LIBRARIES
      )
      IF (LIBPCRE2_INCLUDE_DIR AND LIBPCRE2_LIBRARIES)
      MESSAGE(STATUS "Found libpcre2 libraries")
         INCLUDE_DIRECTORIES(${LIBPCRE2_INCLUDE_DIR})
          MESSAGE( ${LIBPCRE2_LIBRARIES} )
          MESSAGE( ${LIBPCRE2_INCLUDE_DIR} )
          TARGET_LINK_LIBRARIES(runUnitTests ${LIBPCRE2_LIBRARIES} )
      ELSE (LIBPCRE2_INCLUDE_DIR AND LIBPCRE2_LIBRARIES)
          MESSAGE(FATAL_ERROR "Failed to find libpcre2 libraries")
      ENDIF (LIBPCRE2_INCLUDE_DIR AND LIBPCRE2_LIBRARIES)
  
  ENDIF (NOT INSTALL_LIBPCRE2)
  
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
  IF (UNIX AND NOT APPLE)
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
  ELSEIF (WIN32)
    # Windows-specific includes or actions
  ELSEIF (APPLE)
    # ...
  ENDIF (UNIX AND NOT APPLE)
  
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

  ################################
  # External Projects
  ################################
  include(ExternalProject)
  
  # librdkafka 
  IF (INSTALL_LIBRDKAFKA)
  
      MESSAGE(STATUS "installing librdkafka ...")
      CMAKE_MINIMUM_REQUIRED(VERSION 3.0.2)
  
      ExternalProject_Get_Property(project_librdkafka install_dir)
      add_library(librdkafka STATIC IMPORTED)
      set_property(TARGET librdkafka PROPERTY IMPORTED_LOCATION ${install_dir}/lib/librdkafka.a)

      ADD_DEPENDENCIES(runUnitTests project_librdkafka)
  
      TARGET_LINK_LIBRARIES(runUnitTests librdkafka)
      INCLUDE_DIRECTORIES(${install_dir}/include)
  
  ENDIF (INSTALL_LIBRDKAFKA)
  
  # libzookeeper_mt 
  IF (INSTALL_LIBZOOKEEPER_MT)
  
      MESSAGE(STATUS "installing libzookeeper_mt ...")
      CMAKE_MINIMUM_REQUIRED(VERSION 3.0.2)
  
      ExternalProject_Get_Property(project_libzookeeper_mt install_dir)
      add_library(libzookeeper_mt STATIC IMPORTED)
      set_property(TARGET libzookeeper_mt PROPERTY IMPORTED_LOCATION ${install_dir}/lib/libzookeeper_mt.a)


      ADD_DEPENDENCIES(runUnitTests project_libzookeeper_mt)
  
      TARGET_LINK_LIBRARIES(runUnitTests libzookeeper_mt)
      INCLUDE_DIRECTORIES(${install_dir}/include)
  
  ENDIF (INSTALL_LIBZOOKEEPER_MT)
  
  # libuv 
  IF (INSTALL_LIBUV)
  
      MESSAGE(STATUS "installing libuv ...")
      CMAKE_MINIMUM_REQUIRED(VERSION 3.0.2)
  
      ExternalProject_Get_Property(project_libuv install_dir)
      add_library(libuv STATIC IMPORTED)
      set_property(TARGET libuv PROPERTY IMPORTED_LOCATION ${install_dir}/lib/libuv.a)

      ADD_DEPENDENCIES(runUnitTests project_libuv)
  
      TARGET_LINK_LIBRARIES(runUnitTests libuv)
      INCLUDE_DIRECTORIES(${install_dir}/include)
  
  ENDIF (INSTALL_LIBUV)

  # libpcre2 
  IF (INSTALL_LIBPCRE2)
  
      MESSAGE(STATUS "installing libpcre2 ...")
      CMAKE_MINIMUM_REQUIRED(VERSION 3.0.2)
  
      ExternalProject_Get_Property(project_libpcre2 install_dir)
      add_library(libpcre2 STATIC IMPORTED)
      set_property(TARGET libpcre2 PROPERTY IMPORTED_LOCATION ${install_dir}/lib/libpcre2-8.a)

      ADD_DEPENDENCIES(runUnitTests project_libpcre2)
  
      TARGET_LINK_LIBRARIES(runUnitTests libpcre2)
      INCLUDE_DIRECTORIES(${install_dir}/include)
  
  ENDIF (INSTALL_LIBPCRE2)

endif()
