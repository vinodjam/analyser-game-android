#include "music_circle.hpp"

MusicCircle::MusicCircle()
{
	radius=0;
	colour=0;
	x_pos=0;
	y_pos=0;
	time_spawn=0;
}

MusicCircle::MusicCircle(int r,int c,int x,int y,int t)
{
	radius=r;
	colour=c;
	x_pos=x;
	y_pos=y;
	time_spawn=t;
}

void MusicCircle::circle_render(SDL_Renderer* renderer, SDL_Texture *texture,int SCREEN_HEIGHT,int GAME_START_TIME,int TIME_CIRCLE_ON_SCREEN)
{
	int new_y=y_pos+ (SDL_GetTicks()-(GAME_START_TIME+(time_spawn))) * SCREEN_HEIGHT / ( TIME_CIRCLE_ON_SCREEN);
	if (colour==1)
	{
		renderInRect(renderer,texture,x_pos-radius,new_y-radius,2*radius,2*radius);
	}
		
}

void MusicCircle::effect_render(SDL_Renderer* renderer, SDL_Texture *texture1,SDL_Texture *texture2,int SCREEN_HEIGHT, int GAME_START_TIME,int TIME_EFFECT_ON_SCREEN)
{
	int new_radius=0.9*radius+0.25*radius*(SDL_GetTicks()-(GAME_START_TIME+(time_spawn)))/float(TIME_EFFECT_ON_SCREEN);
	if (colour==1)
	{
		renderInRect(renderer,texture1,x_pos-new_radius,y_pos-new_radius,2*new_radius,2*new_radius);
	}
	new_radius -= 0.003*SCREEN_HEIGHT;
	renderInRect(renderer,texture2,x_pos-new_radius,y_pos-new_radius,2*new_radius,2*new_radius);
}


myQueue::myQueue()
	{
		beginning=NULL;
		end=NULL;
	}
	
void myQueue::add_element(MusicCircle temp_circle)
{
	mynode *temp=new mynode;
	if (beginning==NULL)
	{
		end=temp;
		beginning=temp;
		end->circle=temp_circle;
	}
	else
	{
		//cout<<"PROBLEM HERE"<<endl;
		end->next=temp;
		end=end->next;
		end->circle=temp_circle;
	}
	temp->next=NULL;
	temp=NULL;
	delete(temp);
}
			
void myQueue::remove_element()
{
	mynode *temp=beginning;
	if (beginning==end)
	{
		beginning->next=NULL;
		end=end->next;	
	}
	beginning=beginning->next;
	temp->next=NULL;
	temp=NULL;
	delete(temp);
}
		

