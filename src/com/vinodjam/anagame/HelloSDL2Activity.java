package com.vinodjam.anagame;

import org.libsdl.app.SDLActivity; 
import android.content.Intent;
import android.media.MediaPlayer;
import android.net.Uri;

public class HelloSDL2Activity extends SDLActivity
{
	private static MediaPlayer mplayer;
	
	@Override
	protected String[] getArguments() {
         Intent intent = getIntent();

        if (intent != null) {
           
	    initMediaPlayer();
	    //playMusic();
            String filepath = intent.getStringExtra("file_path");
            if (filepath != null) {
		 String[] filename = filepath.split("   ");
                return filename;
            }
            
        }
        return new String[0];
	}
	
	private void initMediaPlayer()
	{
		Intent intent=getIntent();
		if (intent != null) 
		{
			String filePath=intent.getStringExtra("file_path");
			mplayer = MediaPlayer.create(this,Uri.parse(filePath));
			//mplayer.prepare();
			//mplayer.start();
		}
	
	}
	
	private static void playMusic()
	{
		mplayer.start();
	}
	
	private static void pauseMusic()
	{
		mplayer.pause();
	}
	
	private static void cleanUpMusic()
	{
		mplayer.stop();
		mplayer.release();
	}
	
}
