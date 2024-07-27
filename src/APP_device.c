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
#include "APP_sysdevice.h"

/**
 * Returns the name of this device
 *
 * Typically this resolves to the user-visible name of this host (e.g. phone
 * or computer name). However, this is not guaranteed. In some cases it may
 * resolve to the name of the volume on which this application is being run.
 * In other cases (iPhone) it may simply resolve to the device model, in
 * compliance with Apple's privacy model.
 *
 * The string returned has static lifetime and does not need to be freed.
 *
 * @return the name of this device
 */
const char* APP_GetDeviceName(void) {
    return APP_SYS_GetDeviceName();
}

/**
 * Returns the model of this device
 *
 * This is device specific representation of the hardware model running this
 * application. On Unix systems, this is the equivalent of HW_MODEL passed
 * to sysctl. For mobile devices it is the vendor model. Note that the result
 * may be a code internal to the vendor (e.g. iPhone15,3).
 *
 * If the value cannot be determined, this returns "UNKNOWN".
 *
 * The string returned has static lifetime and does not need to be freed.
 *
 * @return the model of this device
 */
extern DECLSPEC const char* APP_GetDeviceModel(void) {
    return APP_SYS_GetDeviceModel();
}

/**
 * Returns the operating system running this device
 *
 * This value returned will be a generic name like "Windows" or "macOS". For
 * the version, use {@link APP_GetDeviceOSVersion}. If the value cannot be 
 * determined, this returns "UNKNOWN".
 *
 * The string returned has static lifetime and does not need to be freed.
 *
 * @return the operating system running this device
 */
extern DECLSPEC const char* APP_GetDeviceOS(void) {
    return APP_SYS_GetDeviceOS();
}

/**
 * Returns the operating system version of this device
 *
 * The value returned will typically be an identification number (in string
 * form) such as "13.6.5". However, the exact representation is platform
 * dependent. It the version cannot be determined, this function will return
 * "UNKNOWN".
 *
 * The string returned has static lifetime and does not need to be freed.
 *
 * @return the operating system version of this device
 */
extern DECLSPEC const char* APP_GetDeviceOSVersion(void) {
    return APP_SYS_GetDeviceOSVersion();
}

/**
 * Returns a unique identifier for this device
 *
 * The value returned will be what is known as a "vendor id". This means that
 * it is a pseudo-serial number that can be used to identify the device across
 * across multiple sessions. However, it is not (necessarily) a true serial
 * number. On some platforms (e.g. iOS) it is an identifier limited to
 * applications deployed by your developer account, and cannot be used to
 * identify a device cross-vendor. Furthermore, on some platforms it can be
 * changed by reseting the device to factory settings.
 *
 * There is no specific format for the string returned, as it varies from
 * platform to platform. For example, on iOS it is formal Version 4 UUID.
 * However, on Android this value is a 16 digit hexadecimal string. If it is
 * not possible to obtain a unique identifier, this function returns the empty
 * string.
 *
 * The string returned has static lifetime and does not need to be freed.
 *
 * @return a unique identifier for this device
 */
extern DECLSPEC const char* APP_GetDeviceID(void)  {
    return APP_SYS_GetDeviceID();
}
