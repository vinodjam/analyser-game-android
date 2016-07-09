package com.vinodjam.analysergame;

import java.io.File;
import java.io.Serializable;

import android.app.Activity;
import android.app.Dialog;
import android.content.Intent;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.Toast;


public class OpenFile extends Activity {
    /** Called when the activity is first created. */
    private static final int PICKFILE_RESULT_CODE = 1;
    
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main_file);
       
        // Assign behaviors to the buttons.
        Button buttonActivity5 = (Button)this.findViewById(R.id.activity_select_images);
        buttonActivity5.setOnClickListener(btnActivitySelectImages);
       
       
    }
    
    // ----- Buttons for open a dialog ----- //
    
  
    
    private OnClickListener btnActivitySelectImages = new OnClickListener() {
    	public void onClick(View v) {
    		// Create the intent for call the activity.
    		 Intent fileintent = new Intent(Intent.ACTION_GET_CONTENT);
		fileintent.setType("file/*");
		//try {
		startActivityForResult(fileintent,PICKFILE_RESULT_CODE);
		//} 
		//catch (ActivityNotFoundException e) {
		//Log.e("tag", "No activity can handle picking a file. Showing alternatives.");
		//}	
	}

	
	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
	switch(requestCode){
		case PICKFILE_RESULT_CODE:
			if(resultCode==RESULT_OK){
				File file = new File(data.getData().getPath());
				if (file.getPath().contains("storage"))
				{
					String FilePath=file.getPath().replace("/share","");//.replace(" ","\\ ");
					Log.v("OpenFile", "File Path is : " + FilePath);
					Intent intent = new Intent(OpenFile.this, HelloSDL2Activity.class);
					intent.putExtra("file_path",FilePath);
				
					OpenFile.this.startActivity(intent);
				}
				else
				{ 
					String FilePath = Environment.getExternalStorageDirectory()+file.getPath().replace("/share","");//.replace(" ","\\ ");
					Log.v("OpenFile", "File Path is : " + FilePath);
					Intent intent = new Intent(OpenFile.this, HelloSDL2Activity.class);
					intent.putExtra("file_path",FilePath);
				
					OpenFile.this.startActivity(intent);
				}
				
				
			}
		break;
   
		}
	}

           
	
	
}
