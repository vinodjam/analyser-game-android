#ifndef MY_SDL_FUNC_CPP_INCLUDED
#define MY_SDL_FUNC_CPP_INCLUDED

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

bool init_SDL(SDL_Renderer **renderer, SDL_Window **window, int* , int* );

bool init_SDLimage();

bool init_SDLmixer();

bool init_SDLttf();

bool init_SDLGame(SDL_Renderer **renderer, SDL_Window **window, int*, int*);

bool loadTexture(SDL_Renderer *renderer, SDL_Texture* &texture,char path[]);

void renderInRect(SDL_Renderer *renderer, SDL_Texture *t,int x,int y,int w,int h);

#endif
