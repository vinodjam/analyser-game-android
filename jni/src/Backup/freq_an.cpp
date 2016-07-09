#include <iostream>

//#include <tremor/ivorbiscodec.h>
//#include <tremor/ivorbisfile.h>

#include <mpg123.h>

#include <ivorbiscodec.h>
#include <ivorbisfile.h>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

#include <stdio.h>
#include <stdlib.h>

#include "my_sdl_func.hpp"
#include "music_circle.hpp"

#include <kiss_fft.h>
#include <kiss_fftr.h>


#define THRESHOLD 0.1
#define APPROX_TIME_DIVISIONS 4
#define TIME_CIRCLE_ON_SCREEN 4000
#define TIME_EFFECT_ON_SCREEN 1000
#define OVERLAPS 2

int SAMPLES_IN_HANN_WINDOW=-1;
int NO_OF_CHANNELS=-1;
long int SAMPLE_RATE=-1;

char MUSIC_PATH[200]="\0";// "/storage/sdcard0/Music/Wesnoth/siege_of_laurelmor.ogg"


using namespace std; //REMOVE*******************

SDL_Window *window=NULL;
SDL_Renderer *renderer=NULL;

int SCREEN_WIDTH = 800;
int SCREEN_HEIGHT = 640;
int GAME_START_TIME=-1;
int my_index=0;
int PLOT_SAMPLES=-1;

float GLOB_MAX_POS=0,GLOB_MAX_POWER=0,GLOB_MAX_POWER_SQ=0,MIN_POS=-1;

float VREF=0;

float *HANN_COEFF=NULL;

myQueue circles,effects;

SDL_Texture *red_circle=NULL, *red_bar=NULL, *black_circle=NULL, *background=NULL, *gray=NULL;

int pow(int a,int b)
{
	int prod=1;
	for (int i=1;i<=b;i++)
	{
		prod*=a;
	}
	return prod;
}

void close_cleanup()
{
	SDL_DestroyTexture(background);
	background=NULL;
	SDL_DestroyTexture(red_circle);
	red_circle=NULL;
	SDL_DestroyTexture(black_circle);
	black_circle=NULL;
	SDL_DestroyTexture(gray);
	gray=NULL;
	//TTF_CloseFont(INSERT HERE);
	//font=NULL;
	SDL_DestroyRenderer(renderer);
	renderer=NULL;
	SDL_DestroyWindow(window);
	window=NULL;
	TTF_Quit();
	Mix_Quit();
	IMG_Quit();
	SDL_Quit();
}
	
void loadAllTextures()
{
	if (loadTexture(renderer,background,"Resources/Pictures/Background.png")!=true)
	{
		SDL_Log("Could not load Background\n");
	}
	if (loadTexture(renderer,red_circle,"Resources/Pictures/RedCircle.png")!=true)
	{
		SDL_Log("Could not load Red Circle","%s\n");
	}
	if (loadTexture(renderer,black_circle,"Resources/Pictures/BlackCircle.png")!=true)
	{
		SDL_Log("Could not load Black Circle\n");
	}
	if (loadTexture(renderer,gray,"Resources/Pictures/Gray.png")!=true)
	{
		SDL_Log("Could not load Gray\n");
	}
	if (loadTexture(renderer,red_bar,"Resources/Pictures/RedBox.png")!=true)
	{
		SDL_Log("Could not load Red Box\n");
	}
	
}
	
	

void renderCircles()
{
	mynode *temp=circles.beginning;
	for (int i=0;i<1000 && temp->next!=NULL;i++)
	{
		if ( (temp->circle.time_spawn+GAME_START_TIME) > (SDL_GetTicks()+500) )
		{
			temp=temp->next;

			//NOT YET TIME
		}
		else if ( (temp->circle.time_spawn+GAME_START_TIME + TIME_CIRCLE_ON_SCREEN + 3000) < (SDL_GetTicks()) ) //3000 is buffer time
		{
			temp=temp->next;
			circles.remove_element();
		}
		else
		{
			temp->circle.circle_render(renderer,red_circle, SCREEN_HEIGHT, GAME_START_TIME,TIME_CIRCLE_ON_SCREEN);
			temp=temp->next;

		}
	} 
}

