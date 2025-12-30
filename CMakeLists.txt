cmake_minimum_required(VERSION 3.10)

project(ChannelMap)

execute_process(
  COMMAND sh script/chmap-config --version
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE PROJECT_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

message(STATUS "${PROJECT_NAME}: version ${PROJECT_VERSION}")

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR})
    set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()

set(CMAKE_BUILD_RPATH ${CMAKE_INSTALL_PREFIX}/lib)



file(GLOB SOURCES "src/*.cpp")
file(GLOB HEADERS "include/*.hpp")

### main library
add_library(channel-map SHARED ${SOURCES})
target_include_directories(channel-map PUBLIC include)
target_include_directories(channel-map PRIVATE)

### filesystem
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(FILESYSTEM_LIB stdc++fs)
endif()

### script
set(SCRIPT_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/script/chmap-config)

### debug
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDEBUG)
    ### root for debug
    list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
    find_package(ROOT)
    ### measure search time
    add_executable(search-time debug/search-time.cpp)
    target_compile_features(search-time PRIVATE cxx_std_17)
    target_link_libraries(search-time PRIVATE channel-map ${FILESYSTEM_LIB}
    				      ROOT::Core ROOT::Hist)
    install(TARGETS search-time DESTINATION bin)
endif()

### config
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/ChannelMapConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)
configure_file(ChannelMapConfig.cmake.in ChannelMapConfig.cmake @ONLY)

### skeleton
add_executable(skeleton-program skeleton.cpp)
target_compile_features(skeleton-program PRIVATE cxx_std_17)
target_link_libraries(skeleton-program PRIVATE channel-map ${FILESYSTEM_LIB})

### install
install(DIRECTORY include/ DESTINATION include)
install(TARGETS skeleton-program DESTINATION bin)
install(TARGETS channel-map EXPORT ChannelMapTargets DESTINATION lib)
install(FILES ${HEADERS} DESTINATION include)
install(FILES ${SCRIPT_SOURCE} DESTINATION bin
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
	GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

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
