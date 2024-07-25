# Primary CMake file for SDL_app
# This compiles all of the individual SDL components as an interface
if(POLICY CMP0091)
	set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)
endif()

if(POLICY CMP0077)
	set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
endif()

cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(VulkanSDL VERSION 1.0.0)

# Define the relative position of this file in the SDL App project
# This is to make this file easily relocatable (for Android, etc.)
set(CMAKE_CURRENT_DEPTH "${PROJECT_SOURCE_DIR}")

if(NOT DEFINED PROJECT_DIR_OFFSET)
	set(PROJECT_DIR_OFFSET "../..")
endif()

if(NOT DEFINED VULKAN_SDL_DIR)
	set(VULKAN_SDL_DIR "${CMAKE_CURRENT_DEPTH}/${PROJECT_DIR_OFFSET}")
endif()

set(SDL2_APP_DIR   "${VULKAN_SDL_DIR}")
set(SDL2_BASE_DIR  "${VULKAN_SDL_DIR}/components/SDL")
set(SDL2_IMAGE_DIR "${VULKAN_SDL_DIR}/components/SDL_image")
set(SDL2_TTF_DIR   "${VULKAN_SDL_DIR}/components/SDL_ttf")

# Resume the normal SDL CMake project

# Get the platform
include("${CMAKE_CURRENT_LIST_DIR}/modules/sdlplatform.cmake")
SDL_DetectCMakePlatform()


option(CMAKE_POSITION_INDEPENDENT_CODE "Build static libraries with -fPIC" ON)
option(VULKAN_SDL_BUILD_SHARED_LIBS  "Build all components as a shared library" ON)
set(BUILD_SHARED_LIBS ${VULKAN_SDL_BUILD_SHARED_LIBS})

set(COMPONENT_LIBS)

if(VULKAN_SDL_BUILD_SHARED_LIBS)
    set(vulkan_sdl_export_name VulkanSDL)
    set(vulkan_sdl_install_name_infix shared)
else()
    set(vulkan_sdl_export_name VulkanSDL-static)
    set(vulkan_sdl_install_name_infix static)
endif()

add_library(VulkanSDL INTERFACE)
add_library(VulkanSDL::${vulkan_sdl_export_name} ALIAS VulkanSDL)

## Base SDL
set(SDL_SHARED ${VULKAN_SDL_BUILD_SHARED_LIBS})
set(SDL_TEST   OFF)
add_subdirectory(sdl2)
if (VULKAN_SDL_BUILD_SHARED_LIBS)
	list(APPEND COMPONENT_LIBS SDL2)
else()
	list(APPEND COMPONENT_LIBS SDL2-static)
endif()
if (MSVC)
	list(APPEND COMPONENT_LIBS SDL2::SDL2main)
endif()

## SDL Image
set(SDL2IMAGE_BUILD_SHARED_LIBS ${VULKAN_SDL_BUILD_SHARED_LIBS})
set(SDL2IMAGE_INSTALL OFF)
set(SDL2IMAGE_VENDORED ON)
add_subdirectory(sdl2image)
if (VULKAN_SDL_BUILD_SHARED_LIBS)
	list(APPEND COMPONENT_LIBS SDL2_image::SDL2_image)
else()
	list(APPEND COMPONENT_LIBS SDL2_image::SDL2_image-static)
endif()

## SDL TTF
set(SDL2TTF_BUILD_SHARED_LIBS ${VULKAN_SDL_BUILD_SHARED_LIBS})
set(SDL2TTF_INSTALL OFF)
set(SDL2TTF_VENDORED ON)
add_subdirectory(sdl2ttf)
if (VULKAN_SDL_BUILD_SHARED_LIBS)
	list(APPEND COMPONENT_LIBS SDL2_ttf::SDL2_ttf)
else()
	list(APPEND COMPONENT_LIBS SDL2_ttf::SDL2_ttf-static)
endif()

## SDL APP
set(SDL2APP_BUILD_SHARED_LIBS ${VULKAN_SDL_BUILD_SHARED_LIBS})
add_subdirectory(sdl2app)
if (VULKAN_SDL_BUILD_SHARED_LIBS)
	list(APPEND COMPONENT_LIBS SDL2_app::SDL2_app)
else()
    list(APPEND COMPONENT_LIBS SDL2_app::SDL2_app-static)
endif()

## Put it all together
target_link_libraries(VulkanSDL INTERFACE ${COMPONENT_LIBS})

add_library(Vulkan::SDL INTERFACE IMPORTED GLOBAL)
set_target_properties(Vulkan::SDL PROPERTIES
    INTERFACE_LINK_LIBRARIES "VulkanSDL"
)
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.17")
    set_target_properties(Vulkan::SDL PROPERTIES
        DEPRECATION "Use VulkanSDL::VulkanSDL or VulkanSDL::VulkanSDL-static instead"
    )
endif()
