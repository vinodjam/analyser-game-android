#include "my_sdl_func.hpp"

class MusicCircle
{
	public:
	
	int radius;
	int colour;
	int x_pos;
	int y_pos;
	int time_spawn;
	

	MusicCircle();
	MusicCircle(int r,int c,int x,int y,int t);
		
	void circle_render(SDL_Renderer*,SDL_Texture*, int, int ,int );												//TO DO
	void effect_render(SDL_Renderer*,SDL_Texture*,SDL_Texture*, int, int ,int );
	int check_click(int x,int y);
		
};


struct mynode
{
	MusicCircle circle;
	mynode* next;
};

class myQueue
{
	public:
	
	mynode *beginning;
	mynode *end;
	
	myQueue();
	
	void add_element(MusicCircle temp_circle);
			
	void remove_element();
		
};


