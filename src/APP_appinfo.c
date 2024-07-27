/*
 * SDL_app:  An all-in-one library for packing SDL applications.
 * Copyright (C) 2022-2023 Walker M. White
 *
 * This library is built on the assumption that an application built for SDL
 * will contain its own versions of the SDL libraries (either statically linked
 * or packaged with a specific set of dynamic libraries).  While this is not
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
#include "SDL_app.h"
#include "APP_sysappinfo.h"

/**
 * Returns the application id as defined by the configuration file.
 *
 * The app id has many incarnations. It is the bundle identifier for iOS
 * and macOS apps. It is the package name for Android apps. This function
 * provides a uniform way of accessing this value.
 *
 * Almost all applications built by SDL_app will have such an identifier.
 * On Windows apps, it is stored in the application resource file. As a
 * fallback, all command line applications (including Linux) will store
 * it in a file called appid.info in the asset directory. However, if the
 * platform does not have a native interpretation of appid, and the file
 * appid.info cannot be found, this function returns NULL.
 *
 * @return the application id as defined by the configuration file.
 */
const char* APP_GetAppID(void) {
    return APP_SYS_GetAppID();
}

/**
 * Returns the path to the application asset directory
 *
 * This function is essentially the same as SDL_GetBasePath with two minor
 * changes, both designed to make that function a little easier to work
 * with. The first difference is on Android. For that platform, this
 * function returns the empty string instead of NULL, simplifying path
 * concatentation.
 *
 * But the biggest change is on Windows. We do not want to have to copy
 * the asset files into the build folder if we are using the Visual Studio
 * debugger. So this function returns the debugger's working directory if
 * it is active. Otherwise it returns SDL_GetBasePath().
 *
 * @return the path to the application asset directory
 */
const char* APP_GetAssetPath() {
    return APP_SYS_GetAssetPath();
}

/**
 * Returns the version of the given SDL dependency
 *
 * This allows the program to query the versions of the various libaries that
 * SDL_app depends on.
 *
 * @param dep   The library dependency
 *
 * @return the version of the given SDL dependency
 */
const char* APP_GetVersion(APP_Dependency dep) {
    switch (dep) {
        case APP_DEPENDENCY_SDL:
            return "2.30.5";
        case APP_DEPENDENCY_IMG:
            return "2.8.2";
        case APP_DEPENDENCY_TTF:
            return "2.22.0";
        case APP_DEPENDENCY_ATK: 	// NO AUDIO TOOLS IN THE VULKAN RELEASE
            return "UNSUPPORTED";
        case APP_DEPENDENCY_APP:
            return "2.3.0";
        default:
            break;
    }
    return "";
}
