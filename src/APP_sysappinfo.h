/*
 * SDL_app:  An all-in-one library for packing SDL applications.
 * Copyright (C) 2022-2023 Walker M. White
 *
 * This library is built on the assumption that an application built for SDL
 * will contain its own versions of the SDL libraries (either statically linked
 * or packaged with a specific set of dynamic libraries). While this is not
 * considered the right way to do it on Unix, it makes one step installation
 * easier for Mac and Windows. It is also the only way to create SDL apps for
 * mobile devices.
 *
 * SDL License:
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */
#ifndef __APP_SYS_DISPLAY_H__
#define __APP_SYS_DISPLAY_H__
#include "SDL_app.h"

/**
 *  \file APP_sysappinfo.h
 *
 *  \brief Include file for application-specific information
 *  \author Walker M. White
 */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * System dependent version of APP_GetAppID
 */
extern const char* APP_SYS_GetAppID();

/**
 * System dependent version of APP_GetAssetPath
 */
extern const char* APP_SYS_GetAssetPath();

#ifdef __cplusplus
}
#endif

#endif /* __APP_SYS_DISPLAY_H__ */
