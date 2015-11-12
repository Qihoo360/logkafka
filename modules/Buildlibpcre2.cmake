MESSAGE(STATUS "installing libpcre2 ...")
CMAKE_MINIMUM_REQUIRED(VERSION 3.0.2)

ExternalProject_Add(project_libpcre2
    URL http://sourceforge.net/projects/pcre/files/pcre2/10.20/pcre2-10.20.tar.gz
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/libpcre2
    CONFIGURE_COMMAND cd <SOURCE_DIR> && ./configure --prefix=<INSTALL_DIR>
    BUILD_COMMAND cd <SOURCE_DIR> && make -j4
    INSTALL_COMMAND cd <SOURCE_DIR> && make install
)

ExternalProject_Get_Property(project_libpcre2 install_dir)

add_library(libpcre2 STATIC IMPORTED)
set_property(TARGET libpcre2 PROPERTY IMPORTED_LOCATION ${install_dir}/lib/libpcre2-8.a)
add_dependencies(libpcre2 project_libpcre2)
ADD_DEPENDENCIES(logkafka_lib project_libpcre2)

TARGET_LINK_LIBRARIES(logkafka libpcre2)
INCLUDE_DIRECTORIES(${install_dir}/include)
