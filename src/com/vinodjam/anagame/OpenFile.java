package com.vinodjam.anagame;
import java.io.File;
import java.io.Serializable;

import android.app.Activity;
import android.app.Dialog;
import android.content.Intent;
import android.os.Bundle;
import android.os.Environment;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.Toast;
import ar.com.daidalos.afiledialog.*;
import android.util.Log;

public class OpenFile extends Activity {
    /** Called when the activity is first created. */
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
            Intent intent = new Intent(OpenFile.this, FileChooserActivity.class);
            Log.v("OpenFile", "Intent Created ");// + FilePath);
            intent.putExtra(FileChooserActivity.INPUT_START_FOLDER, Environment.getExternalStorageDirectory() + "/Music/");
            // Define the filter for select images.
            intent.putExtra(FileChooserActivity.INPUT_REGEX_FILTER, ".*MP3|.*OGG|.*mp3|.*ogg|");
            
    		// Call the activity            
            OpenFile.this.startActivityForResult(intent, 0);  
    	}
	};
    
	
	
	
	// ---- Methods for display the results ----- //
	
	private FileChooserDialog.OnFileSelectedListener onFileSelectedListener = new FileChooserDialog.OnFileSelectedListener() {
		public void onFileSelected(Dialog source, File file) {
			source.hide();
			Log.v("OpenFile", "1File Path is : " + file.getName());
			Toast toast = Toast.makeText(OpenFile.this, "File selected: " + file.getName(), Toast.LENGTH_LONG);
			toast.show();
		}
		public void onFileSelected(Dialog source, File folder, String name) {
			source.hide();
			Log.v("OpenFile", "2File Path is : " + folder.getName());
			Toast toast = Toast.makeText(OpenFile.this, "File created: " + folder.getName() + "/" + name, Toast.LENGTH_LONG);
			toast.show();
		}
	};
	
	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
	    if (resultCode == Activity.RESULT_OK) {
	    	boolean fileCreated = false;
	    	String filePath = "";
	    	
	    	Bundle bundle = data.getExtras();
	        if(bundle != null)
	        {
	        	if(bundle.containsKey(FileChooserActivity.OUTPUT_NEW_FILE_NAME)) {
	        		fileCreated = true;
	        		File folder = (File) bundle.get(FileChooserActivity.OUTPUT_FILE_OBJECT);
	        		String name = bundle.getString(FileChooserActivity.OUTPUT_NEW_FILE_NAME);
	        		filePath = folder.getAbsolutePath() + "/" + name;
	        	} else {
	        		fileCreated = false;
	        		File file = (File) bundle.get(FileChooserActivity.OUTPUT_FILE_OBJECT);
	        		filePath = file.getAbsolutePath();
	        	}
	        }
	    	
	        String message = fileCreated? "File created" : "File opened";
	        message += ": " + filePath;
	        Log.v("OpenFile", "3File Path is : " + filePath);
	    	Toast toast = Toast.makeText(OpenFile.this, message, Toast.LENGTH_LONG);
			toast.show();
		Intent intent = new Intent(OpenFile.this, HelloSDL2Activity.class);
		intent.putExtra("file_path",filePath);
		
		OpenFile.this.startActivity(intent);
	    }
	}
}
