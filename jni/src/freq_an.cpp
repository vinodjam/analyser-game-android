#include <iostream>

#include "../HelloSDL2Activity.h"
#include <jni.h>
#include <android/log.h>

#include <string.h>
#include <sstream>

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
#define APPROX_TIME_DIVISIONS 8
#define TIME_CIRCLE_ON_SCREEN 4000
#define TIME_EFFECT_ON_SCREEN 1000
#define OVERLAPS 2

int SAMPLES_IN_HANN_WINDOW=-1;
int NO_OF_CHANNELS=-1;
long int SAMPLE_RATE=-1;

int CURR_SCORE=0;
int MAX_SCORE=-1;

char MUSIC_PATH[200]="\0";// "/storage/sdcard0/Music/Wesnoth/siege_of_laurelmor.ogg"


using namespace std; //REMOVE*******************

SDL_Window *window=NULL;
SDL_Renderer *renderer=NULL;

TTF_Font *font = NULL;

int SCREEN_WIDTH = 800;
int SCREEN_HEIGHT = 640;
int GAME_START_TIME=-1;
int SONG_TIME=-1;
int my_index=0;
int PLOT_SAMPLES=-1;

int PAUSED_TICKS=0;

float GLOB_MAX_POS=0,GLOB_MAX_POWER=0,GLOB_MAX_POWER_SQ=0,MIN_POS=-1;

float VREF=0;

float *HANN_COEFF=NULL;

myQueue circles,effects;

SDL_Texture *red_circle=NULL, *red_bar=NULL, *black_circle=NULL, *background=NULL, *gray=NULL, *score=NULL;
SDL_Texture *blue_circle=NULL, *green_circle=NULL, *yellow_circle=NULL, *white=NULL;

SDL_Color textColor = { 255, 255, 255 };

void music_play_jni()
{
	jint temp=4098;//????
	JNIEnv *env = (JNIEnv*)SDL_AndroidGetJNIEnv();//Android_JNI_GetEnv();
	jobject jni_activity = (jobject)SDL_AndroidGetActivity();
	jclass jni_class(env->GetObjectClass(jni_activity));
	//JNIEnv *env = GetEnv();
	//jmethodID mid = (*env)->GetStaticMethodID(env, "HelloSDL2Activity", "playMusic", "V");
	jmethodID mid = (env)->GetStaticMethodID(jni_class,"playMusic", "()V");
	SDL_Log("2) act %08x", (unsigned int)jni_activity);
	SDL_Log("1) env %08x", (unsigned int)env);
	SDL_Log("3) cls %08x", (unsigned int)jni_class);
	SDL_Log("5) met %08x", (unsigned int)mid);
	(env)->CallStaticVoidMethod(jni_class, mid);
	
	env->DeleteLocalRef(jni_activity);
	env->DeleteLocalRef(jni_class);
}

void music_pause_jni()
{
	jint temp=4098;//????
	JNIEnv *env = (JNIEnv*)SDL_AndroidGetJNIEnv();//Android_JNI_GetEnv();
	SDL_Log("1) env %08x", (unsigned int)env);
	jobject jni_activity = (jobject)SDL_AndroidGetActivity();
	SDL_Log("2) act %08x", (unsigned int)jni_activity);
	jclass jni_class(env->GetObjectClass(jni_activity));
	SDL_Log("3) cls %08x", (unsigned int)jni_class);
	//JNIEnv *env = GetEnv();
	//jmethodID mid = (*env)->GetStaticMethodID(env, "HelloSDL2Activity", "playMusic", "V");
	jmethodID mid = (env)->GetStaticMethodID(jni_class,"pauseMusic", "()V");
	 SDL_Log("5) met %08x", (unsigned int)mid);
	(env)->CallStaticVoidMethod(jni_class, mid);
	
	env->DeleteLocalRef(jni_activity);
	env->DeleteLocalRef(jni_class);
}

