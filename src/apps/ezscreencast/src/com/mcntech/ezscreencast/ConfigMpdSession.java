package com.mcntech.ezscreencast;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.AdapterView.OnItemSelectedListener;

public class ConfigMpdSession extends Activity implements OnItemSelectedListener {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
    	int i;
    	Button mBtnSave;
    	super.onCreate(savedInstanceState);
        setContentView(R.layout.mpd_session_dlg);
        mBtnSave = (CheckBox) findViewById(R.id.buttonSave);
        mBtnSave.setOnClickListener(new OnClickListener() {
	        public void onClick(View v) {
	        	ConfigDatabase.savePreferences(getApplicationContext(), ConfigDatabase.KEY_ENABLE_AUDIO, ConfigDatabase.mEnableAudio);
	        }
    	});
    }
	@Override
	public void onItemSelected(AdapterView<?> parent, View view, int position,
			long id) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void onNothingSelected(AdapterView<?> parent) {
		// TODO Auto-generated method stub
		
	}

}
