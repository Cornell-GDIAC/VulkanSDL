//
//  image.h
//  A simple function for loading images in SDL
//
//  The header stb_image used by the tutorial is not portable, as it cannot
//  be used on Android (again, no file system). Instead, in SDL you should use
//  SDL_Image (which also seems to produce better quality images for JPEGs).
//  This function shows how to do this.
//
//  Author:  Walker White
//  Version: 7/26/24.
//

#ifndef __IMAGE_H__
#define __IMAGE_H__
#include <SDL_image.h>
#include <string>
#include <cstring>
#include <cstdint>

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
    const char* base = SDL_GetBasePath();
    std::string fullpath = base == NULL ? path : std::string(base)+path;
    
    SDL_Surface* surface = IMG_Load(fullpath.c_str());
    if (surface == NULL) {
        return NULL;
    }
    
    // Convert the surface to RGBA, paying attention to endianness
    SDL_Surface* normal;
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
    normal = SDL_ConvertSurfaceFormat(surface,SDL_PIXELFORMAT_ABGR8888,0);
#else
    normal = SDL_ConvertSurfaceFormat(surface,SDL_PIXELFORMAT_RGBA8888,0);
#endif
    if (normal == NULL) {
        SDL_FreeSurface(surface);
        return NULL;
    }
    
    // Technically we can just return the surface, which is what we normally do
    // But we are trying to keep the code close to the tutorial.
    uint8_t* result = (uint8_t*)malloc(sizeof(uint8_t)*normal->w*normal->h);
    if (result != NULL) {
        memcpy(result,normal->pixels,sizeof(uint8_t)*normal->w*normal->h*4);
        if (w != NULL) {
            *w = normal->w;
        }
        if (h != NULL) {
            *h = normal->h;
        }
    }
    
    SDL_FreeSurface(normal);
    SDL_FreeSurface(surface);
    
    return result;
}


#endif /* __IMAGE_H__ */
