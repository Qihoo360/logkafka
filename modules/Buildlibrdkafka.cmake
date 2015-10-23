MESSAGE(STATUS "installing librdkafka ...")
CMAKE_MINIMUM_REQUIRED(VERSION 3.0.2)

ExternalProject_Add(project_librdkafka
    GIT_REPOSITORY https://github.com/edenhill/librdkafka.git
    GIT_TAG 0.8.6
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/librdkafka
    CONFIGURE_COMMAND cd <SOURCE_DIR> && ./configure --prefix=<INSTALL_DIR>
    BUILD_COMMAND cd <SOURCE_DIR> && make
    INSTALL_COMMAND cd <SOURCE_DIR> && make install
)

ExternalProject_Get_Property(project_librdkafka install_dir)

#add_library(librdkafka SHARED IMPORTED)
#set_property(TARGET librdkafka PROPERTY IMPORTED_LOCATION ${install_dir}/lib/librdkafka.so)
add_library(librdkafka STATIC IMPORTED)
set_property(TARGET librdkafka PROPERTY IMPORTED_LOCATION ${install_dir}/lib/librdkafka.a)
add_dependencies(librdkafka project_librdkafka)
ADD_DEPENDENCIES(logkafka_lib project_librdkafka)

TARGET_LINK_LIBRARIES(logkafka librdkafka)
INCLUDE_DIRECTORIES(${install_dir}/include)
