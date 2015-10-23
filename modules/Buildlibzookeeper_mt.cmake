MESSAGE(STATUS "installing libzookeeper_mt ...")
CMAKE_MINIMUM_REQUIRED(VERSION 3.0.2)

IF (OS_X_VERSION VERSION_EQUAL "10.10")
    MESSAGE(STATUS "downloading lib zookeeper patch for Mac OS X ${OS_X_VERSION} ...")
    #SET(ZOOKEEPER_PATCH_FILE ${CMAKE_CURRENT_BINARY_DIR}/libzookeeper_mt/ZOOKEEPER-2049.noprefix.trunk.patch)
    #SET(ZOOKEEPER_PATCH_URL https://issues.apache.org/jira/secure/attachment/12673212/ZOOKEEPER-2049.noprefix.trunk.patch)
    #EXECUTE_PROCESS(WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/libzookeeper_mt 
    #                COMMAND wget ${ZOOKEEPER_PATCH_URL} -O ${ZOOKEEPER_PATCH_FILE})
    SET(ZOOKEEPER_PATCH_FILE ${PROJECT_SOURCE_DIR}/patches/ZOOKEEPER-2049.noprefix.trunk.patch)
    SET(LIBZK_OSX_PATCH_CMD "patch -p0 -N -s < ${ZOOKEEPER_PATCH_FILE} || true")
ELSE (OS_X_VERSION VERSION_EQUAL "10.10")
    SET(LIBZK_OSX_PATCH_CMD ":") # do nothing
ENDIF (OS_X_VERSION VERSION_EQUAL "10.10")

ExternalProject_Add(project_libzookeeper_mt
    URL http://archive.apache.org/dist/zookeeper/zookeeper-3.4.6/zookeeper-3.4.6.tar.gz
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/libzookeeper_mt
    PATCH_COMMAND cd <SOURCE_DIR> && sh -c "${LIBZK_OSX_PATCH_CMD}"
    CONFIGURE_COMMAND cd <SOURCE_DIR>/src/c && ./configure --prefix=<INSTALL_DIR>
    BUILD_COMMAND cd <SOURCE_DIR>/src/c && make -j4
    INSTALL_COMMAND cd <SOURCE_DIR>/src/c && make install
)

ExternalProject_Get_Property(project_libzookeeper_mt install_dir)

add_library(libzookeeper_mt STATIC IMPORTED)
set_property(TARGET libzookeeper_mt PROPERTY IMPORTED_LOCATION ${install_dir}/lib/libzookeeper_mt.a)
add_dependencies(libzookeeper_mt project_libzookeeper_mt)
ADD_DEPENDENCIES(logkafka_lib project_libzookeeper_mt)

TARGET_LINK_LIBRARIES(logkafka libzookeeper_mt)
INCLUDE_DIRECTORIES(${install_dir}/include)
