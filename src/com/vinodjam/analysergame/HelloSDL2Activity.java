package com.vinodjam.analysergame;

import org.libsdl.app.SDLActivity; 
import android.content.Intent;

public class HelloSDL2Activity extends SDLActivity
{
	@Override
	protected String[] getArguments() {
         Intent intent = getIntent();

        if (intent != null) {
           
            String filepath = intent.getStringExtra("file_path");
            if (filepath != null) {
		 String[] filename = filepath.split("   ");
                return filename;
            }
            
        }
        return new String[0];
	}
	
}
