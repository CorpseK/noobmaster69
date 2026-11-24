cmake_minimum_required(VERSION 3.24)

project(TNGPlayerOnly VERSION 0.2.0 LANGUAGES CXX)

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

add_library(${PROJECT_NAME} SHARED src/main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE CommonLibSSE::CommonLibSSE)
target_compile_options(${PROJECT_NAME} PRIVATE /utf-8 /permissive- /Zc:preprocessor)
target_compile_definitions(${PROJECT_NAME} PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
