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
    SDL_Rect rect;
    rect.w = GRID_SIZE;
    rect.h = GRID_SIZE;
    
    int xlimit = (w/rect.w % 2 == 0 ? w/rect.w : 1+w/rect.w);
    int ylimit = (h/rect.h % 2 == 0 ? h/rect.h : 1+h/rect.h);
    
    for (y = 0; y <= ylimit; y++) {
        for (x = 0; x <= xlimit; x++) {
            i = (x+y) % 2;
            SDL_SetRenderDrawColor(renderer, col[i].r, col[i].g, col[i].b, col[i].a);
            
            rect.x = x*rect.w;
            rect.y = y*rect.h;
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}
