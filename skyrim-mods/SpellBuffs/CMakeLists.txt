cmake_minimum_required(VERSION 3.24)

project(SpellBuffs VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# --- CommonLibSSE-NG (pinned commit) -------------------------------------
include(FetchContent)
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ENABLE_SKYRIM_VR OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
	CommonLibSSE
	GIT_REPOSITORY https://github.com/CharmedBaryon/CommonLibSSE-NG.git
	GIT_TAG b93280e832f263dbef44e44cbe2936622a02f91a
)
FetchContent_MakeAvailable(CommonLibSSE)

find_path(DETOURS_INCLUDE_DIRS "detours/detours.h")
# Pick the release/debug detours.lib explicitly: a plain find_library() can grab the
# debug build, which then fails to link against the release CRT (_CrtDbgReport).
find_library(DETOURS_LIBRARY_RELEASE NAMES detours
	PATHS "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib" NO_DEFAULT_PATH REQUIRED)
find_library(DETOURS_LIBRARY_DEBUG NAMES detours
	PATHS "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/lib" NO_DEFAULT_PATH)
if(DETOURS_LIBRARY_DEBUG)
	set(DETOURS_LIBRARY optimized ${DETOURS_LIBRARY_RELEASE} debug ${DETOURS_LIBRARY_DEBUG})
else()
	set(DETOURS_LIBRARY ${DETOURS_LIBRARY_RELEASE})
endif()

add_library(${PROJECT_NAME} SHARED src/main.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE ${DETOURS_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE CommonLibSSE::CommonLibSSE ${DETOURS_LIBRARY})
target_compile_options(${PROJECT_NAME} PRIVATE /utf-8 /permissive- /Zc:preprocessor)
target_compile_definitions(${PROJECT_NAME} PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
