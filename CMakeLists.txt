CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(logkafka)

SET (PROJECT_VERSION_MAJOR 1)
SET (PROJECT_VERSION_MINOR 0)
SET (PROJECT_VERSION_PATCH 0)
#define PROJECT_VERSION_MAJOR @PROJECT_VERSION_MAJOR@
#define PROJECT_VERSION_MINOR @PROJECT_VERSION_MINOR@
#define PROJECT_VERSION_PATCH @PROJECT_VERSION_PATCH@

###############################
# Initialize System Info 
###############################

IF (APPLE)
    IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        EXEC_PROGRAM(sw_vers ARGS -productVersion OUTPUT_VARIABLE OS_X_VERSION)
        STRING(REGEX MATCH "([0-9]+)\\.([0-9]+)" OS_X_VERSION ${OS_X_VERSION})
        MESSAGE(STATUS "OS_X_VERSION=${OS_X_VERSION}")
    ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
ENDIF(APPLE)

###############################
# Initialize CXXFLAGS
###############################
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support.")
endif()

###############################
# Building
###############################
SUBDIRS(${PROJECT_SOURCE_DIR}/src 
        ${PROJECT_SOURCE_DIR}/unittest)

IF (NOT DEFINED CMAKE_BINARY_DIR)
    SET(CMAKE_BINARY_DIR ${PROJECT_SOURCE_DIR}/_build)
ENDIF (NOT DEFINED CMAKE_BINARY_DIR)

SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# copy conf to binary dir ( only work for cmake >2.8 )
#FILE(COPY ${PROJECT_SOURCE_DIR}/conf 
#     DESTINATION ${CMAKE_BINARY_DIR}
#     PATTERN "CVS" EXCLUDE
#     PATTERN .svn EXCLUDE
#     PATTERN .git EXCLUDE)

################################
# copy macro for cmake 2.6 
################################
# Copy files from source directory to destination directory, substituting any
# variables.  Create destination directory if it does not exist.

macro(configure_files srcDir destDir)
    message(STATUS "Configuring directory ${destDir}")
    make_directory(${destDir})

    file(GLOB templateFiles RELATIVE ${srcDir} ${srcDir}/*)
    foreach(templateFile ${templateFiles})
        set(srcTemplatePath ${srcDir}/${templateFile})
        if(NOT IS_DIRECTORY ${srcTemplatePath})
            message(STATUS "Configuring file ${templateFile}")
            configure_file(
                    ${srcTemplatePath}
                    ${destDir}/${templateFile}
                    @ONLY)
        endif(NOT IS_DIRECTORY ${srcTemplatePath})
    endforeach(templateFile)
endmacro(configure_files)

# copy conf to binary dir ( for cmake 2.6 )
configure_files(${PROJECT_SOURCE_DIR}/conf ${CMAKE_BINARY_DIR}/conf)

################################
# Installing
################################

if (NOT DEFINED CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/_install)
endif (NOT DEFINED CMAKE_INSTALL_PREFIX)

INSTALL(FILES ${CMAKE_BINARY_DIR}/bin/logkafka DESTINATION bin
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)

INSTALL(DIRECTORY conf 
        DESTINATION .
        PATTERN "CVS" EXCLUDE
        PATTERN .svn EXCLUDE
        PATTERN .git EXCLUDE
        PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ)

INSTALL(DIRECTORY tools
        DESTINATION .
        PATTERN "CVS" EXCLUDE
        PATTERN .svn EXCLUDE
        PATTERN .git EXCLUDE
        PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ)

INSTALL(DIRECTORY DESTINATION logs)
INSTALL(DIRECTORY DESTINATION data)


################################
# CPack 
################################
INCLUDE (InstallRequiredSystemLibraries)
SET (CPACK_RESOURCE_FILE_LICENSE
      "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
SET (CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
SET (CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
SET (CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
INCLUDE (CPack)
