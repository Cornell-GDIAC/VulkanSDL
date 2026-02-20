/**
 * A simple demo to verify that base SDL is working correctly
 *
 * Author: Walker M. White
 * Vesion: Feb 19, 2026
 */
#define SDL_MAIN_USE_CALLBACKS 1 /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_image.h>
#include <SDL3/SDL_ttf.h>
#include <SDL3/SDL_app.h>
#include <string>
#include <stdio.h>
#include <memory>
#include "extras/pattern.h"

#define WIDTH 1024
#define HEIGHT 576

#if defined SDL_PLATFORM_IOS && SDL_PLATFORM_IOS == 1
#include <TargetConditionals.h>
#if TARGET_OS_MACCATALYST
#else
    #define MOBILE_PLATFORM 1
#endif
#elif defined SDL_PLATFORM_ANDROID
    #define MOBILE_PLATFORM 1
#endif

/** The struct for defining the application state */
typedef struct
{
    SDL_DisplayID display;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_DisplayOrientation deviceOrientation;
    SDL_DisplayOrientation configOrientation;
    SDL_DisplayOrientation windowOrientation;
    SDL_Rect full;
    SDL_Rect safe;
    SDL_Texture* image;
    SDL_Texture* label;
    SDL_FRect impos;
    SDL_FRect txpos;
    Uint64 phase1;
    Uint64 phase2;
    Uint64 phase3;
    long gap;
} AppState;