void renderEffects()
{
	mynode *temp=effects.beginning;
	if (temp!=NULL)
	{
		for (;temp!=NULL;)
		{
			//<<i<<endl;
			if ((temp->circle.time_spawn+GAME_START_TIME+TIME_EFFECT_ON_SCREEN)<(SDL_GetTicks()))
			{
				temp=temp->next;
				effects.remove_element();
			}
			else
			{
				temp->circle.effect_render(renderer,red_circle,black_circle,SCREEN_HEIGHT, GAME_START_TIME,TIME_EFFECT_ON_SCREEN);
				temp=temp->next;

			}
		} 
	}
	//else cout<<"EMPTY"<<endl;
}

	
void checkCircleClick(int x,int y)
{
	mynode *temp=circles.beginning;
	for (int i=0;i<15 && temp!=NULL;temp=temp->next,i++)
	{
		if ((temp->circle.x_pos-temp->circle.radius)<x && (temp->circle.x_pos+temp->circle.radius)>x)
		{			
			//SDL_Log("Y POS %d\n",new_ypos);
			int timediff=SDL_GetTicks()-GAME_START_TIME-temp->circle.time_spawn;
			if (timediff>TIME_CIRCLE_ON_SCREEN*0.8)
			{
				MusicCircle new_effect=temp->circle;
				int new_ypos=temp->circle.y_pos+ ( ( (SDL_GetTicks()-GAME_START_TIME-temp->circle.time_spawn) * SCREEN_HEIGHT )/ TIME_CIRCLE_ON_SCREEN );
			
				new_effect.y_pos=new_ypos;
				new_effect.time_spawn=(SDL_GetTicks()-GAME_START_TIME);
				effects.add_element(new_effect);
				while (circles.beginning!=temp && temp->next!=NULL)
					circles.remove_element();
				temp=temp->next;
				circles.remove_element();
				if (temp==NULL)
					break;
				//SDL_Log("Removing %d\n",new_ypos);
				//effects.beginning->circle.effect_render(renderer,red_circle,black_circle,SCREEN_HEIGHT, GAME_START_TIME,TIME_EFFECT_ON_SCREEN);
			}
		} 
	}
	/*
	temp=circles.beginning;
	int number=0;
	while (temp!=NULL)
	{
		temp=temp->next;
		number++;
	}
	SDL_Log("No of Circles : %d\n",number);
	temp=effects.beginning;
	number=0;
	while (temp!=NULL)
	{
		temp=temp->next;
		number++;
	}
	SDL_Log("No of Effects : %d\n",number);
	SDL_Log("--------------\n");
	*/
}



