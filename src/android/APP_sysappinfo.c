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
#include "../APP_sysdisplay.h"
#include <SDL_system.h>
#include <jni.h>

#define MAX_SIZE 1024

/**
 * System dependent version of APP_GetAppID
 * 
 * @return the application id as defined by the configuration file.
 */
const char* APP_SYS_GetAppID(void) {
    static char app_id[MAX_SIZE];

    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    jclass clazz = (*env)->GetObjectClass(env, activity);
    jmethodID method_id = (*env)->GetStaticMethodID(env, clazz, "getApplicationID", "()Ljava/lang/String;");
    jobject value = (*env)->CallStaticObjectMethod(env, clazz, method_id);
    const char *nativeString = (*env)->GetStringUTFChars(env, value, 0);

    size_t len = strlen(nativeString);
    if (len >= MAX_SIZE) { len = MAX_SIZE-1; }
    strncat(app_id, nativeString, len);

    // Clean up
    (*env)->ReleaseStringUTFChars(env, value, nativeString);
    (*env)->DeleteLocalRef(env, activity);
    (*env)->DeleteLocalRef(env, clazz);
    (*env)->DeleteLocalRef(env, value);

    return app_id;
}

/**
 * System dependent version of APP_GetAssetPath
 * 
 * @return the path to the application asset directory
 */
const char* APP_SYS_GetAssetPath() {
    return "";
}