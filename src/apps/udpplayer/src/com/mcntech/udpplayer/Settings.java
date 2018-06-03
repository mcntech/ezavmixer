package com.mcntech.udpplayer;


import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.KeyEvent;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.TextView.OnEditorActionListener;
import android.view.View.OnClickListener;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;

public class Settings extends Activity implements OnItemSelectedListener {
	
	CheckBox mEnAudSpectCheckBox;
	CheckBox mEnStatsCheckBox;
	EditText mUrl;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
    	int i;
    	super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_settings);
		boolean isSystemApp = (getApplicationInfo().flags
				  & (ApplicationInfo.FLAG_SYSTEM | ApplicationInfo.FLAG_UPDATED_SYSTEM_APP)) != 0;
        Configure.loadSavedPreferences(this, isSystemApp);
        
        /* Audio Enable */
        mEnAudSpectCheckBox = (CheckBox) findViewById(R.id.enable_audio_spectrum);
        mEnAudSpectCheckBox.setOnClickListener(new OnClickListener() {
		        public void onClick(View v) {
		        	Configure.mEnableAudioSpect = mEnAudSpectCheckBox.isChecked();
		        	Configure.savePreferences(getApplicationContext(), Configure.KEY_ENABLE_AUDIO, Configure.mEnableAudioSpect);
		        }
        	});
        mEnAudSpectCheckBox.setChecked(Configure.mEnableAudioSpect);

		Button browseBtn = (Button)findViewById(R.id.select_file);
		browseBtn.requestFocus();

        /* Stats Enable */
        mEnStatsCheckBox = (CheckBox) findViewById(R.id.enable_stats);
        mEnStatsCheckBox.setOnClickListener(new OnClickListener() {
		        public void onClick(View v) {
		        	 Configure.mEnableStats = mEnStatsCheckBox.isChecked();
		        	 Configure.savePreferences(getApplicationContext(), Configure.KEY_ENABLE_STATS, Configure.mEnableStats);
		        }
        	}); 
        mEnStatsCheckBox.setChecked(Configure.mEnableStats);

        mUrl = (EditText) findViewById(R.id.editUrl);
        mUrl.setText(String.valueOf(Configure.mUrl));
        mUrl.setOnEditorActionListener( new DoneOnEditorUrlListener());
        
     }

     class DoneOnEditorUrlListener implements OnEditorActionListener {
			@Override
			public boolean onEditorAction(TextView v, int actionId,
					KeyEvent event) {
               if (actionId == EditorInfo.IME_ACTION_DONE) {
                   InputMethodManager imm = (InputMethodManager)v.getContext().getSystemService(Context.INPUT_METHOD_SERVICE);
                   imm.hideSoftInputFromWindow(v.getWindowToken(), 0);
            	   if(v.getText().length() > 0) {
            		   Configure.mUrl = v.getText().toString();
            		   Configure.savePreferences(getApplicationContext(), Configure.KEY_URL, Configure.mUrl);
            	   }
                   return true;
               }
				return false;
			}
     }    

	public void selfRestart(View v) {
		/*System.exit(2);*/
		//startSinglePlayer();
	}

	public void Start(View v) {
		Intent intent = new Intent(this, MainActivity.class);
    	Bundle b = new Bundle();
		b.putString("url", Configure.mUrl);
		b.putInt("option", 1);
		intent.putExtras(b);
		startActivity(intent);
		finish();
	}

	public void Browse(View v) {
		Intent intent = new Intent(this, MainActivity.class);
		startActivity(intent);
		finish();
	}
	@Override
	public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2,
			long arg3) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void onNothingSelected(AdapterView<?> arg0) {
		// TODO Auto-generated method stub
		
	}
}