void music_cleanup_jni()
{
	jint temp=4098;//????
	JNIEnv *env = (JNIEnv*)SDL_AndroidGetJNIEnv();//Android_JNI_GetEnv();
	jobject jni_activity = (jobject)SDL_AndroidGetActivity();
	jclass jni_class(env->GetObjectClass(jni_activity));
	//JNIEnv *env = GetEnv();
	//jmethodID mid = (*env)->GetStaticMethodID(env, "HelloSDL2Activity", "playMusic", "V");
	jmethodID mid = (env)->GetStaticMethodID(jni_class,"cleanUpMusic", "()V");
	(env)->CallStaticVoidMethod(jni_class, mid);
	
	env->DeleteLocalRef(jni_activity);
	env->DeleteLocalRef(jni_class);
}



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
	SDL_Log("Cleaning Up \n");
	SDL_DestroyTexture(background);
	background=NULL;
	SDL_DestroyTexture(red_circle);
	red_circle=NULL;
	SDL_DestroyTexture(yellow_circle);
	yellow_circle=NULL;
	SDL_DestroyTexture(blue_circle);
	blue_circle=NULL;
	SDL_DestroyTexture(green_circle);
	green_circle=NULL;
	SDL_DestroyTexture(black_circle);
	black_circle=NULL;
	SDL_DestroyTexture(gray);
	gray=NULL;
	SDL_DestroyTexture(white);
	white=NULL;
	TTF_CloseFont(font);
	font=NULL;
	SDL_DestroyRenderer(renderer);
	renderer=NULL;
	SDL_DestroyWindow(window);
	window=NULL;
	Mix_CloseAudio();
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
		SDL_Log("Could not load Red Circle\n");
	}
	if (loadTexture(renderer,blue_circle,"Resources/Pictures/DarkBlueCircle.png")!=true)
	{
		SDL_Log("Could not load DarkBlue Circle\n");
	}
	if (loadTexture(renderer,green_circle,"Resources/Pictures/GreenCircle.png")!=true)
	{
		SDL_Log("Could not load Green Circle\n");
	}
	if (loadTexture(renderer,yellow_circle,"Resources/Pictures/YellowCircle.png")!=true)
	{
		SDL_Log("Could not load Yellow Circle\n");
	}
	if (loadTexture(renderer,black_circle,"Resources/Pictures/BlackCircle.png")!=true)
	{
		SDL_Log("Could not load Black Circle\n");
	}
	if (loadTexture(renderer,gray,"Resources/Pictures/Gray.png")!=true)
	{
		SDL_Log("Could not load Gray\n");
	}
	if (loadTexture(renderer,white,"Resources/Pictures/White.png")!=true)
	{
		SDL_Log("Could not load White\n");
	}
	if (loadTexture(renderer,red_bar,"Resources/Pictures/RedBox.png")!=true)
	{
		SDL_Log("Could not load Red Box\n");
	}
	
	font = TTF_OpenFont( "Resources/Data/WorkSans-SemiBold.ttf", 28 );
	if( font == NULL )
	{
		printf( "Failed to load lazy font! SDL_ttf Error: %s\n", TTF_GetError() );
	}
}
	
	

void renderCircles()
{
	mynode *temp=circles.beginning;
	for (int i=0;i<30 && temp!=NULL;i++)
	{
		if ( (temp->circle.time_spawn+GAME_START_TIME+PAUSED_TICKS) > (SDL_GetTicks()+500) )
		{
			temp=temp->next;

			//NOT YET TIME
		}
		else if ( (temp->circle.time_spawn+GAME_START_TIME + TIME_CIRCLE_ON_SCREEN + 3000 +PAUSED_TICKS) < (SDL_GetTicks()) ) //3000 is buffer time
		{
			temp=temp->next;
			circles.remove_element();
		}
		else
		{
			temp->circle.circle_render(renderer,red_circle,blue_circle,green_circle,yellow_circle, SCREEN_HEIGHT, GAME_START_TIME,TIME_CIRCLE_ON_SCREEN, PAUSED_TICKS);
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
			if ((temp->circle.time_spawn+GAME_START_TIME+TIME_EFFECT_ON_SCREEN+PAUSED_TICKS)<(SDL_GetTicks()))
			{
				temp=temp->next;
				effects.remove_element();
			}
			else
			{
				temp->circle.effect_render(renderer,black_circle,red_circle,blue_circle,green_circle,yellow_circle,SCREEN_HEIGHT, GAME_START_TIME,TIME_EFFECT_ON_SCREEN, PAUSED_TICKS);
				temp=temp->next;

			}
		} 
	}
	//else cout<<"EMPTY"<<endl;
}

