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

void MusicCircle::circle_render(SDL_Renderer* renderer, SDL_Texture *red_circle, SDL_Texture *blue_circle, SDL_Texture *green_circle, SDL_Texture *yellow_circle, int SCREEN_HEIGHT,int GAME_START_TIME,int TIME_CIRCLE_ON_SCREEN,int PAUSED_TICKS)
{
	int new_y=y_pos+ (SDL_GetTicks()-(GAME_START_TIME+(time_spawn)+PAUSED_TICKS)) * SCREEN_HEIGHT / ( TIME_CIRCLE_ON_SCREEN);
	if (colour==1)
	{
		renderInRect(renderer,red_circle,x_pos-radius,new_y-radius,2*radius,2*radius);
	}
	else if (colour==2)
	{
		renderInRect(renderer,blue_circle,x_pos-radius,new_y-radius,2*radius,2*radius);
	}
	else if (colour==3)
	{
		renderInRect(renderer,green_circle,x_pos-radius,new_y-radius,2*radius,2*radius);
	}
	else if (colour==4)
	{
		renderInRect(renderer,yellow_circle,x_pos-radius,new_y-radius,2*radius,2*radius);
	}
		
}

void MusicCircle::effect_render(SDL_Renderer* renderer,SDL_Texture *black_circle, SDL_Texture *red_circle,SDL_Texture *blue_circle, SDL_Texture *green_circle, SDL_Texture *yellow_circle,int SCREEN_HEIGHT, int GAME_START_TIME,int TIME_EFFECT_ON_SCREEN, int PAUSED_TICKS)
{
	int new_radius=1.1*radius+0.1*radius*pow(10,(SDL_GetTicks()-(GAME_START_TIME+(time_spawn)+PAUSED_TICKS))/float(TIME_EFFECT_ON_SCREEN));
	if (colour==1)
	{
		renderInRect(renderer,red_circle,x_pos-new_radius,y_pos-new_radius,2*new_radius,2*new_radius);
	}
	else if (colour==2)
	{
		renderInRect(renderer,blue_circle,x_pos-new_radius,y_pos-new_radius,2*new_radius,2*new_radius);
	}
	else if (colour==3)
	{
		renderInRect(renderer,green_circle,x_pos-new_radius,y_pos-new_radius,2*new_radius,2*new_radius);
	}
	else if (colour==4)
	{
		renderInRect(renderer,yellow_circle,x_pos-new_radius,y_pos-new_radius,2*new_radius,2*new_radius);
	}
	if ((SDL_GetTicks()-(GAME_START_TIME+time_spawn+PAUSED_TICKS))<TIME_EFFECT_ON_SCREEN/4)
		new_radius -= 0.009*SCREEN_HEIGHT;
	else if ((SDL_GetTicks()-(GAME_START_TIME+time_spawn+PAUSED_TICKS))<TIME_EFFECT_ON_SCREEN/2)
		new_radius -= 0.006*SCREEN_HEIGHT;
	else new_radius -= 0.004*SCREEN_HEIGHT;
	renderInRect(renderer,black_circle,x_pos-new_radius,y_pos-new_radius,2*new_radius,2*new_radius);
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
		

