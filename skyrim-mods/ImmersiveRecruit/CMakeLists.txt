cmake_minimum_required(VERSION 3.24)

project(ImmersiveRecruit VERSION 1.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# --- CommonLibSSE-NG, maintained fork (alandtse/CommonLibVR, ng branch) -----
# The original CharmedBaryon repo stopped at 2024 and treats game 1.7.x as SE
# ("failed to open address library file"); this fork knows 1.7.x IDs and layouts.
include(FetchContent)
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ENABLE_SKYRIM_VR OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
	CommonLibSSE
	GIT_REPOSITORY https://github.com/alandtse/CommonLibVR.git
	GIT_TAG 7c27a75da3eb0db080c1f1f63f6df1b7cd733c2a # v16.12.0
)
FetchContent_MakeAvailable(CommonLibSSE)

add_library(${PROJECT_NAME} SHARED src/main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE CommonLibSSE::CommonLibSSE)
target_compile_options(${PROJECT_NAME} PRIVATE /utf-8 /permissive- /Zc:preprocessor)
target_compile_definitions(${PROJECT_NAME} PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
