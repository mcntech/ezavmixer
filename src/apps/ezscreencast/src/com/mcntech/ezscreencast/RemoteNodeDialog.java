package com.mcntech.ezscreencast;

import android.app.Activity;
import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.CheckBox;
import android.widget.Spinner;
import java.util.ArrayList;
import java.util.List;
import android.view.View.OnClickListener;
import com.mcntech.ezvidcast.R;

public class RemoteNodeDialog extends Activity implements OnItemSelectedListener {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
    	int i;
    	super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_settings);
        ConfigDatabase.loadSavedPreferences(this, false);
        
		ConfigDatabase.mUrl
        
        mBtnAddNode = (Button) findViewById(R.id.add_node);
        mBtnDelNode = (Button) findViewById(R.id.add_node);
        mBtnEditNode = (Button) findViewById(R.id.add_node);

		mEnVidCheckBox.setOnClickListener(new OnClickListener() {
		        public void onClick(View v) {
		        	 ConfigDatabase.mEnableVideo = mEnVidCheckBox.isChecked();
		        	 ConfigDatabase.savePreferences(getApplicationContext(), ConfigDatabase.KEY_ENABLE_VIDEO, ConfigDatabase.mEnableVideo);
		        }
        	}); 

        mEnVidCheckBox.setChecked(ConfigDatabase.mEnableVideo);
    }

    
	@Override
	public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2,
			long arg3) {
		 switch (arg0.getId()) {
		 	case R.id.Add:
		 	{
		 		int position = arg0.getSelectedItemPosition();
		 		String latencny = arg0.getAdapter().getItem(position).toString();
		 		ConfigDatabase.mLatency = Integer.parseInt(latencny);
		 		ConfigDatabase.savePreferences(getApplicationContext(), ConfigDatabase.KEY_LATENCY, ConfigDatabase.mLatency);
		 	}
		 	break;
		 	case R.id.Edit:
		 	{
		 		int position = arg0.getSelectedItemPosition();
		 		String bitrate = arg0.getAdapter().getItem(position).toString();
		 		ConfigDatabase.mVideoBitrate = Integer.parseInt(bitrate) * BITRATE_MBPS;
		 		ConfigDatabase.savePreferences(getApplicationContext(), ConfigDatabase.KEY_VIDEO_BITRATE, ConfigDatabase.mVideoBitrate);
		 	}
		 	break;	
		 	case R.id.Del:
		 	{
		 		int position = arg0.getSelectedItemPosition();
		 		String value = arg0.getAdapter().getItem(position).toString();
		 		ConfigDatabase.mVideoResolution = value;
		 		ConfigDatabase.savePreferences(getApplicationContext(), ConfigDatabase.KEY_VIDEO_RESOLUTION, ConfigDatabase.mVideoResolution);
		 	}
		 	break;				 	
		 }
	}

	@Override
	public void onNothingSelected(AdapterView<?> arg0) {
		// TODO Auto-generated method stub
		
	}

	public void selfRestart(View v) {
		System.exit(2);
	}
}
