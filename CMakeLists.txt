cmake_minimum_required(VERSION 3.10)

file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" CHANNEL_MAP_VERSION LIMIT_COUNT 1)
project(ChannelMap VERSION ${CHANNEL_MAP_VERSION} LANGUAGES CXX)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
message(STATUS "${PROJECT_NAME}: version ${PROJECT_VERSION}, build type ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR})
    set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()
set(CMAKE_BUILD_RPATH ${CMAKE_INSTALL_PREFIX}/lib)

add_library(channel-map SHARED
    src/calibration.cpp
    src/channel_map_dopeness.cpp
    src/channel_map_readCSV.cpp
    src/dictionary.cpp
    src/geometry.cpp
    src/inputNormalization.cpp
    src/item.cpp
    src/printerKeySpace.cpp
)
target_compile_features(channel-map PUBLIC cxx_std_17)
# CMake 3.23+ keeps these headers ahead of older system-installed copies.
set_target_properties(channel-map PROPERTIES IMPORTED_NO_SYSTEM TRUE)
target_include_directories(channel-map PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(channel-map PUBLIC stdc++fs)
endif()
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(channel-map PRIVATE DEBUG)
endif()

add_executable(dope-skeleton-program dope_skeleton.cpp)
target_link_libraries(dope-skeleton-program PRIVATE channel-map)

# Generate the package configuration here; no source template is required.
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/ChannelMapConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/ChannelMapConfig.cmake" [=[
include("${CMAKE_CURRENT_LIST_DIR}/ChannelMapTargets.cmake")
if (NOT TARGET ChannelMap::ChannelMap)
    add_library(ChannelMap::ChannelMap INTERFACE IMPORTED)
    set_target_properties(ChannelMap::ChannelMap PROPERTIES
        INTERFACE_LINK_LIBRARIES ChannelMap::channel-map
    )
endif()
get_target_property(ChannelMap_INCLUDE_DIRS ChannelMap::channel-map INTERFACE_INCLUDE_DIRECTORIES)
set(ChannelMap_LIBRARIES ChannelMap::ChannelMap)
]=])

install(TARGETS dope-skeleton-program RUNTIME DESTINATION bin)
install(TARGETS channel-map EXPORT ChannelMapTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY include/chmap DESTINATION include)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/ChannelMapConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/ChannelMapConfigVersion.cmake"
    DESTINATION lib/cmake/ChannelMap
)
install(EXPORT ChannelMapTargets
    FILE ChannelMapTargets.cmake
    NAMESPACE ChannelMap::
    DESTINATION lib/cmake/ChannelMap
)
