#include "pattern.h"

/**
 * Draws a Gimpish background pattern to show transparency in the image 
 */
void drawgimp(SDL_Renderer *renderer, int w, int h) {
    const int GRID_SIZE = 32;
    SDL_Color col[2] = {
        { 0x66, 0x66, 0x66, 0xff },
        { 0x99, 0x99, 0x99, 0xff },
    };
    
    int i, x, y;
    SDL_FRect rect;
    rect.w = GRID_SIZE;
    rect.h = GRID_SIZE;
    
    int xlimit = (w/GRID_SIZE % 2 == 0 ? w/GRID_SIZE : 1+w/GRID_SIZE);
    int ylimit = (h/GRID_SIZE % 2 == 0 ? h/GRID_SIZE : 1+h/GRID_SIZE);

    for (y = 0; y <= ylimit; y++) {
        for (x = 0; x <= xlimit; x++) {
            i = (x+y) % 2;
            i = (x+y) % 2;
            SDL_SetRenderDrawColor(renderer, col[i].r, col[i].g, col[i].b, col[i].a);
            
            rect.x = x*rect.w;
            rect.y = y*rect.h;
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}
