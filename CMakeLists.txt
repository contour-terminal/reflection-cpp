cmake_minimum_required(VERSION 3.10)
project(reflection-cpp VERSION 0.4.0 LANGUAGES CXX)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(GNUInstallDirs)
include(ThirdParties)
include(ClangTidy)
include(PedanticCompiler)

set(reflection_cpp_HEADERS
    ${PROJECT_SOURCE_DIR}/include/reflection-cpp/reflection.hpp
)
add_library(reflection-cpp INTERFACE)
add_library(reflection-cpp::reflection-cpp ALIAS reflection-cpp)

target_compile_features(reflection-cpp INTERFACE cxx_std_20)
target_include_directories(reflection-cpp INTERFACE
    $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Generate the version, config and target files
include(CMakePackageConfigHelpers)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/reflection-cpp-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)
configure_package_config_file(reflection-cpp-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/reflection-cpp-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/reflection-cpp
)

install(FILES ${reflection_cpp_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/reflection-cpp
)
install(TARGETS reflection-cpp
    EXPORT reflection-cpp-targets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/reflection-cpp-config.cmake
            ${CMAKE_CURRENT_BINARY_DIR}/reflection-cpp-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/reflection-cpp
)
install(EXPORT reflection-cpp-targets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/reflection-cpp
    NAMESPACE reflection-cpp::
)

# ---------------------------------------------------------------------------
# unit tests

option(REFLECTION_TESTING "Enables building of unittests for reflection-cpp [default: OFF]" OFF)
if(REFLECTION_TESTING)
    find_package(Catch2 3.4.0 QUIET)
    if(NOT Catch2_FOUND)
        ThirdPartiesAdd_Catch2()
    endif()
    enable_testing()
    add_executable(test-reflection-cpp
        test-reflection-cpp.cpp
    )
    target_compile_features(test-reflection-cpp INTERFACE cxx_std_20)
    target_link_libraries(test-reflection-cpp reflection-cpp Catch2::Catch2 Catch2::Catch2WithMain)
    add_test(test-reflection-cpp ./test-reflection-cpp)
endif()
message(STATUS "[reflection-cpp] Compile unit tests: ${REFLECTION_TESTING}")