int decode_ogg(char path[])
{
	int counter=0;
	char pcmout[4096];
	long left,right;
	float sample;
	int pos=0;
	float maxi,power;
	int notimes=0,temptot=0;
	
	//bool first_time=true;
	char *path2=NULL;
	
	FILE *music=NULL;
	//music=fopen(path,"rb");
	music=fopen(MUSIC_PATH,"rb");
	if (music==NULL)
	{
		//fclose(music);
		SDL_Log("Path Error 1 \n");
		
	}
	OggVorbis_File vf;
	int eof=0;
	int current_section;
	if(ov_open(music, &vf, NULL, 0) < 0) 
	{
		SDL_Log("Input does not appear to be an Ogg bitstream.\n");
		return -3;
	}  
	vorbis_info *vi=ov_info(&vf,-1);

	SDL_Log("\nBitstream is %d channel, %ldHz\n",vi->channels,vi->rate);
	SDL_Log("Encoded by: %s\n\n",ov_comment(&vf,-1)->vendor);
    
	NO_OF_CHANNELS=vi->channels;
	SAMPLE_RATE=vi->rate;
	int temp1=0;
	float temp2=SAMPLE_RATE/NO_OF_CHANNELS;//*vi->channels;
    
	while (pow(2,temp1)<(temp2/APPROX_TIME_DIVISIONS))
	{
		temp1++;
	}				//TO CHOOSE CORRECT POWER OF 2
	SAMPLES_IN_HANN_WINDOW=pow(2,(temp1-1));
	SDL_Log("Sample in a window         : %d\n",SAMPLES_IN_HANN_WINDOW);

	//SAMPLES_IN_HANN_WINDOW=32768;
	char *samples_buffer=(char*)malloc(NO_OF_CHANNELS*SAMPLES_IN_HANN_WINDOW*sizeof(char)*2);		//2 BYTES PER SAMPLE

	HANN_COEFF=(float*)malloc(SAMPLES_IN_HANN_WINDOW*sizeof(float));
	
	float *sample_for_fft=(float*)malloc(SAMPLES_IN_HANN_WINDOW*sizeof(float));
	kiss_fft_cpx *output=(kiss_fft_cpx*)malloc((SAMPLES_IN_HANN_WINDOW/2+1)*sizeof(kiss_fft_cpx));
	
	float LOCAL_VREF[8]={0,0,0,0,0,0,0,0};
	float *output_buffer_2sec[8];
	for (int i=0;i<8;i++)
	{
		output_buffer_2sec[i]=(float*)malloc(OVERLAPS*APPROX_TIME_DIVISIONS*2*sizeof(float));
	}
	
	kiss_fftr_cfg test_cfg=kiss_fftr_alloc(SAMPLES_IN_HANN_WINDOW,0,NULL,NULL);
	
	for (int i=0;i<SAMPLES_IN_HANN_WINDOW;i++)
	{
		HANN_COEFF[i]=0.5f*(1.0f-cos(2.0f*M_PI*(float)(i)/(float)(SAMPLES_IN_HANN_WINDOW-1.0f)));
		//SDL_Log("%f ",HANN_COEFF[i]);
	}
	
	PLOT_SAMPLES=SAMPLES_IN_HANN_WINDOW/4;
	
	float time_increment=(float)SAMPLES_IN_HANN_WINDOW*float(1000)/float(SAMPLE_RATE);
	//int timep=time_increment;
	
	int LAST_CIRCLE=2000;
	
	//SDL_Log("\n\n");
	while(!eof)
	{
		long ret;
		ret=ov_read(&vf,pcmout,sizeof(pcmout),&current_section);
		temptot+=ret;
		if (ret == 0)
		{
			// EOF 
			eof=1;
		
		} 
		else if (ret < 0) 
		{
			// error in the stream.  Not a problem, just reporting it in
			//case we (the app) cares.  In this case, we don't. 
		}
		else
		{
			counter+=ret;
			if (counter>=SAMPLES_IN_HANN_WINDOW*NO_OF_CHANNELS*2)
			{
				notimes++;
				memcpy((samples_buffer+counter-ret) , pcmout, ret-(counter%SAMPLES_IN_HANN_WINDOW));
				for (int x=0;x<SAMPLES_IN_HANN_WINDOW*NO_OF_CHANNELS*2;x+=2*NO_OF_CHANNELS)
				{
					if (NO_OF_CHANNELS==2)	//CAN BE IMPROVED
					{
						left=(samples_buffer[x]|(samples_buffer[x+1] << 8));
						right=(samples_buffer[x+2]|(samples_buffer[x+3] << 8));
						sample=(left+right)*0.5*HANN_COEFF[x/4];
						//sample_for_fft[x/4].r=sample;
						//sample_for_fft[x/4].i=0;
						sample_for_fft[x/4]=sample;
					}
					else if (NO_OF_CHANNELS==1)
					{
						sample=(samples_buffer[x]|(samples_buffer[x+1] << 8));
						sample_for_fft[x/2]=sample*HANN_COEFF[x/2];
						//sample_for_fft[x/2].r=sample*HANN_COEFF[x/2];
						//sample_for_fft[x/2].i=0;
					}
				}
				kiss_fftr(test_cfg,sample_for_fft,output);
				maxi=0;
				for (int l=0;l<8;l++)
				{
					int j=l*SAMPLES_IN_HANN_WINDOW/16;
					for ( ; j<((l+1)*SAMPLES_IN_HANN_WINDOW/16);j++)		//OVERLAPS*TIME_DIVISONS=8, half of output is useful
					{
						power=pow(output[j].r,2)+pow(output[j].i,2);
						output_buffer_2sec[l][my_index]+=power*power;
					}
					LOCAL_VREF[l]=LOCAL_VREF[l]*LOCAL_VREF[l]*7+output_buffer_2sec[l][my_index];
					LOCAL_VREF[l]/=8;
					LOCAL_VREF[l]=sqrt(LOCAL_VREF[l]);
					output_buffer_2sec[l][my_index]=sqrt(output_buffer_2sec[l][my_index]);
				}
				float max_inc=0;
				int l_max=-1;
				for (int l=0;l<8;l++)
				{
					float inc=log10(output_buffer_2sec[l][my_index]/LOCAL_VREF[l]);
					if (inc>THRESHOLD)
					{
						if (inc>max_inc)
						{
							max_inc=log10(output_buffer_2sec[l][my_index]/LOCAL_VREF[l]);
							l_max=l;
						}

					}
				}
				int tspawn=time_increment*notimes/2;
				if (l_max!=-1 && tspawn-LAST_CIRCLE>500)
				{
					int colour=1;
					int rad=SCREEN_WIDTH/5;
					int y_pos=0;
					int x_pos=SCREEN_WIDTH/16+l_max*SCREEN_WIDTH/8;
					MusicCircle temp_circle(rad,colour,x_pos,y_pos,tspawn);
					circles.add_element(temp_circle);
					LAST_CIRCLE=tspawn;
				}		
				my_index++;
				my_index%=8;
				
				for (int j=0;j<(SAMPLES_IN_HANN_WINDOW/2+1)/8;j++)
				{
					power=pow(output[j].r,2)+pow(output[j].i,2);
					
					if (GLOB_MAX_POWER<power)
					{
						GLOB_MAX_POWER=power;
						for (int k=0;k<SAMPLES_IN_HANN_WINDOW/2+1;k++)
						{
							power=pow(output[k].r,2)+pow(output[k].i,2);
							VREF+=power*power;
						}
						VREF/=(SAMPLES_IN_HANN_WINDOW/2+1);
						VREF=sqrt(VREF);
					}
							
	
				}
				//SDL_RWwrite(fft_file,&pos,sizeof(pos),1);
				//SDL_RWwrite(fft_file,&output[pos],sizeof(output[pos]),1);	
				counter%=SAMPLES_IN_HANN_WINDOW*NO_OF_CHANNELS*2;
				memmove(samples_buffer,samples_buffer+SAMPLES_IN_HANN_WINDOW/OVERLAPS,counter+SAMPLES_IN_HANN_WINDOW*2*(1-1.0/OVERLAPS));
				counter+=SAMPLES_IN_HANN_WINDOW*NO_OF_CHANNELS*2*(1-1.0/OVERLAPS);
				//printf("Counter %d\n",counter);
				//SDL_Log("Samples read in a window         : %d\n",notimes*SAMPLES_IN_HANN_WINDOW/2);
			}
			else
			{
				memcpy( (samples_buffer+counter-ret) , pcmout, ret);
			}			
		}
	}
	//cout<<NO_OF_CHANNELS*SAMPLES_IN_HANN_WINDOW*sizeof(char)*2<<endl;
	SDL_Log("Actual Sample         : %ld\n",ov_pcm_total(&vf,-1));
	ov_clear(&vf);    
	//SDL_Log("Samples read in a window         : %d\n",notimes*SAMPLES_IN_HANN_WINDOW/2);
	//SDL_Log("Temptot         : %d\n",temptot/4);
	free(samples_buffer);
	free(sample_for_fft);
	free(output);	
	for (int i=0;i<8;i++)
		free(output_buffer_2sec[i]);

	SDL_Log("Done.\n");
	return(0);
}