int pauseNow()
{
	//Mix_PauseMusic();
	music_pause_jni();
	std::stringstream texText;
	texText.str("");
	texText<<"RESUME";
	std::stringstream texText2;
	texText2.str("");
	texText2<<"QUIT";
	SDL_Surface *resumesurf=TTF_RenderText_Solid( font, texText.str().c_str(), textColor );
	SDL_Surface *quitsurf=TTF_RenderText_Solid( font, texText2.str().c_str(), textColor );
	SDL_Texture *resumet = SDL_CreateTextureFromSurface( renderer, resumesurf );
	SDL_Texture *quitt = SDL_CreateTextureFromSurface( renderer, quitsurf );
	SDL_FreeSurface( resumesurf );
	SDL_FreeSurface( quitsurf );
	if (resumet==NULL)
		SDL_Log("Texture Error Text\n");
		
		
	int now=SDL_GetTicks();
	SDL_Event eventpause;
	SDL_RenderClear(renderer);
	SDL_SetRenderDrawColor(renderer,0x00,0x00,0x00,0xFF);
	renderInRect(renderer,green_circle,0,0,SCREEN_WIDTH, SCREEN_HEIGHT/2);
	renderInRect(renderer,red_circle,0,SCREEN_HEIGHT/2,SCREEN_WIDTH, SCREEN_HEIGHT/2);
	
	renderInRect(renderer,resumet,SCREEN_WIDTH/4,SCREEN_HEIGHT/6,SCREEN_WIDTH/2,SCREEN_HEIGHT/4);
	renderInRect(renderer,quitt,SCREEN_WIDTH/4,4*SCREEN_HEIGHT/6,SCREEN_WIDTH/2,SCREEN_HEIGHT/4);
		
	SDL_RenderPresent(renderer);
	
	bool resume=false;
	while (resume!=true)
	{
		while(SDL_PollEvent(&eventpause)!=0)
		{
			if (eventpause.type==SDL_MOUSEBUTTONDOWN)
			{
				int x,y;
				SDL_GetMouseState(&x,&y);
				if (y<SCREEN_HEIGHT/2)
				{
					PAUSED_TICKS+=SDL_GetTicks()-now;
					resume=true;
					music_play_jni();
					//Mix_ResumeMusic();
					SDL_DestroyTexture(resumet);
					resumet=NULL;	
					SDL_DestroyTexture(quitt);
					quitt=NULL;	
					return 0;
				}
				else if (y>SCREEN_HEIGHT/2)
				{
					SDL_DestroyTexture(resumet);
					resumet=NULL;	
					SDL_DestroyTexture(quitt);
					quitt=NULL;	
					return 1;
				}
			}
		}
	}
	return 0;
	
}
	
