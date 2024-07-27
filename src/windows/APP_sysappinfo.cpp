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
 #include "../APP_sysappinfo.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <atlstr.h>
#include <iostream>
#include <fstream>
#include <string>

// Index in our resource file
#define APPID_STRING 102

/**
 * System dependent version of APP_GetAppID
 */
const char* APP_SYS_GetAppID() {
    static std::string APPID;

    // First try the resource
    if (APPID.empty()) {
        CString resource;
        if (resource.LoadString(APPID_STRING)) {
            char* utf8 = SDL_iconv_string("UTF-8", "UTF-16LE",
                (char*)((LPCTSTR)resource),
                (SDL_wcslen((LPCTSTR)resource) + 1) * sizeof(WCHAR));

            APPID = utf8;

            SDL_free(utf8);
        }
    }

    // Now try the file
    if (APPID.empty()) {
        std::string path = std::string(SDL_GetBasePath())+"appinfo.id";
        std::ifstream file;
        file.open(path);
        if (file.is_open()) {
            file >> APPID;
        }
        file.close();
    }

    return APPID.empty() ? NULL : APPID.c_str();
}

/**
 * System dependent version of APP_GetAssetPath
 */
const char* APP_SYS_GetAssetPath() {
    static std::string ASSETDIR;
    if (IsDebuggerPresent()) {
        DWORD bufflen = GetCurrentDirectory(0, NULL)+1;
        WCHAR* path = (WCHAR*)SDL_malloc(bufflen * sizeof(WCHAR));
        GetCurrentDirectory(bufflen, path);

        char* utf8 = SDL_iconv_string("UTF-8", "UTF-16LE", (char*)(path), (SDL_wcslen(path) + 1) * sizeof(WCHAR));

        ASSETDIR = utf8;
        SDL_free(utf8);
        SDL_free(path);
    } else {
        char* path = SDL_GetBasePath();
        ASSETDIR = path;
        SDL_free(path);
    }

    return ASSETDIR.c_str();
}
