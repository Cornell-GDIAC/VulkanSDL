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
#include "string.h"

#define MAX_SIZE 1024
#define MAX_PATH 4096 // Linux max

/**
 * System dependent version of APP_GetAppID
 * 
 * @return the application id as defined by the configuration file.
 */
const char* APP_SYS_GetAppID(void) {
    static char app_id[MAX_SIZE];

	char* base = SDL_GetBasePath();
	size_t len = strlen(base);
	char* path = (char*)malloc((len+11)*sizeof(char));

	strcpy(path,base);
	strcpy(path+len,"appid.info");
	path[len+10] = 0;
	
	SDL_RWops* file = SDL_RWFromFile(path,"r");
	SDL_free(base);
	free(path);
	
	if (file == NULL) {
		return NULL;
	}
	
	size_t read = SDL_RWread(file, app_id, sizeof(char), MAX_SIZE-1);
	SDL_RWclose(file);
	
	if (read) {
		app_id[read] = 0;
		return app_id;
	}
	
	return NULL;
}

/**
 * System dependent version of APP_GetAssetPath
 * 
 * @return the path to the application asset directory
 */
const char* APP_SYS_GetAssetPath(void) {
	static char asset_path[MAX_PATH];
	
	char* path = SDL_GetBasePath();
	size_t amt = strlen(path);
	if (amt >= MAX_PATH) {
		amt = MAX_PATH-1;
	}
	
	strncpy(asset_path,path,amt);
	asset_path[amt] = 0;
	SDL_free(path);
	
    return asset_path;
}
