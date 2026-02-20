//
//  image.h
//  A simple function for loading images in SDL
//
//  The header stb_image used by the tutorial is a bit limiting. SDL_Image
//  provides a much wider array of potential file formats (and can also 
//  produce better looking JPEGs on certain platforms). This function shows
//  how to do this.
//
//  Author:  Walker White
//  Version: 7/26/24.
//

#ifndef __IMAGE_H__
#define __IMAGE_H__
#include <SDL3/SDL_image.h>
#include <SDL3/SDL_app.h>
#include <string>
#include <cstring>
#include <cstdint>

/**
 * Returns the absolute path to the given asset.
 *
 * This function allows us to use the asset/bundle directory on most devices,
 * but switch to the working directory in Windows for better Visual Studio
 * support
 *
 * @param asset The asset name
 *
 * @return the absolute path to the given asset.
 */
std::string get_asset(const std::string& asset) {
#if defined (SDL_PLATFORM_WINDOWS)
    char* path = SDL_GetCurrentDirectory();
    std::string result = std::string(path)+asset;
    SDL_free(path);
# else
    std::string result = std::string(SDL_GetBasePath())+asset;
#endif
    return result;
}


/**
 * Returns an array of pixels representing an RGBA image.
 *
 * This function assumes that path is a path to a file relative to the asset
 * directory (mobile devices do not allow external access to files).
 *
 * Upon success, the width and height will be stored in the provided pointers.
 * The returned array is size w*h*4 where each byte represents a color channel
 * value. It is the responsibility of the caller to free the memory returned.
 *
 * If the image cannot be loaded, this function returns NULL and the pointers
 * w and h are not updated.
 *
 * @param path  The path to the image in the asset directory
 * @param w     Pointer to store the image width
 * @param h     Pointer to store the image height
 *
 * @return an array of pixels representing an RGBA image.
 */
uint8_t* load_image_asset(const std::string path, int* w, int* h) {
	std::string fullpath = get_asset(path);
    SDL_Surface* surface = IMG_Load(fullpath.c_str());
    if (surface == NULL) {
        return NULL;
    }
    
    // Convert the surface to RGBA, paying attention to endianness
    SDL_Surface* normal = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
    SDL_DestroySurface(surface);
    if (normal == nullptr) {
        SDL_Log("Could not process file %s. %s", path.c_str(), SDL_GetError());
        return NULL;
    }

    // Technically we can just return the surface, which is what we normally do
    // But we are trying to keep the code close to the tutorial.
    uint8_t* result = (uint8_t*)malloc(sizeof(uint8_t)*normal->w*normal->h*4);
    if (result != NULL) {
        memcpy(result,normal->pixels,sizeof(uint8_t)*normal->w*normal->h*4);
        if (w != NULL) {
            *w = normal->w;
        }
        if (h != NULL) {
            *h = normal->h;
        }
    }
    
    SDL_DestroySurface(normal);
    return result;
}


#endif /* __IMAGE_H__ */
