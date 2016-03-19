package com.mcntech.rtspplayer;

import com.bfrx.fcvdst.R;

import android.app.Activity;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.KeyEvent;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.TextView.OnEditorActionListener;
import android.view.View.OnClickListener;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;

public class Settings extends Activity implements OnItemSelectedListener {
	
	CheckBox mEnAudCheckBox;
	CheckBox mEnVidCheckBox;
	CheckBox mEnAutoStartCheckBox;
	CheckBox mEnLogoCheckBox;
	CheckBox mEnStatsCheckBox;
	/*EditText mAudDealy;*/
    @Override
    protected void onCreate(Bundle savedInstanceState) {
    	int i;
    	super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_settings);
		boolean isSystemApp = (getApplicationInfo().flags
				  & (ApplicationInfo.FLAG_SYSTEM | ApplicationInfo.FLAG_UPDATED_SYSTEM_APP)) != 0;
        Configure.loadSavedPreferences(this, isSystemApp);
        
        /* Audio Enable */
        mEnAudCheckBox = (CheckBox) findViewById(R.id.enable_audio);
        mEnAudCheckBox.setOnClickListener(new OnClickListener() {
		        public void onClick(View v) {
		        	Configure.mEnableAudio = mEnAudCheckBox.isChecked();
		        	Configure.savePreferences(getApplicationContext(), Configure.KEY_ENABLE_AUDIO, Configure.mEnableAudio);
		        }
        	});
        mEnAudCheckBox.setChecked(Configure.mEnableAudio);
        
        /* Video Enable */
/*
        mEnVidCheckBox = (CheckBox) findViewById(R.id.enable_video);
        mEnVidCheckBox.setOnClickListener(new OnClickListener() {
		        public void onClick(View v) {
		        	 Configure.mEnableVideo = mEnVidCheckBox.isChecked();
		        	 Configure.savePreferences(getApplicationContext(), Configure.KEY_ENABLE_VIDEO, Configure.mEnableVideo);
		        }
        	}); 
        mEnVidCheckBox.setChecked(Configure.mEnableVideo);
*/
        /* Auto Start Enable */
        mEnAutoStartCheckBox = (CheckBox) findViewById(R.id.enable_auto_start);
        mEnAutoStartCheckBox.setOnClickListener(new OnClickListener() {
		        public void onClick(View v) {
		        	 Configure.mEnableAutoStart = mEnAutoStartCheckBox.isChecked();
		        	 Configure.savePreferences(getApplicationContext(), Configure.KEY_ENABLE_AUTO_START, Configure.mEnableAutoStart);
		        }
        	}); 
        mEnAutoStartCheckBox.setChecked(Configure.mEnableAutoStart);

        /* Logo Enable */
        mEnLogoCheckBox = (CheckBox) findViewById(R.id.enable_logo);
        mEnLogoCheckBox.setOnClickListener(new OnClickListener() {
		        public void onClick(View v) {
		        	 Configure.mEnableLogo = mEnLogoCheckBox.isChecked();
		        	 Configure.savePreferences(getApplicationContext(), Configure.KEY_ENABLE_LOGO, Configure.mEnableLogo);
		        }
        	}); 
        mEnLogoCheckBox.setChecked(Configure.mEnableLogo);
        
        /* Stats Enable */
        mEnStatsCheckBox = (CheckBox) findViewById(R.id.enable_stats);
        mEnStatsCheckBox.setOnClickListener(new OnClickListener() {
		        public void onClick(View v) {
		        	 Configure.mEnableStats = mEnStatsCheckBox.isChecked();
		        	 Configure.savePreferences(getApplicationContext(), Configure.KEY_ENABLE_STATS, Configure.mEnableStats);
		        }
        	}); 
        mEnStatsCheckBox.setChecked(Configure.mEnableStats);
        
/*        
        mAudDealy = (EditText) findViewById(R.id.editAudDelay);
        mAudDealy.setText(String.valueOf(Configure.mAudioDelay));
        mAudDealy.setOnEditorActionListener( new DoneOnEditorActionListener());
*/        
     }

     class DoneOnEditorActionListener implements OnEditorActionListener {
			@Override
			public boolean onEditorAction(TextView v, int actionId,
					KeyEvent event) {
               if (actionId == EditorInfo.IME_ACTION_DONE) {
                   InputMethodManager imm = (InputMethodManager)v.getContext().getSystemService(Context.INPUT_METHOD_SERVICE);
                   imm.hideSoftInputFromWindow(v.getWindowToken(), 0);
            	   if(v.getText().length() > 0) {
            		   Configure.mAudioDelay = Integer.parseInt(v.getText().toString());
            		   Configure.savePreferences(getApplicationContext(), Configure.KEY_AUDIO_DELAY, Configure.mAudioDelay);
            	   }
                   return true;
               }
				return false;
			}
     }    
	public void selfRestart(View v) {
		System.exit(2);
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