int decode_mp3(char path[])
{
	SDL_Log("Decoding Started\n");
	int counter=0;
	unsigned char pcmout[4096];
	long left,right;
	float sample;
	int pos=0;
	float maxi,power;
	int notimes=0,temptot=0;
	
	size_t buffer_size=-1;
	mpg123_handle *mh = NULL;
	
	int  err  = MPG123_OK;
	int encoding = 0;
	err = mpg123_init();
	SDL_Log("Decoding Error mpg123 Started %d\n",err);

	if(err != MPG123_OK || (mh = mpg123_new(NULL, &err)) == NULL)
	{
		SDL_Log( "Basic setup goes wrong: %s", mpg123_plain_strerror(err));
		mpg123_close(mh);
		mpg123_delete(mh);
		mpg123_exit();
		return -1;
	}
	if(    mpg123_open(mh, MUSIC_PATH) != MPG123_OK
	    /* Peek into track and get first output format. */
	    || mpg123_getformat(mh, &SAMPLE_RATE, &NO_OF_CHANNELS, &encoding) != MPG123_OK )
	{
		SDL_Log( "Trouble with mpg123: %s\n", mpg123_strerror(mh) );
		mpg123_close(mh);
		mpg123_delete(mh);
		mpg123_exit();
		return -1;
	}
	if(encoding != MPG123_ENC_SIGNED_16 && encoding != MPG123_ENC_FLOAT_32)
	{ /* Signed 16 is the default output format anyways; it would actually by only different if we forced it.
	     So this check is here just for this explanation. */
		mpg123_close(mh);
		mpg123_delete(mh);
		mpg123_exit();
		SDL_Log( "Bad encoding: 0x%x!\n", encoding);
		return -2;
	}
	/* Ensure that this output format will not change (it could, when we allow it). */
	mpg123_format_none(mh);
	mpg123_format(mh, SAMPLE_RATE, NO_OF_CHANNELS, encoding);


	SDL_Log("\nBitstream is %d channel, %ldHz\n",NO_OF_CHANNELS,SAMPLE_RATE);

    
	int temp1=0;
	float temp2=SAMPLE_RATE/NO_OF_CHANNELS;//*vi->channels;
    
	while (pow(2,temp1)<(temp2/APPROX_TIME_DIVISIONS))
	{
		temp1++;
	}				//TO CHOOSE CORRECT POWER OF 2
	SAMPLES_IN_HANN_WINDOW=pow(2,(temp1-1));
	SDL_Log("Sample in a window         : %d\n",SAMPLES_IN_HANN_WINDOW);

	//SAMPLES_IN_HANN_WINDOW=32768;
	char *samples_buffer=(char*)malloc(NO_OF_CHANNELS*SAMPLES_IN_HANN_WINDOW*sizeof(char)*2);		//2 BYTES PER SAMPLE

	HANN_COEFF=(float*)malloc(SAMPLES_IN_HANN_WINDOW*sizeof(float));
	
	float *sample_for_fft=(float*)malloc(SAMPLES_IN_HANN_WINDOW*sizeof(float));
	kiss_fft_cpx *output=(kiss_fft_cpx*)malloc((SAMPLES_IN_HANN_WINDOW/2+1)*sizeof(kiss_fft_cpx));
	
	float LOCAL_VREF[8]={0,0,0,0,0,0,0,0};
	float *output_buffer_2sec[8];
	for (int i=0;i<8;i++)
	{
		output_buffer_2sec[i]=(float*)malloc(OVERLAPS*APPROX_TIME_DIVISIONS*2*sizeof(float));
	}
	
	kiss_fftr_cfg test_cfg=kiss_fftr_alloc(SAMPLES_IN_HANN_WINDOW,0,NULL,NULL);
	
	for (int i=0;i<SAMPLES_IN_HANN_WINDOW;i++)
	{
		HANN_COEFF[i]=0.5f*(1.0f-cos(2.0f*M_PI*(float)(i)/(float)(SAMPLES_IN_HANN_WINDOW-1.0f)));
		//SDL_Log("%f ",HANN_COEFF[i]);
	}
	
	PLOT_SAMPLES=SAMPLES_IN_HANN_WINDOW/4;
	
	float time_increment=(float)SAMPLES_IN_HANN_WINDOW*float(1000)/float(SAMPLE_RATE);
	//int timep=time_increment;
	
	int LAST_CIRCLE=2000;
	
	//SDL_Log("\n\n");
	while(err==MPG123_OK)
	{
		size_t ret;
		err = mpg123_read( mh, pcmout, sizeof(pcmout), &ret );

		temptot+=ret;
		if (ret == 0)
		{
			/* EOF */
			
		
		} 
		else if (ret < 0) 
		{
			/* error in the stream.  Not a problem, just reporting it in
			case we (the app) cares.  In this case, we don't. */
		}
		else
		{
			counter+=ret;
			if (counter>=SAMPLES_IN_HANN_WINDOW*NO_OF_CHANNELS*2)
			{
				notimes++;
				memcpy((samples_buffer+counter-ret) , pcmout, ret-(counter%SAMPLES_IN_HANN_WINDOW));
				for (int x=0;x<SAMPLES_IN_HANN_WINDOW*NO_OF_CHANNELS*2;x+=2*NO_OF_CHANNELS)
				{
					if (NO_OF_CHANNELS==2)	//CAN BE IMPROVED
					{
						left=(samples_buffer[x]|(samples_buffer[x+1] << 8));
						right=(samples_buffer[x+2]|(samples_buffer[x+3] << 8));
						sample=(left+right)*0.5*HANN_COEFF[x/4];
						//sample_for_fft[x/4].r=sample;
						//sample_for_fft[x/4].i=0;
						sample_for_fft[x/4]=sample;
					}
					else if (NO_OF_CHANNELS==1)
					{
						sample=(samples_buffer[x]|(samples_buffer[x+1] << 8));
						sample_for_fft[x/2]=sample*HANN_COEFF[x/2];
						//sample_for_fft[x/2].r=sample*HANN_COEFF[x/2];
						//sample_for_fft[x/2].i=0;
					}
				}
				kiss_fftr(test_cfg,sample_for_fft,output);
				maxi=0;
				for (int l=0;l<8;l++)
				{
					int j=l*SAMPLES_IN_HANN_WINDOW/16;
					for ( ; j<((l+1)*SAMPLES_IN_HANN_WINDOW/16);j++)		//OVERLAPS*TIME_DIVISONS=8, half of output is useful
					{
						power=pow(output[j].r,2)+pow(output[j].i,2);
						output_buffer_2sec[l][my_index]+=power*power;
					}
					LOCAL_VREF[l]=LOCAL_VREF[l]*LOCAL_VREF[l]*7+output_buffer_2sec[l][my_index];
					LOCAL_VREF[l]/=8;
					LOCAL_VREF[l]=sqrt(LOCAL_VREF[l]);
					output_buffer_2sec[l][my_index]=sqrt(output_buffer_2sec[l][my_index]);
				}
				float max_inc=0;
				int l_max=-1;
				for (int l=0;l<8;l++)
				{
					float inc=log10(output_buffer_2sec[l][my_index]/LOCAL_VREF[l]);
					if (inc>THRESHOLD)
					{
						if (inc>max_inc)
						{
							max_inc=log10(output_buffer_2sec[l][my_index]/LOCAL_VREF[l]);
							l_max=l;
						}

					}
				}
				int tspawn=time_increment*notimes/2;
				if (l_max!=-1 && tspawn-LAST_CIRCLE>500)
				{
					int colour=1;
					int rad=SCREEN_WIDTH/5;
					int y_pos=-rad;
					int x_pos=SCREEN_WIDTH/16+l_max*SCREEN_WIDTH/8;
					MusicCircle temp_circle(rad,colour,x_pos,y_pos,tspawn);
					circles.add_element(temp_circle);
					LAST_CIRCLE=tspawn;
				}		
				my_index++;
				my_index%=8;
				
				for (int j=0;j<(SAMPLES_IN_HANN_WINDOW/2+1)/8;j++)
				{
					power=pow(output[j].r,2)+pow(output[j].i,2);
					
					if (GLOB_MAX_POWER<power)
					{
						GLOB_MAX_POWER=power;
						for (int k=0;k<SAMPLES_IN_HANN_WINDOW/2+1;k++)
						{
							power=pow(output[k].r,2)+pow(output[k].i,2);
							VREF+=power*power;
						}
						VREF/=(SAMPLES_IN_HANN_WINDOW/2+1);
						VREF=sqrt(VREF);
					}
							
	
				}
				//SDL_RWwrite(fft_file,&pos,sizeof(pos),1);
				//SDL_RWwrite(fft_file,&output[pos],sizeof(output[pos]),1);	
				counter%=SAMPLES_IN_HANN_WINDOW*NO_OF_CHANNELS*2;
				memmove(samples_buffer,samples_buffer+SAMPLES_IN_HANN_WINDOW/OVERLAPS,counter+SAMPLES_IN_HANN_WINDOW*2*(1-1.0/OVERLAPS));
				counter+=SAMPLES_IN_HANN_WINDOW*NO_OF_CHANNELS*2*(1-1.0/OVERLAPS);
				//printf("Counter %d\n",counter);
				//SDL_Log("NoTimes : %d\n",notimes);
				//SDL_Log("Samples read in a window         : %d\n",notimes*SAMPLES_IN_HANN_WINDOW/2);
			}
			else
			{
				memcpy( (samples_buffer+counter-ret) , pcmout, ret);
			}			
		}
	}
	
	if(err != MPG123_DONE)
		SDL_Log( "Warning: Decoding ended prematurely because: %s\n",
	         err == MPG123_ERR ? mpg123_strerror(mh) : mpg123_plain_strerror(err) );


	mpg123_close(mh);
	mpg123_delete(mh);
	mpg123_exit();
	
	free(samples_buffer);
	free(sample_for_fft);
	free(output);	
	for (int i=0;i<8;i++)
		free(output_buffer_2sec[i]);

	SDL_Log("Done.\n");
	return(0);
}


