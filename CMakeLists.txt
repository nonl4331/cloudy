cmake_minimum_required(VERSION 4.0)

project(cloudy VERSION 0.1.0 DESCRIPTION "Volumetric Path Tracer" LANGUAGES CXX)

add_library(cloudy_lib STATIC src/lib.cpp)
target_compile_features(cloudy_lib PUBLIC cxx_std_20)
target_compile_options(
    cloudy_lib
    INTERFACE
        -fstack-protector-strong
        -fstack-clash-protection
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wsign-conversion
        -Wcast-qual
        -Wformat=2
        -Wundef
        -Werror=float-equal
        -Wshadow
        -Wcast-align
        -Wunused
        -Wnull-dereference
        -Wdouble-promotion
        -Wimplicit-fallthrough
        -Wextra-semi
        -Woverloaded-virtual
        -Wnon-virtual-dtor
        -Wold-style-cast
)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(cloudy_lib PRIVATE fmt::fmt)

find_package(spdlog CONFIG REQUIRED)
target_link_libraries(cloudy_lib PRIVATE spdlog::spdlog)

find_package(embree CONFIG REQUIRED)
target_link_libraries(cloudy_lib PRIVATE embree)

include(FetchContent)
FetchContent_Declare(
	openvdb
	GIT_REPOSITORY https://github.com/AcademySoftwareFoundation/openvdb.git
	GIT_TAG        v13.0.0
	GIT_SHALLOW    TRUE
	EXCLUDE_FROM_ALL
	SOURCE_SUBDIR "nanovdb"
)
FetchContent_MakeAvailable(openvdb)
target_include_directories(cloudy_lib PRIVATE ${openvdb_SOURCE_DIR}/nanovdb)

find_path(TINYGLTF_INCLUDE_DIRS "tiny_gltf.h")
target_include_directories(cloudy_lib PRIVATE ${TINYGLTF_INCLUDE_DIRS})

add_executable(cloudy src/main.cpp)
target_link_libraries(cloudy PRIVATE cloudy_lib)
target_link_options(
    cloudy
    PRIVATE -Wl,--as-needed -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now
)

find_package(Stb MODULE REQUIRED)
target_include_directories(cloudy SYSTEM PRIVATE ${Stb_INCLUDE_DIR})

find_package(CLI11 CONFIG REQUIRED)
target_link_libraries(cloudy PRIVATE CLI11::CLI11)
