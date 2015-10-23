MESSAGE(STATUS "installing libuv ...")
CMAKE_MINIMUM_REQUIRED(VERSION 3.0.2)

ExternalProject_Add(project_libuv
    GIT_REPOSITORY https://github.com/libuv/libuv.git
    GIT_TAG v1.6.0
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/libuv
    CONFIGURE_COMMAND cd <SOURCE_DIR> && sh autogen.sh && ./configure --prefix=<INSTALL_DIR>
    BUILD_COMMAND cd <SOURCE_DIR> && make
    INSTALL_COMMAND cd <SOURCE_DIR> && make install
)

ExternalProject_Get_Property(project_libuv install_dir)

#add_library(libuv SHARED IMPORTED)
#set_property(TARGET libuv PROPERTY IMPORTED_LOCATION ${install_dir}/lib/libuv.so)
add_library(libuv STATIC IMPORTED)
set_property(TARGET libuv PROPERTY IMPORTED_LOCATION ${install_dir}/lib/libuv.a)
add_dependencies(libuv project_libuv)
ADD_DEPENDENCIES(logkafka_lib project_libuv)

TARGET_LINK_LIBRARIES(logkafka libuv)
INCLUDE_DIRECTORIES(${install_dir}/include)