int checkCircleClick(int x,int y)
{
	if (x>SCREEN_WIDTH-200 && y<150)
	{
		if (pauseNow()==1)
			return 1;
	}
	else
	{
		mynode *temp=circles.beginning;
		for (int i=0;i<15 && temp!=NULL;temp=temp->next,i++)
		{
			if ((temp->circle.x_pos-temp->circle.radius)<x && (temp->circle.x_pos+temp->circle.radius)>x)
			{			
				
				int timediff=SDL_GetTicks()-GAME_START_TIME-temp->circle.time_spawn-PAUSED_TICKS;
				int new_ypos=temp->circle.y_pos+ ( ( (SDL_GetTicks()-GAME_START_TIME-temp->circle.time_spawn-PAUSED_TICKS) * SCREEN_HEIGHT )/ TIME_CIRCLE_ON_SCREEN );
				//SDL_Log("Y POS %d\n",new_ypos);
				if (new_ypos+temp->circle.radius>0.9*SCREEN_HEIGHT && timediff>0.8*TIME_CIRCLE_ON_SCREEN)
				{
					CURR_SCORE++;
					MusicCircle new_effect=temp->circle;
					
					new_effect.y_pos=new_ypos;
					new_effect.time_spawn=(SDL_GetTicks()-GAME_START_TIME-PAUSED_TICKS);
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
	SAMPLES_IN_HANN_WINDOW=pow(2,(temp1));
	SDL_Log("Sample in a window         : %d\n",SAMPLES_IN_HANN_WINDOW);
	my_index=0;
	//SAMPLES_IN_HANN_WINDOW=32768;
	char *samples_buffer=(char*)malloc(NO_OF_CHANNELS*SAMPLES_IN_HANN_WINDOW*sizeof(char)*2);		//2 BYTES PER SAMPLE

	HANN_COEFF=(float*)malloc(SAMPLES_IN_HANN_WINDOW*sizeof(float));
	
	float *sample_for_fft=(float*)malloc(SAMPLES_IN_HANN_WINDOW*sizeof(float));
	kiss_fft_cpx *output=(kiss_fft_cpx*)malloc((SAMPLES_IN_HANN_WINDOW/2+1)*sizeof(kiss_fft_cpx));
	
	double LOCAL_VREF[8]={0,0,0,0,0,0,0,0};
	double *output_buffer_2sec[8];
	for (int i=0;i<8;i++)
	{
		output_buffer_2sec[i]=(double*)malloc(OVERLAPS*APPROX_TIME_DIVISIONS*2*sizeof(double));
		for (int j=0;j<OVERLAPS*APPROX_TIME_DIVISIONS*2;j++)
			output_buffer_2sec[i][j]=0;
		LOCAL_VREF[i]=0;
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
					LOCAL_VREF[l]=LOCAL_VREF[l]*LOCAL_VREF[l]*(OVERLAPS*APPROX_TIME_DIVISIONS*2-1)+output_buffer_2sec[l][my_index];
					LOCAL_VREF[l]/=OVERLAPS*APPROX_TIME_DIVISIONS*2;
					//SDL_Log("LocV %f \t L : %d\n",LOCAL_VREF[l],l);
					//SDL_Log("OutB %f \t L : %d \t MI : %d\n",output_buffer_2sec[l][my_index],l,my_index);
					LOCAL_VREF[l]=sqrt(LOCAL_VREF[l]);
					output_buffer_2sec[l][my_index]=sqrt(output_buffer_2sec[l][my_index]);
				}
				float max_inc;
				max_inc=0;
				int l_max=-1;
				for (int l=0;l<8;l++)
				{
					float inc=log10(output_buffer_2sec[l][my_index]/LOCAL_VREF[l]);
					//SDL_Log("Increase : %f\t%f\t%f\n",inc,output_buffer_2sec[l][my_index],LOCAL_VREF[l]);
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
					int colour=1 +rand()%4;
					//colour+=rand(
					int rad=SCREEN_WIDTH/5;
					int y_pos=-0.1*SCREEN_HEIGHT;
					int x_pos=SCREEN_WIDTH/16+l_max*SCREEN_WIDTH/8;
					if (x_pos+rad>SCREEN_WIDTH)
					{
						x_pos-=rad/2;
					}
					MusicCircle temp_circle(rad,colour,x_pos,y_pos,tspawn);
					circles.add_element(temp_circle);
					LAST_CIRCLE=tspawn;
				}		
				my_index++;
				my_index%=OVERLAPS*APPROX_TIME_DIVISIONS*2;//OVERLAPS*APPROX_TIME_DIVISIONS*2
				
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
	SDL_Log("Actual Sample         : %lld\n",ov_pcm_total(&vf,-1));
	SONG_TIME=time_increment*notimes/2;
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
	
	const char **declist=mpg123_supported_decoders();
	SDL_Log("List\n");
	SDL_Log("%s",*declist);
	const char **declist2=mpg123_decoders();
	SDL_Log("List\n");
	SDL_Log("%s",*declist2);
	//SDL_Log("%s",*(declist+1));
	//SDL_Log("%s",*(declist+2));
	
	my_index=0;
	if(err != MPG123_OK || (mh = mpg123_new(NULL, &err)) == NULL)
	{
		SDL_Log( "Basic setup goes wrong: %s", mpg123_plain_strerror(err));
		mpg123_close(mh);
		mpg123_delete(mh);
		mpg123_exit();
		return -1;
	}
	
	SDL_Log("Current %s\n",mpg123_current_decoder(mh));
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
	SAMPLES_IN_HANN_WINDOW=pow(2,(temp1));
	SDL_Log("Sample in a window         : %d\n",SAMPLES_IN_HANN_WINDOW);

	//SAMPLES_IN_HANN_WINDOW=32768;
	char *samples_buffer=(char*)malloc(NO_OF_CHANNELS*SAMPLES_IN_HANN_WINDOW*sizeof(char)*2);		//2 BYTES PER SAMPLE

	HANN_COEFF=(float*)malloc(SAMPLES_IN_HANN_WINDOW*sizeof(float));
	
	float *sample_for_fft=(float*)malloc(SAMPLES_IN_HANN_WINDOW*sizeof(float));
	kiss_fft_cpx *output=(kiss_fft_cpx*)malloc((SAMPLES_IN_HANN_WINDOW/2+1)*sizeof(kiss_fft_cpx));
	
	double LOCAL_VREF[8]={0,0,0,0,0,0,0,0};
	double *output_buffer_2sec[8];
	for (int i=0;i<8;i++)
	{
		output_buffer_2sec[i]=(double*)malloc(OVERLAPS*APPROX_TIME_DIVISIONS*2*sizeof(double));
		for (int j=0;j<OVERLAPS*APPROX_TIME_DIVISIONS*2;j++)
			output_buffer_2sec[i][j]=0;
		LOCAL_VREF[i]=0;
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
		//long unsigned int ret;
		size_t ret;
		err = mpg123_read( mh, pcmout, sizeof(pcmout), &ret );
		
		//ret=ret2;
		//SDL_Log("%lud\t	%zu\n",ret,ret2);
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
				for (int x=0;x<SAMPLES_IN_HANN_WINDOW*2*NO_OF_CHANNELS;x+=2*NO_OF_CHANNELS)
				{
					if (NO_OF_CHANNELS==2)	//CAN BE IMPROVED
					{
						//IF DECODER IS NEON
						//
						//
						left=(samples_buffer[x/2]|(samples_buffer[x/2+1] << 8));
						//right=(samples_buffer[x+2]|(samples_buffer[x+3] << 8));
						right=(samples_buffer[SAMPLES_IN_HANN_WINDOW*NO_OF_CHANNELS+x/2]|(samples_buffer[SAMPLES_IN_HANN_WINDOW*NO_OF_CHANNELS+x/2+1] << 8));
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
					LOCAL_VREF[l]=LOCAL_VREF[l]*LOCAL_VREF[l]*(OVERLAPS*APPROX_TIME_DIVISIONS*2-1)+output_buffer_2sec[l][my_index];
					LOCAL_VREF[l]/=OVERLAPS*APPROX_TIME_DIVISIONS*2;
					LOCAL_VREF[l]=sqrt(LOCAL_VREF[l]);
					output_buffer_2sec[l][my_index]=sqrt(output_buffer_2sec[l][my_index]);
				}
				float max_inc;
				max_inc=0;
				int l_max=-1;
				for (int l=0;l<8;l++)
				{
					float inc=log10(output_buffer_2sec[l][my_index]/LOCAL_VREF[l]);
					if (inc>THRESHOLD)
					{
						//SDL_Log("Increase : %f\n",inc);		
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
					int colour=1 +rand()%4;
					int rad=SCREEN_WIDTH/5;
					int y_pos=-0.1*SCREEN_HEIGHT;
					int x_pos=SCREEN_WIDTH/16+l_max*SCREEN_WIDTH/8;
					if (x_pos+rad>SCREEN_WIDTH)
					{
						x_pos-=rad/2;
					}
					MusicCircle temp_circle(rad,colour,x_pos,y_pos,tspawn);
					circles.add_element(temp_circle);
					LAST_CIRCLE=tspawn;
				}		
				my_index++;
				my_index%=OVERLAPS*APPROX_TIME_DIVISIONS*2;
				
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
	
	if(err != MPG123_DONE)
		SDL_Log( "Warning: Decoding ended prematurely because: %s\n",
	         err == MPG123_ERR ? mpg123_strerror(mh) : mpg123_plain_strerror(err) );

	SDL_Log("Samples read in a window         : %d\n",notimes*SAMPLES_IN_HANN_WINDOW/2);
	mpg123_close(mh);
	mpg123_delete(mh);
	mpg123_exit();
	SONG_TIME=time_increment*notimes/2;
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
	
	
	GLOB_MAX_POWER_SQ=GLOB_MAX_POWER*GLOB_MAX_POWER;
	if (decode_ogg(path)==-3)
		decode_mp3(path);
	//open_ogg(path);
	//prelim_for_creation();
	//create_circles();
	VREF/=100;
	float time_increment=(float)SAMPLES_IN_HANN_WINDOW*float(1000)/float(SAMPLE_RATE);
	int tmp=time_increment;
	SDL_Log("Time Increment %f %d\n%f %f\n",time_increment,tmp,(float)SAMPLES_IN_HANN_WINDOW,(float)SAMPLE_RATE);
	//Mix_Music* temp_music=NULL;
	//temp_music=Mix_LoadMUS(path);
	//if (temp_music==NULL)
	//{
	//	SDL_Log("Could not load Music  %s\n",Mix_GetError());
	//	return false;
	//}
	//Mix_PlayMusic(temp_music,1);
	//if (Mix_PlayingMusic()==0)
	//	SDL_Log("Not Playing :( \n");
	Mix_HaltMusic();
	SDL_Log("Finished Loading\n");
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
	SDL_Log("Before Counting\n");
	
	int music_on=false;;
	mynode *temp=circles.beginning;
	int number=0;
	while (temp!=NULL)
	{
		temp=temp->next;
		number++;
	}
	SDL_Log("Number of Circles : %d\n",number);
	
	MAX_SCORE=number;
	
	GAME_START_TIME=SDL_GetTicks();
	int LAST_CIRCLE=GAME_START_TIME,flag=0;
	SDL_Log("\n");
	
	while (quit!=true)
	{
		if (music_on==false && SDL_GetTicks()>GAME_START_TIME+TIME_CIRCLE_ON_SCREEN )
		{
			//int terr=Mix_PlayMusic(temp_music,1);
			//if (terr==-1)
				//SDL_Log("Music Error : %s\n",Mix_GetError());
			//SDL_Log("Terr %d\n",terr);
			music_play_jni();
			music_on=true;
			//terr=Mix_GetMusicType(NULL);
			/*
			SDL_Log("%d\n",Mix_GetMusicType(NULL));
			SDL_Log("Music Started\n");
			SDL_Log("Could not ?? Music  %s\n",Mix_GetError());
			SDL_Log("volume was    : %d\n", Mix_VolumeMusic(MIX_MAX_VOLUME));
			SDL_Log("volume is now : %d\n", Mix_VolumeMusic(-1));
			*/
			
			
		}
		
	
		
		float avgFPS = countedFrames / ( (SDL_GetTicks()-GAME_START_TIME-PAUSED_TICKS) / 1000.f ); 
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
		else if (flag==1 && (Mix_PlayingMusic()==0))
		{
			if (SDL_GetTicks()-LAST_CIRCLE>5000)
				quit=true;
		}
		/*
		if (music_on==true && (Mix_PlayingMusic()==0))
		{
			Mix_PlayMusic(temp_music,1);
			SDL_Log("Music Error : %s\n",Mix_GetError());
			SDL_Log("Restarted\n");
		}
		*/
		/*
		if (music_on==true && (Mix_PausedMusic()==1))
		{
			Mix_ResumeMusic();
			SDL_Log("Music Error : %s\n",Mix_GetError());
			SDL_Log("Resumed\n");
		}
		*/
		SDL_RenderPresent(renderer);
		++countedFrames;
		
		
		SDL_RenderClear(renderer);
		SDL_SetRenderDrawColor(renderer,0x00,0x00,0x00,0xFF);

		
		
		renderEffects();
		renderInRect(renderer,gray,0,0.9*SCREEN_HEIGHT,SCREEN_WIDTH,0.02*SCREEN_HEIGHT);
		renderCircles();
		
		std::stringstream texText;
		texText.str("");
		texText<<CURR_SCORE<<" / "<<MAX_SCORE;
		SDL_Surface *scoresurf=TTF_RenderText_Solid( font, texText.str().c_str(), textColor );
		score = SDL_CreateTextureFromSurface( renderer, scoresurf );
		SDL_FreeSurface( scoresurf );
		if (score==NULL)
			SDL_Log("Texture Error Text\n");
		renderInRect(renderer,score,0,0,200,80);
		renderInRect(renderer,white,300,30,100,5);
		renderInRect(renderer,white,300 + 100*(SDL_GetTicks()-GAME_START_TIME-PAUSED_TICKS)/SONG_TIME,30, 5 ,20);
		
		renderInRect(renderer,white,SCREEN_WIDTH-100,30,20, 100);
		renderInRect(renderer,white,SCREEN_WIDTH- 60,30,20, 100);
		 
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
				if (checkCircleClick(x,y)==1)
				{	
					quit=true;
					while (circles.beginning!=NULL)
						circles.remove_element();
				}
			}
		}
	}
	
	/*
	counter=0;
	while (quit!=true)
	{
		
		float avgFPS = countedFrames / ( (SDL_GetTicks()-GAME_START_TIME) / 1000.f ); 
		if ( avgFPS > 2000000 )
		{ 
			avgFPS = 0; 
		}
		//SDL_Log("\r%f %d",avgFPS,countedFrames);
		size_t ret=1;
		CURRENT_TIME=SDL_GetTicks()+tmp;
		//long ret;
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
				
				err = mpg123_read( mh, pcmout, sizeof(pcmout), &ret );
				temptot+=ret;
				if (ret == 0)
				{
					// EOF 
					//eof=1;
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
						memcpy((samples_buffer+counter-ret) , pcmout, ret-(counter%(SAMPLES_IN_HANN_WINDOW*NO_OF_CHANNELS*2)));
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
	mpg123_close(mh);
	mpg123_delete(mh);
	mpg123_exit();
	
	free(samples_buffer);
	free(sample_for_fft);
	free(output);	
	for (int i=0;i<8;i++)
		free(output_buffer_2sec[i]);
	*/	
	//Mix_HaltChannel(-1);
	//Mix_FreeMusic( temp_music );
	//temp_music = NULL;
	music_cleanup_jni();
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
	srand(SDL_GetTicks());
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
	CURR_SCORE=0;
	VREF=0;
	PAUSED_TICKS=0;

	std::stringstream texText;
	texText.str("");
	texText<<"LOADING";
	SDL_Surface *scoresurf=TTF_RenderText_Solid( font, texText.str().c_str(), textColor );
	SDL_Texture *loading = SDL_CreateTextureFromSurface( renderer, scoresurf );
	SDL_FreeSurface( scoresurf );
	if (loading==NULL)
		SDL_Log("Texture Error Text\n");
	
	SDL_RenderClear(renderer);
	SDL_SetRenderDrawColor(renderer,0x00,0x00,0x00,0xFF);

	renderInRect(renderer,loading,0,0,SCREEN_WIDTH,SCREEN_HEIGHT/2);
	SDL_RenderPresent(renderer);
	//SDL_Delay(3000);
	
	start_game(path);
	close_cleanup();
	SDL_DestroyTexture(loading);
	loading=NULL;	
	return 0;
}
