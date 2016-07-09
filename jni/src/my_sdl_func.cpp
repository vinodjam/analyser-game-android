#include "my_sdl_func.hpp"

bool init_SDL(SDL_Renderer **renderer, SDL_Window **window,int *SCREEN_WIDTH,int *SCREEN_HEIGHT)
{
	bool success=false;
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)!=0)
	{
		SDL_Log("Failed in init SDL\n");
		SDL_Log(SDL_GetError(),"%s\n");
		success=false;
	}
	else
	{
		if (SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY,"1")==0)
		{
			SDL_Log("Failed to enable linear texture filtering\n");
			SDL_Log(SDL_GetError(),"%s\n");
			success=false;
		}
		else
		{   
			SDL_DisplayMode displayMode;
			if( SDL_GetCurrentDisplayMode( 0, &displayMode ) == 0 )
			{
				*SCREEN_WIDTH = displayMode.w;
				*SCREEN_HEIGHT = displayMode.h;
			}
			*window=SDL_CreateWindow("Game",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,*SCREEN_WIDTH,*SCREEN_HEIGHT,SDL_WINDOW_SHOWN);
			if (*window==NULL)
			{
				SDL_Log("Failed to init Window\n");
				SDL_Log(SDL_GetError(),"%s\n");
				success=false;
			}
			else
			{
				*renderer=SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED| SDL_RENDERER_PRESENTVSYNC);
				if (*renderer==NULL)
				{
					SDL_Log("Failed to init Renderer\n");
					SDL_Log(SDL_GetError(),"%s\n");
					success=false;	
				}
				else
				{
					SDL_SetRenderDrawColor(*renderer,0x00,0x00,0x00,SDL_ALPHA_OPAQUE);
					success=true;
				}
			}
		}
	}
	return success;
}

bool init_SDLimage()
{
	bool success;
	int imgFlags=IMG_INIT_PNG;
	if (IMG_Init(imgFlags) & (imgFlags !=imgFlags))
	{
		SDL_Log("Failed to init PNG support \n");
		SDL_Log(IMG_GetError(),"%s\n");
		success=false;
	}
	else success=true;
	return success;
}

bool init_SDLmixer()
{
	bool success;
	if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 )
	{
		SDL_Log( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() ); 
		success = false; 
	}
	else success=true;
	return success;
}

bool init_SDLttf()
{
	bool success;
	if( TTF_Init() == -1 ) 
	{
		SDL_Log( "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError() ); 
		success = false; 
	}
	else success=true;
	return success;
}	

bool init_SDLGame(SDL_Renderer **renderer, SDL_Window **window,int* SCREEN_WIDTH,int* SCREEN_HEIGHT)
{
	if (init_SDL(renderer,window,SCREEN_WIDTH,SCREEN_HEIGHT)==true)
		if (init_SDLimage()==true)
			if (init_SDLmixer()==true)
				if (init_SDLttf()==true)
				{
					SDL_Log("SDL Completely Initialised \n");
					return true;
				}
				else return false;
			else return false;
		else return false;
	else return false;
}


bool loadTexture(SDL_Renderer *renderer, SDL_Texture* &texture,char path[])
{
	bool success=false;
	texture=NULL;
	SDL_Surface* temp_surface=IMG_Load(path);
	if (temp_surface==NULL)
	{
		SDL_Log("Failed to load image at %s\n",path);
		SDL_Log(IMG_GetError(),"%s\n");
		success=false;
	}
	else
	{
		SDL_SetColorKey(temp_surface,SDL_TRUE,SDL_MapRGB(temp_surface->format, 0xFF,0xFF,0xFF));
		texture=SDL_CreateTextureFromSurface(renderer,temp_surface);
		SDL_FreeSurface(temp_surface);
		if (texture==NULL)
		{
			SDL_Log("Texture could not be created %s\n",path);
			SDL_Log(SDL_GetError(),"%s\n");
			success=false;
		}
		else
		{
			success=true;
		}
	}
	return success;
}

void renderInRect(SDL_Renderer *renderer, SDL_Texture *t,int x,int y,int w,int h)
{
	SDL_Rect target={x,y,w,h};
	SDL_RenderCopy(renderer,t,NULL,&target);
}
