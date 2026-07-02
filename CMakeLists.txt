cmake_minimum_required(VERSION 4.0)

project(cloudy VERSION 0.1.0 DESCRIPTION "Volumetric Path Tracer" LANGUAGES CXX)

add_executable(cloudy src/main.cpp)
target_sources(cloudy PUBLIC src/img.cpp src/coord.cpp src/cam.cpp src/sky.cpp)
target_compile_features(cloudy PUBLIC cxx_std_20)
target_link_options(
    cloudy
    PRIVATE -Wl,--as-needed -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now
)
target_compile_options(
    cloudy
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
target_link_libraries(cloudy PRIVATE fmt::fmt)

find_package(spdlog CONFIG REQUIRED)
target_link_libraries(cloudy PRIVATE spdlog::spdlog)

find_package(embree CONFIG REQUIRED)
target_link_libraries(cloudy PRIVATE embree)

include(FetchContent)
FetchContent_Declare(
	nanovdb
	GIT_REPOSITORY https://github.com/AcademySoftwareFoundation/openvdb.git
	GIT_TAG        v13.0.0
	GIT_SHALLOW    TRUE
	EXCLUDE_FROM_ALL
	SOURCE_SUBDIR "nanovdb"
)
FetchContent_MakeAvailable(nanovdb)
target_include_directories(cloudy PRIVATE ${nanovdb_SOURCE_DIR}/nanovdb)

find_package(OpenVDB CONFIG REQUIRED)
target_link_libraries(cloudy PRIVATE OpenVDB::openvdb)

find_path(TINYGLTF_INCLUDE_DIRS "tiny_gltf.h")
target_include_directories(cloudy PRIVATE ${TINYGLTF_INCLUDE_DIRS})

find_package(Stb MODULE REQUIRED)
target_include_directories(cloudy SYSTEM PRIVATE ${Stb_INCLUDE_DIR})

find_package(OpenEXR CONFIG REQUIRED)
target_link_libraries(cloudy PRIVATE OpenEXR::OpenEXR)

find_package(CLI11 CONFIG REQUIRED)
target_link_libraries(cloudy PRIVATE CLI11::CLI11)
