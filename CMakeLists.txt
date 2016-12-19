cmake_minimum_required( VERSION 3.0.2 )

#project name
get_filename_component( CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME )
project( ${CURRENT_DIR_NAME} )

if(CMAKE_COMPILER_IS_GNUCXX)
    add_compile_options(-std=c++11)
    add_compile_options(-fPIC)
else()
    add_definitions(-DQ_COMPILER_INITIALIZER_LISTS)
endif(CMAKE_COMPILER_IS_GNUCXX)

#find packages
find_package( Qt5Core          REQUIRED )
find_package( Qt5Network       REQUIRED )
find_package( Qt5Sql           REQUIRED )


#settings
set( ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}" )
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${ROOT_DIR}/build/cmake/" )
set( CURRENT_SRC_PATH "${ROOT_DIR}/src" )

if(UNIX)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g -O0")
endif(UNIX)

#includes
include( custom-tools )
include( cotire )

#load config
set( CONFIG_PATH "${ROOT_DIR}/config.in" )
set( CONFIG_TEMPLATE_PATH "${ROOT_DIR}/build/cmake/templates/config.in" )
load_config( ${CONFIG_PATH} ${CONFIG_TEMPLATE_PATH} )
message( "Platform found at: \"${ROOT_DIR}/${PLATFORM_PATH}\"" )

include_directories( "${CURRENT_SRC_PATH}" )
include_directories( "${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_PATH}" )
include_directories( "${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_PATH}/common" )
include_directories( "${CMAKE_CURRENT_LIST_DIR}/${PLATFORM_PATH}/3rdparty/fastcgi/include" )


#build file list
file( GLOB_RECURSE H_FILES "${CURRENT_SRC_PATH}/*.h" )
file( GLOB_RECURSE CPP_FILES "${CURRENT_SRC_PATH}/*.cpp" )

set( USER_FILES ${H_FILES} ${CPP_FILES} )

#wrapping
unset( MOC_LIST )
qt5_wrap_cpp( MOC_LIST ${H_FILES} OPTIONS "--no-notes" )

set( GENERATED_FILES ${MOC_LIST} )
set( ALL_FILES ${USER_FILES} ${GENERATED_FILES} )

apply_source_groups( USER_FILES ${CURRENT_SRC_PATH} "Sources" )
apply_source_groups( GENERATED_FILES ${CURRENT_SRC_PATH} "Generated" )

#compiling
add_executable( ${PROJECT_NAME} ${ALL_FILES} )

#modules
add_subdirectory( "${ROOT_DIR}/${PLATFORM_PATH}/server-core"        "${CMAKE_CURRENT_BINARY_DIR}/server-core_build" )
add_subdirectory( "${ROOT_DIR}/${PLATFORM_PATH}/database"           "${CMAKE_CURRENT_BINARY_DIR}/database_build" )
add_subdirectory( "${ROOT_DIR}/${PLATFORM_PATH}/network-core"       "${CMAKE_CURRENT_BINARY_DIR}/network-core_build" )
add_subdirectory( "${ROOT_DIR}/${PLATFORM_PATH}/web-exchange"       "${CMAKE_CURRENT_BINARY_DIR}/web-exchange_build" )
add_subdirectory( "${ROOT_DIR}/${PLATFORM_PATH}/utils"              "${CMAKE_CURRENT_BINARY_DIR}/utils_build" )

#linking
target_link_libraries( ${PROJECT_NAME} Qt5::Core )
target_link_libraries( ${PROJECT_NAME} Qt5::Network )
target_link_libraries( ${PROJECT_NAME} Qt5::Sql )

target_link_libraries( ${PROJECT_NAME} server-core )
target_link_libraries( ${PROJECT_NAME} database )
target_link_libraries( ${PROJECT_NAME} network-core )
target_link_libraries( ${PROJECT_NAME} web-exchange )
target_link_libraries( ${PROJECT_NAME} utils )

#external libraries
if (UNIX)
    target_link_libraries( ${PROJECT_NAME} "${ROOT_DIR}/${PLATFORM_PATH}/3rdParty/fastcgi/libfcgi/.libs/libfcgi.a" )
else (WIN32)
    if (${CMAKE_GENERATOR} STREQUAL "Visual Studio 12 2013 Win64")
        target_link_libraries( ${PROJECT_NAME} "${ROOT_DIR}/${PLATFORM_PATH}/3rdparty/fastcgi/libfcgi/Debug64/libfcgi.lib" )
        add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${ROOT_DIR}/${PLATFORM_PATH}/3rdparty/fastcgi/libfcgi/Debug64 $<TARGET_FILE_DIR:${PROJECT_NAME}>)
    else ()
        target_link_libraries( ${PROJECT_NAME} "${ROOT_DIR}/${PLATFORM_PATH}/3rdparty/fastcgi/libfcgi/Debug/libfcgi.lib" )
        add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${ROOT_DIR}/${PLATFORM_PATH}/3rdparty/fastcgi/libfcgi/Debug $<TARGET_FILE_DIR:${PROJECT_NAME}>)
    endif()
endif()

#cotire
set_target_properties( ${PROJECT_NAME} PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "src/Common.h" )
set_target_properties( ${PROJECT_NAME} PROPERTIES COTIRE_ADD_UNITY_BUILD FALSE )
cotire( ${PROJECT_NAME} )