bool start_game(char path[])
{
	VREF/=100;
	GLOB_MAX_POWER_SQ=GLOB_MAX_POWER*GLOB_MAX_POWER;
	if (decode_ogg(path)==-3)
		decode_mp3(path);
	//open_ogg(path);
	//prelim_for_creation();
	//create_circles();
	float time_increment=(float)SAMPLES_IN_HANN_WINDOW*float(1000)/float(SAMPLE_RATE);
	int tmp=time_increment;
	SDL_Log("Time Increment %f %d\n%f %f\n",time_increment,tmp,(float)SAMPLES_IN_HANN_WINDOW,(float)SAMPLE_RATE);
	Mix_Music* temp_music=NULL;
	temp_music=Mix_LoadMUS(path);
	if (temp_music==NULL)
	{
		SDL_Log("Could not load Music  %s\n",Mix_GetError());
		return false;
	}
	//Mix_PlayMusic(temp_music,1);


	SDL_Event event;
	bool quit=false;
	
	int countedFrames=0;
	int no_times=0;

	
	/*
	FILE *music=fopen(path,"rb");
	OggVorbis_File vf;
	int eof=0;
	int current_section;
	if(ov_open(music, &vf, NULL, 0) < 0) 
	{
		SDL_Log("Input does not appear to be an Ogg bitstream.\n");
		
	}
	*/  
	int	counter=0;
	char pcmout[4096];
	long left,right;
	float sample;
	float power;
	int temptot=0;
	char *samples_buffer=(char*)malloc(NO_OF_CHANNELS*SAMPLES_IN_HANN_WINDOW*sizeof(char)*2);			//2 BYTES PER SAMPLE
	//char *secondary_buffer=(char*)malloc(NO_OF_CHANNELS*SAMPLES_IN_HANN_WINDOW*sizeof(char)*2);			//2 BYTES PER SAMPLE

	float *sample_for_fft=(float*)malloc(SAMPLES_IN_HANN_WINDOW*sizeof(float));
	kiss_fft_cpx *output=(kiss_fft_cpx*)malloc((SAMPLES_IN_HANN_WINDOW/2+1)*sizeof(kiss_fft_cpx));
    
	kiss_fftr_cfg test_cfg=kiss_fftr_alloc(SAMPLES_IN_HANN_WINDOW,0,NULL,NULL);
	

	
	GAME_START_TIME=SDL_GetTicks();
	int LAST_UPDATE=GAME_START_TIME,CURRENT_TIME=0;
	SDL_Log("\n");
	
	int music_on=false;;
	mynode *temp=circles.beginning;
	int number=0;
	while (temp!=NULL)
	{
		temp=temp->next;
		number++;
	}
	SDL_Log("Number of Circles : %d\n",number);
	
	GAME_START_TIME=SDL_GetTicks();
	int LAST_CIRCLE=GAME_START_TIME,flag=0;
	SDL_Log("\n");
	while (quit!=true)
	{
		if (music_on==false && SDL_GetTicks()>GAME_START_TIME+TIME_CIRCLE_ON_SCREEN)
		{
			Mix_PlayMusic(temp_music,1);
			music_on=true;
			SDL_Log("Music Started\n");
		}
		float avgFPS = countedFrames / ( (SDL_GetTicks()-GAME_START_TIME) / 1000.f ); 
		if ( avgFPS > 2000000 )
		{ 
			avgFPS = 0; 
		}
		//SDL_Log("\r%f %d",avgFPS,countedFrames);
		if ((circles.beginning==NULL || circles.beginning->next==NULL) && effects.beginning==NULL && flag==0)
		{
			flag=1;
			LAST_CIRCLE=SDL_GetTicks();
			SDL_Log("LAST CIRCLE Exiting Started : %d     %d\n",LAST_CIRCLE,GAME_START_TIME);
		}
		else if (flag==1)
		{
			if (SDL_GetTicks()-LAST_CIRCLE>5000)
				quit=true;
		}
		while(SDL_PollEvent(&event)!=0)
		{
			if (event.type==SDL_QUIT)
			{
				quit=true;
			}
			else if (event.type==SDL_MOUSEBUTTONDOWN)
			{
				int x,y;
				SDL_GetMouseState(&x,&y);
				checkCircleClick(x,y);
			}
		}
		
		SDL_RenderClear(renderer);
		SDL_SetRenderDrawColor(renderer,0x00,0x00,0x00,0xFF);

		renderEffects();
		renderInRect(renderer,gray,0,0.9*SCREEN_HEIGHT,SCREEN_WIDTH,0.02*SCREEN_HEIGHT);
		renderCircles();
		SDL_RenderPresent(renderer);
		 ++countedFrames;
	}
	
	/*
	while (quit!=true)
	{
		
		float avgFPS = countedFrames / ( (SDL_GetTicks()-GAME_START_TIME) / 1000.f ); 
		if ( avgFPS > 2000000 )
		{ 
			avgFPS = 0; 
		}
		//SDL_Log("\r%f %d",avgFPS,countedFrames);
		
		CURRENT_TIME=SDL_GetTicks()+tmp;
		long ret;
		//int lag=tmp*10;
		if (CURRENT_TIME-LAST_UPDATE>time_increment/OVERLAPS)
		{
			counter%=SAMPLES_IN_HANN_WINDOW*NO_OF_CHANNELS*2;
			memmove(samples_buffer,samples_buffer+SAMPLES_IN_HANN_WINDOW/OVERLAPS,counter+SAMPLES_IN_HANN_WINDOW*2*(1-1.0/OVERLAPS));
			counter+=SAMPLES_IN_HANN_WINDOW*NO_OF_CHANNELS*2*(1-1.0/OVERLAPS);
					
			LAST_UPDATE=CURRENT_TIME;
			SDL_RenderClear(renderer);
			SDL_SetRenderDrawColor(renderer,0x00,0x00,0x00,0xFF);
			renderInRect(renderer,red_bar,0,SCREEN_HEIGHT-2,SCREEN_WIDTH,2);
			++countedFrames;
			no_times++;
		
			//SDL_Log("Time : %ld\t\tSupposed Time : %ld\n",ov_time_tell(&vf),(long int)no_times*tmp/2);

			while (counter<=SAMPLES_IN_HANN_WINDOW*2*NO_OF_CHANNELS && ret>0)
			{	
				//SDL_Log("Entered Once\n");
				ret=ov_read(&vf,pcmout,sizeof(pcmout),&current_section);
				temptot+=ret;
				if (ret == 0)
				{
					// EOF 
					eof=1;
					SDL_Log("EOF\n");
					quit=true;
			
				} 
				else if (ret < 0) 
				{
					// error in the stream.  Not a problem, just reporting it in
					//case we (the app) cares.  In this case, we don't. 
				}
				else
				{
					counter+=ret;
		
					if (counter>=SAMPLES_IN_HANN_WINDOW*NO_OF_CHANNELS*2)
					{
						memcpy((samples_buffer+counter-ret) , pcmout, ret-(counter%SAMPLES_IN_HANN_WINDOW));
						for (int x=0;x<SAMPLES_IN_HANN_WINDOW*NO_OF_CHANNELS*2;x+=2*NO_OF_CHANNELS)
						{
							if (NO_OF_CHANNELS==2)	//CAN BE IMPROVED
							{
								left=(samples_buffer[x]|(samples_buffer[x+1] << 8));
								right=(samples_buffer[x+2]|(samples_buffer[x+3] << 8));
								sample=(left+right)*0.5*HANN_COEFF[x/4];
								sample_for_fft[x/4]=sample;//*HANN_COEFF[x/2];

								//sample_for_fft[x/4].r=sample;
								//sample_for_fft[x/4].i=0;
							}
							else if (NO_OF_CHANNELS==1)
							{
								sample=(samples_buffer[x]|(samples_buffer[x+1] << 8));
								sample_for_fft[x/2]=sample*HANN_COEFF[x/2];

								//sample_for_fft[x/2].r=sample*HANN_COEFF[x/2];
								//sample_for_fft[x/2].i=0;
							}
						}
						kiss_fftr(test_cfg,sample_for_fft,output);
						//for (int j=0;j<SAMPLES_IN_HANN_WINDOW/2+1;j++)
						for (int j=0;j<PLOT_SAMPLES;j++)
						{
							//SDL_Log("Test\n");
							power=pow(output[j].r,2)+pow(output[j].i,2);
							//renderInRect(red_bar,SCREEN_WIDTH*j/SAMPLES_IN_HANN_WINDOW,SCREEN_HEIGHT-SCREEN_HEIGHT*power/GLOB_MAX_POWER,5,SCREEN_HEIGHT*power/GLOB_MAX_POWER);	
							int x=SCREEN_WIDTH*j*2/PLOT_SAMPLES;
							int w=5;
							//float htemp=log10(power)/log10(GLOB_MAX_POWER);
							float htemp=log10(power/VREF);
							int h=SCREEN_HEIGHT*htemp;
							int y=SCREEN_HEIGHT-h;

						
							//x=10;
							//h=100;
							//y=SCREEN_HEIGHT-100;
							
							renderInRect(renderer,red_bar,x,y,w,h);
						}

						//printf("Counter %d\n",counter);
						}
					else
					{
						memcpy( (samples_buffer+counter-ret) , pcmout, ret);
					}			
				}
			}
			SDL_RenderPresent(renderer);
			//SDL_Log("Exited Once\n");
		}
		while(SDL_PollEvent(&event)!=0)
		{
			if (event.type==SDL_QUIT)
			{
				quit=true;
			}
			else if (event.type==SDL_MOUSEBUTTONDOWN)
			{
				int x,y;
				SDL_GetMouseState(&x,&y);
				//checkCircleClick(x,y);
			}
		}
	
	}
	*/
	SDL_Log("\nFinished Game\n");
	free(HANN_COEFF);
	HANN_COEFF=NULL;
	free(samples_buffer);
	free(sample_for_fft);
	free(output);	

	return true;
}



int main(int argc, char* args[] )
{	
	init_SDLGame(&renderer,&window, &SCREEN_WIDTH, &SCREEN_HEIGHT);
	if (renderer==NULL)
	{
		SDL_Log("NULL renderer\n");
		exit(0);
	}
	loadAllTextures();
	SDL_Log("Parameter 1 : %s\n",args[0]);
	SDL_Log("Parameter 2 : %s\n",args[1]);
	strcpy(MUSIC_PATH,args[1]);
	char path[200]="\0";
	strcpy(path,MUSIC_PATH);

	//SDL_RenderClear(renderer);
	//SDL_SetRenderDrawColor(renderer,0x00,0x00,0x00,0xFF);
	//renderInRect(renderer,gray,0,0.9*SCREEN_HEIGHT,SCREEN_WIDTH,0.02*SCREEN_HEIGHT);
	//SDL_RenderPresent(renderer);
	//SDL_Delay(3000);
	
	start_game(path);
	close_cleanup();	
	return 0;
}
