/**
 * A simple demo to verify that base SDL is working correctly
 *
 * Author: Walker M. White
 * Vesion: July 10, 2024
 */
#include <stdio.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_app.h>
#include <memory>
#include "extras/pattern.h"


#define WIDTH 1024
#define HEIGHT 576

#if defined __IPHONEOS__
    #define MOBILE_PLATFORM 1
#elif defined __ANDROID__
    #define MOBILE_PLATFORM 1
#endif

/**
 * Returns the string representation of the orientation
 */
const char* get_orientation(SDL_DisplayOrientation orient) {
	switch(orient) {
	case SDL_ORIENTATION_PORTRAIT:
		return "portrait";
	case SDL_ORIENTATION_PORTRAIT_FLIPPED:
		return "portrait flipped";
	case SDL_ORIENTATION_LANDSCAPE:
		return "landscape";
	case SDL_ORIENTATION_LANDSCAPE_FLIPPED:
		return "landscape flipped";
	case SDL_ORIENTATION_UNKNOWN:
		return "unknown";
	}
	return "unknown";
}

/**
 * A simple demo that draws a checker board pattern
 */
int main(int argc, char* argv[]) {
    SDL_Window *window;
    SDL_Texture* image;
    SDL_Texture* label;
    SDL_Surface* surface1;
    SDL_Surface* surface2;
    SDL_Renderer *renderer;
    SDL_Event event;

    int w = WIDTH;
    int h = HEIGHT;

    Uint32 flags;
#if defined MOBILE_PLATFORM
    bool fullscreen = true;
    bool useHighDPI = true;
#else
    bool fullscreen = false;
    bool useHighDPI = true;
#endif

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0){
        SDL_Log("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

	// Initialize the TTF library
    if ( TTF_Init() < 0 ) {
    	SDL_Log("Could not initialize TTF: %s",SDL_GetError());
        return 2;
    }

    SDL_DisplayMode mode;
    SDL_GetCurrentDisplayMode(0, &mode);


    flags = SDL_WINDOW_HIDDEN;
    if (fullscreen) {
        flags |= SDL_WINDOW_FULLSCREEN;
    }
    if (useHighDPI) {
        flags |= SDL_WINDOW_ALLOW_HIGHDPI;
    }

    window = SDL_CreateWindow("SDL Demo", 0, 0, w, h, flags);
    if (window == NULL) {
        SDL_Log("SDL_CreateWindow() failed: %s\n", SDL_GetError());
        return(2);
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        SDL_Log("SDL_CreateRenderer() failed: %s\n", SDL_GetError());
        return(2);
    }

    surface1 = IMG_Load("logo.png");
    if (surface1 == nullptr) {
    	SDL_Log("Failed to load image: %s\n", SDL_GetError());
        return 3;
    } else {
    	SDL_Log("Successfully loaded image");
    }
	SDL_Log("Image Source is (%d,%d)",surface1->w,surface1->h);

	TTF_Font* font = TTF_OpenFont("fonts/MarkerFelt.ttf", 64*(useHighDPI ? 2 : 1));
    if (font == nullptr) {
    	SDL_Log("Font initialization error: %s", TTF_GetError());
        return 2;
    }
    SDL_Color color;
    color.r = color.g = color.b = color.a = 255;
    surface2 = TTF_RenderText_Blended(font, "Hello World!", color);
	SDL_Log("Label Source is (%d,%d)",surface2->w,surface2->h);

    SDL_SetWindowTitle(window, "SDL Demo");
    SDL_SetWindowSize(window, w, h);
    SDL_SetWindowPosition(window, (int)((mode.w-w)/2.0f), (int)((mode.h-h)/2.0f));
    SDL_ShowWindow(window);

	SDL_Rect full;
	SDL_Rect disp;
	SDL_Rect safe;

	full.x = 0;
	full.y = 0;
	SDL_GetRendererOutputSize(renderer,&(full.w),&(full.h));
    SDL_Log("OpenGL is (%d,%d)-(%d,%d)", full.x,full.y,full.w,full.h);
	SDL_GetDisplayBounds(0,&disp);
    SDL_Log("SDL Display is (%d,%d)-(%d,%d)", disp.x,disp.y,disp.w,disp.h);
	APP_GetDisplayPixelBounds(0,&disp);
    SDL_Log("Pixel Display is (%d,%d)-(%d,%d)", disp.x,disp.y,disp.w,disp.h);
	SDL_GetDisplayUsableBounds(0,&safe);
    SDL_Log("Usable Display is (%d,%d)-(%d,%d)", safe.x,safe.y,safe.w,safe.h);
	APP_GetDisplaySafeBounds(0,&safe);
    SDL_Log("Safe is (%d,%d)-(%d,%d)", safe.x,safe.y,safe.w,safe.h);
    // For some reason, Android needs to call this twice
	//SDL_GetRendererOutputSize(renderer,&(full.w),&(full.h));
    //SDL_Log("OpenGL is (%d,%d)-(%d,%d)", full.x,full.y,full.w,full.h);
    if (!fullscreen) {
    	safe.x = full.x;
    	safe.y = full.y;
    	safe.w = full.w;
    	safe.h = full.h;
    } else {
		full.x = disp.x;
    	full.y = disp.y;
    	full.w = disp.w;
    	full.h = disp.h;
    }

	int notch = APP_CheckDisplayNotch(0);
    SDL_Log("Notch result is %d", notch);
    float density = APP_GetDisplayPixelDensity(0);
    SDL_Log("Pixel density is %f", density);

    SDL_DisplayOrientation orient;
    orient = APP_GetDeviceOrientation(0);
    SDL_Log("Device orientation is %s", get_orientation(orient));
    orient = SDL_GetDisplayOrientation(0);
    SDL_Log("Display orientation is %s", get_orientation(orient));
    orient = APP_GetDefaultOrientation(0);
    SDL_Log("Default orientation is %s", get_orientation(orient));

	// Center image in the safe area
    image = SDL_CreateTextureFromSurface(renderer, surface1);
    SDL_Rect impos;
#ifdef _WINDOWS
    impos.w = surface1->w / 2; impos.h = surface1->h / 2;
#else
    impos.w = surface1->w; impos.h = surface1->h;
#endif
    impos.x = (int)((safe.w- impos.w)/2.0f)+safe.x;
    impos.y = (int)((safe.h- impos.h)/4.0f)+safe.y;
	SDL_Log("Image is (%d,%d)-(%d,%d)",impos.x,impos.y,impos.w,impos.h);

	// Center text in the safe area
    label = SDL_CreateTextureFromSurface(renderer, surface2);
    SDL_Rect txpos;
#ifdef _WINDOWS
    txpos.w = surface2->w/2; txpos.h = surface2->h/2;
#else
    txpos.w = surface2->w; txpos.h = surface2->h;
#endif
    txpos.x = (int)((safe.w- txpos.w)/2.0f)+safe.x;
    txpos.y = (int)(4*(safe.h- txpos.h)/5.0f)+safe.y;
	SDL_Log("Label is (%d,%d)-(%d,%d)",txpos.x,txpos.y,txpos.w,txpos.h);

    SDL_Log("Name: %s",APP_GetDeviceName());
    SDL_Log("Model: %s",APP_GetDeviceModel());
    SDL_Log("OS: %s",APP_GetDeviceOS());
    SDL_Log("Version: %s",APP_GetDeviceOSVersion());
    SDL_Log("Vendor ID: %s",APP_GetDeviceID());

    int tsize = 64;
    SDL_Rect test;
    test.x = test.y = 0;
    test.w = test.h = tsize;
    bool running = true;
    while (running) {
        while ( SDL_PollEvent(&event) ) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;
                default:
                    break;
            }
        }

        drawgimp(renderer, full.w, full.h);

        // Show the corners
        test.x = safe.x;
        test.y = safe.y;
        SDL_SetRenderDrawColor(renderer,255,0,0,255);
        SDL_RenderFillRect(renderer,&test);

        test.y = safe.h+safe.y-tsize;
        SDL_SetRenderDrawColor(renderer,0,255,0,255);
        SDL_RenderFillRect(renderer,&test);

        test.x = safe.w+safe.x-tsize;
        SDL_SetRenderDrawColor(renderer,0,0,255,255);
        SDL_RenderFillRect(renderer,&test);

        test.y = safe.y;
        SDL_SetRenderDrawColor(renderer,255,255,255,255);
        SDL_RenderFillRect(renderer,&test);

        test.x = safe.x;

        SDL_RenderCopy(renderer, image, NULL, &impos);
        SDL_RenderCopy(renderer, label, NULL, &txpos);
        SDL_RenderPresent(renderer);
        SDL_Delay(100);
    }

    SDL_HideWindow(window);
    SDL_DestroyTexture(image);
    SDL_DestroyTexture(label);
    SDL_FreeSurface(surface1);
    SDL_FreeSurface(surface2);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
