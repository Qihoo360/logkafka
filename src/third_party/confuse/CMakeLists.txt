include_directories(src .)
aux_source_directory(src DIR_LIB_SRCS)
add_library (confuse ${DIR_LIB_SRCS})

set(package_version, '2.7')
add_definitions(-DPACKAGE_VERSION="${package_version}")
set(package_string, 'libConfuse 2.7')
add_definitions(-DPACKAGE_STRING="${package_string}")
add_definitions(-DHAVE_STRDUP)
add_definitions(-DHAVE_STRNDUP)

IF (OS_X_VERSION VERSION_GREATER "10.10")
    add_definitions(-DHAVE_UNISTD_H)
ENDIF (OS_X_VERSION VERSION_GREATER "10.10")