/** Application metadata */
static const struct
{
    const char *key;
    const char *value;
} extended_metadata[] =
{
    { SDL_PROP_APP_METADATA_URL_STRING, "https://gdiac.cs.cornell.edu" },
    { SDL_PROP_APP_METADATA_CREATOR_STRING, "Cornell GDIAC" },
    { SDL_PROP_APP_METADATA_COPYRIGHT_STRING, "MIT License" },
    { SDL_PROP_APP_METADATA_TYPE_STRING, "game" }
};

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

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    size_t i;

    if (!SDL_SetAppMetadata("SDL Basic Demo", "1.0", "edu.cornell.gdiac.SDLDemo")) {
        SDL_Log("SDL_AppInit: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    for (i = 0; i < SDL_arraysize(extended_metadata); i++) {
        if (!SDL_SetAppMetadataProperty(extended_metadata[i].key, extended_metadata[i].value)) {
            SDL_Log("SDL_AppInit: %s\n", SDL_GetError());
            return SDL_APP_FAILURE;
        }
    }

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_Log("SDL_AppInit: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    
    // Initialize the TTF library
    if (!TTF_Init()) {
        SDL_Log("Could not initialize TTF: %s",SDL_GetError());
        return SDL_APP_FAILURE;
    }
    
    AppState *as = (AppState *)SDL_calloc(1, sizeof(AppState));
    if (!as) {
        SDL_Log("SDL_AppInit: Out of Memory\n");
        return SDL_APP_FAILURE;
    }

    *appstate = as;

    Uint32 flags;
#if defined MOBILE_PLATFORM
	SDL_Log("ALERT: Detected mobile platform");
    bool fullscreen = true;
    bool useHighDPI = true;
#else
    bool fullscreen = false;
    bool useHighDPI = true;
#endif

    flags = SDL_WINDOW_HIDDEN;
        if (fullscreen) {
    	SDL_Log("ALERT: Going Fullscreen");
        flags |= SDL_WINDOW_FULLSCREEN;
    }
    if (useHighDPI) {
        flags |= SDL_WINDOW_HIGH_PIXEL_DENSITY;
    }
    
    int w = WIDTH;
    int h = HEIGHT;

    SDL_DisplayID display = 0;

    int count = 0;
    SDL_DisplayID* displays = SDL_GetDisplays(&count);
    SDL_Log("Found %d displays.\n",count);
    if (count > 0) {
        display = displays[0];
        SDL_free(displays);
    }
        
    const SDL_DisplayMode* mode = SDL_GetCurrentDisplayMode(display);
    if (mode == NULL) {
        SDL_Log("SDL_GetCurrentDisplayMode() failed: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    
    SDL_Log("Display id is %d",display);

    SDL_DisplayOrientation orient;
    orient = APP_GetDisplayOrientation(display);
    as->windowOrientation = orient;
    
    if (orient == SDL_ORIENTATION_PORTRAIT || orient == SDL_ORIENTATION_PORTRAIT_FLIPPED) {
    	// This will cause a flip on Android if not careful
        h = WIDTH;
    	w = HEIGHT;
    }
    
    as->window = SDL_CreateWindow("SDL Demo", w, h, flags);
    if (as->window == NULL) {
        SDL_Log("SDL_CreateWindow() failed: %s\n", SDL_GetError());
        SDL_free(as);
        return SDL_APP_FAILURE;
    }
    
    as->renderer = SDL_CreateRenderer(as->window, NULL);
    if (as->renderer == NULL) {
        SDL_Log("SDL_CreateRenderer() failed: %s\n", SDL_GetError());
        SDL_free(as);
        return SDL_APP_FAILURE;
    }

    SDL_SetWindowTitle(as->window, "SDL Demo");
    SDL_SetWindowSize(as->window, w, h);
    SDL_SetWindowPosition(as->window, (int)((mode->w-w)/2.0f), (int)((mode->h-h)/2.0f));
    SDL_ShowWindow(as->window);

    SDL_Rect disp;

    as->full.x = 0;
    as->full.y = 0;
    SDL_GetCurrentRenderOutputSize(as->renderer,&(as->full.w),&(as->full.h));
    SDL_Log("Renderer is (%d,%d)-(%d,%d)", as->full.x,as->full.y,as->full.w,as->full.h);
    SDL_GetDisplayBounds(display,&disp);
    SDL_GetWindowSize(as->window,&(as->full.w),&(as->full.h));
    SDL_Log("Window size in points is (%d,%d)-(%d,%d)", as->full.x,as->full.y,as->full.w,as->full.h);
    SDL_GetDisplayBounds(display,&disp);
    SDL_GetWindowSizeInPixels(as->window,&(as->full.w),&(as->full.h));
    SDL_Log("Window size in pixels is (%d,%d)-(%d,%d)", as->full.x,as->full.y,as->full.w,as->full.h);
    SDL_GetDisplayBounds(display,&disp);
    SDL_Log("SDL Display is (%d,%d)-(%d,%d)", disp.x,disp.y,disp.w,disp.h);
    SDL_GetDisplayUsableBounds(display,&disp);
    SDL_Log("Usable Display is (%d,%d)-(%d,%d)", disp.x,disp.y,disp.w,disp.h);
    SDL_GetWindowSafeArea(as->window,&disp);
    SDL_Log("Safe Window is (%d,%d)-(%d,%d)", disp.x,disp.y,disp.w,disp.h);
    APP_GetWindowSafeAreaInPixels(as->window,&disp);
    SDL_Log("Safe Window is (%d,%d)-(%d,%d)", disp.x,disp.y,disp.w,disp.h);
    as->safe = disp;
    as->display = display;
    if (!fullscreen) {
        as->safe = as->full;
    }

	orient = APP_GetDeviceOrientation();
    as->deviceOrientation = orient;
    SDL_Log("Device orientation is %s", get_orientation(orient));
    orient = APP_GetDisplayConfiguration(display);
    as->windowOrientation = orient;
    SDL_Log("Configuration orientation is %s", get_orientation(orient));
    orient = APP_GetDisplayOrientation(display);
    as->configOrientation = orient;
    SDL_Log("Window orientation is %s", get_orientation(orient));
    
    bool notch = APP_CheckDisplayNotch(display);
    SDL_Log("Notch result is %s", notch ? "yes" : "no");
    float density = SDL_GetDisplayContentScale(display);
    SDL_Log("Display density is %f", density);
    density = SDL_GetWindowDisplayScale(as->window);
    SDL_Log("Window density is %f", density);

#ifdef SDL_PLATFORM_WINDOWS
    const char* bpath = SDL_GetCurrentDirectory();
#else
    const char* bpath = SDL_GetBasePath();
#endif
	if (bpath == NULL) {
		bpath = "";
	}
    std::string assets = bpath;
    std::string path = assets+"logo.png";

    // Load an image
    SDL_Surface* surface1 = IMG_Load(path.c_str());
    if (surface1 == nullptr) {
        SDL_Log("Failed to load image: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    } else {
        SDL_Log("Successfully loaded image");
    }
    SDL_Log("Image Source is (%d,%d)",surface1->w,surface1->h);
    
    // Center image in the safe area
    as->image = SDL_CreateTextureFromSurface(as->renderer, surface1);
#ifdef SDL_PLATFORM_WINDOWS
    as->impos.w = surface1->w / 2; as->impos.h = surface1->h / 2;
#else
    as->impos.w = surface1->w; as->impos.h = surface1->h;
#endif
    as->impos.x = (as->safe.w- as->impos.w)/2.0f+as->safe.x;
    as->impos.y = (as->safe.h- as->impos.h)/4.0f+as->safe.y;
    SDL_Log("Image is (%0.f,%0.f)-(%0.f,%0.f)",
            as->impos.x,as->impos.y,as->impos.w,as->impos.h);
    
    SDL_DestroySurface(surface1);
   
    // Create a label
    path = assets+"fonts/MarkerFelt.ttf";
    TTF_Font* font = TTF_OpenFont(path.c_str(), 64*(useHighDPI ? 2 : 1));
    if (font == nullptr) {
        SDL_Log("Font initialization error: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    } else {
        SDL_Log("Successfully loaded label");
    }
    SDL_Color color;
    color.r = color.g = color.b = color.a = 255;
    SDL_Surface* surface2 = TTF_RenderText_Blended(font, "Hello World!", 0, color);
    SDL_Log("Label Source is (%d,%d)",surface2->w,surface2->h);
    
    // Center text in the safe area
    as->label = SDL_CreateTextureFromSurface(as->renderer, surface2);
#ifdef SDL_PLATFORM_WINDOWS
    as->txpos.w = surface2->w/2; as->txpos.h = surface2->h/2;
#else
    as->txpos.w = surface2->w; as->txpos.h = surface2->h;
#endif
    as->txpos.x = (int)((as->safe.w- as->txpos.w)/2.0f)+as->safe.x;
    as->txpos.y = (int)(4*(as->safe.h- as->txpos.h)/5.0f)+as->safe.y;
    SDL_Log("Label is (%0.f,%0.f)-(%0.f,%0.f)",
            as->txpos.x,as->txpos.y,as->txpos.w,as->txpos.h);

    SDL_DestroySurface(surface2);
    
    as->phase1 = SDL_GetTicksNS() / 1000;
    as->phase2 = SDL_GetTicksNS() / 1000;

    SDL_Log("Name: %s",APP_GetDeviceName());
    SDL_Log("Model: %s",APP_GetDeviceModel());
    SDL_Log("OS: %s",APP_GetDeviceOS());
    SDL_Log("Version: %s",APP_GetDeviceOSVersion());
    SDL_Log("Vendor ID: %s",APP_GetDeviceID());

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    AppState *as = (AppState *)appstate;
    
    as->phase3 = SDL_GetTicksNS() / 1000;
    
    int tsize = 64;
    SDL_FRect test;
    test.x = test.y = 0;
    test.w = test.h = tsize;
    
    long gap1 = 0;
    long gap2 = 0;
    long gap3 = 0;

    // See if we need to update the safe area
    // This is common on Android devices as they re-layout the window
    SDL_Rect disp;
    APP_GetWindowSafeAreaInPixels(as->window,&disp);
    if (disp.w != as->safe.w || disp.h != as->safe.h || disp.x != as->safe.x || disp.y != as->safe.y) {
        SDL_Log("Safe update to (%d,%d)-(%d,%d)", disp.x,disp.y,disp.w,disp.h);
        as->safe = disp;
        int nw, nh;
	    SDL_GetWindowSizeInPixels(as->window,&nw,&nh);
        SDL_Log("Window size is now (%d,%d)", nw,nh);

		// Update the full area
	    SDL_GetCurrentRenderOutputSize(as->renderer,&(as->full.w),&(as->full.h));
		
		// Update the objects
		as->impos.x = (as->safe.w- as->impos.w)/2.0f+as->safe.x;
		as->impos.y = (as->safe.h- as->impos.h)/4.0f+as->safe.y;
		as->txpos.x = (int)((as->safe.w- as->txpos.w)/2.0f)+as->safe.x;
		as->txpos.y = (int)(4*(as->safe.h- as->txpos.h)/5.0f)+as->safe.y;
    }
    
    SDL_DisplayOrientation orient;
    orient = APP_GetDisplayOrientation(as->display);
    if (orient != as->windowOrientation) {
        SDL_Log("Display orientation is now %s", get_orientation(orient));
        as->windowOrientation = orient;
    }

    orient = APP_GetDisplayConfiguration(as->display);
    if (orient != as->configOrientation) {
        SDL_Log("Configuration orientation is now %s", get_orientation(orient));
        as->configOrientation = orient;
    }

    orient = APP_GetDeviceOrientation();
    if (orient != as->deviceOrientation) {
        SDL_Log("Device orientation is now %s", get_orientation(orient));
        as->deviceOrientation = orient;
    }

    drawgimp(as->renderer, as->full.w, as->full.h);
    
    // Show the corners
    test.x = as->safe.x;
    test.y = as->safe.y;
    SDL_SetRenderDrawColor(as->renderer,255,0,0,255);
    SDL_RenderFillRect(as->renderer,&test);
    
    test.y = as->safe.h+as->safe.y-tsize;
    SDL_SetRenderDrawColor(as->renderer,0,255,0,255);
    SDL_RenderFillRect(as->renderer,&test);
    
    test.x = as->safe.w+as->safe.x-tsize;
    SDL_SetRenderDrawColor(as->renderer,0,0,255,255);
    SDL_RenderFillRect(as->renderer,&test);
    
    test.y = as->safe.y;
    SDL_SetRenderDrawColor(as->renderer,255,255,255,255);
    SDL_RenderFillRect(as->renderer,&test);
    
    test.x = as->safe.x;
        
    SDL_RenderTexture(as->renderer, as->image, NULL, &(as->impos));
    SDL_RenderTexture(as->renderer, as->label, NULL, &(as->txpos));

    SDL_RenderPresent(as->renderer);

    gap1 = (long)(as->phase3 - as->phase2);

    as->phase1 = SDL_GetTicksNS() / 1000;
    gap2 = (long)(as->phase1 - as->phase3);

    long span = 1000000/60;
    long actual = 0;
    if (gap2 < span) {
        actual = (span-gap2);
        SDL_DelayPrecise(actual*1000);
    }

    as->phase2 = SDL_GetTicksNS() / 1000;
    gap3 = (long)(as->phase2 - as->phase1);
    /*
    SDL_Log("Actual %ld vs %ld",gap3,actual);
    SDL_Log("Gap 1: %ld",gap1);
    SDL_Log("Gap 2: %ld",gap2);
    SDL_Log("Gap 3: %ld",gap3);
    SDL_Log("Total: %ld vs %ld",gap1+gap2+gap3,span);
     */
    
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    AppState *as = (AppState *)appstate;
    switch (event->type) {
    case SDL_EVENT_QUIT:
        return SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    if (appstate != NULL) {
        AppState *as = (AppState *)appstate;
        // We have to clear the renderer
        SDL_RenderPresent(as->renderer);
        SDL_DestroyTexture(as->image);
        SDL_DestroyTexture(as->label);
        SDL_DestroyRenderer(as->renderer);
        SDL_DestroyWindow(as->window);
        SDL_free(as);
    }
}
