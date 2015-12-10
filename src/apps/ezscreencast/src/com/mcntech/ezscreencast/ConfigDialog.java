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
import com.mcntech.ezscreencast.R;

public class ConfigDialog extends Activity implements OnItemSelectedListener {
	
	CheckBox mEnAudCheckBox;
	CheckBox mEnVidCheckBox;
	final int BITRATE_MBPS = 1000000;
	
    @Override
    protected void onCreate(Bundle savedInstanceState) {
    	int i;
    	super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_settings);
		boolean isSystemApp = (getApplicationInfo().flags
				  & (ApplicationInfo.FLAG_SYSTEM | ApplicationInfo.FLAG_UPDATED_SYSTEM_APP)) != 0;
        ConfigDatabase.loadSavedPreferences(this, isSystemApp);
        
        PrepareAudioSourceSelection();
        PrepareBitrateSelection();
        PrepareSegmentDurationSelection();
        PrepareVdeioResolutionSelection();
        
        /* Display a list of checkboxes */
        mEnAudCheckBox = (CheckBox) findViewById(R.id.enable_audio);
        mEnAudCheckBox.setOnClickListener(new OnClickListener() {
		        public void onClick(View v) {
		        	ConfigDatabase.mEnableAudio = mEnAudCheckBox.isChecked();
		        	ConfigDatabase.savePreferences(getApplicationContext(), ConfigDatabase.KEY_ENABLE_AUDIO, ConfigDatabase.mEnableAudio);
		        }
        	});
        mEnAudCheckBox.setChecked(ConfigDatabase.mEnableAudio);
        mEnAudCheckBox.setEnabled(ConfigDatabase.mSystemApp);
        
        /* Display a list of checkboxes */
        mEnVidCheckBox = (CheckBox) findViewById(R.id.enable_video);
        mEnVidCheckBox.setOnClickListener(new OnClickListener() {
		        public void onClick(View v) {
		        	 ConfigDatabase.mEnableVideo = mEnVidCheckBox.isChecked();
		        	 ConfigDatabase.savePreferences(getApplicationContext(), ConfigDatabase.KEY_ENABLE_VIDEO, ConfigDatabase.mEnableVideo);
		        }
        	}); 

        mEnVidCheckBox.setChecked(ConfigDatabase.mEnableVideo);
    }

    private void PrepareSegmentDurationSelection()
    {   
    	int i;
	    Spinner spinner = (Spinner) findViewById(R.id.segment_duration);
	    spinner.setOnItemSelectedListener(this);
	    List<String> duraions = new ArrayList<String>();

	    for(i = 1; i < 30; i=i+1)
	    	duraions.add(Integer.toString(i));
	
	    ArrayAdapter<String> dataAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, duraions);
	    dataAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
	    spinner.setAdapter(dataAdapter);
	    
	    int SpinnerPostion = dataAdapter.getPosition(Integer.toString( ConfigDatabase.mSegmentDuration));
	    spinner.setSelection(SpinnerPostion, false);        
    }
  
    private void PrepareVdeioResolutionSelection()
    {   
	    Spinner spinner = (Spinner) findViewById(R.id.video_resolution);
	    spinner.setOnItemSelectedListener(this);
	    List<String> list = new ArrayList<String>();
	    list.add(ConfigDatabase.VID_RES_480P);
	    list.add(ConfigDatabase.VID_RES_720P);
	    list.add(ConfigDatabase.VID_RES_1080P);
	    list.add(ConfigDatabase.VID_RES_4K);

	    ArrayAdapter<String> dataAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, list);
	    dataAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
	    spinner.setAdapter(dataAdapter);
	    
	    int selectPostion = dataAdapter.getPosition(ConfigDatabase.mVideoResolution);
	    spinner.setSelection(selectPostion, false);        
    }
    
    private void PrepareBitrateSelection()
    {
	    int i;
	    Spinner bitrate_spinner = (Spinner) findViewById(R.id.video_bitrate);
	    bitrate_spinner.setOnItemSelectedListener(this);
	    List<String> bitrates = new ArrayList<String>();
	
	    for(i = 1; i < 10; i=i+1)
	    	bitrates.add(Integer.toString(i));
	    for(i = 10; i < 20; i=i+2)
	    	bitrates.add(Integer.toString(i));	    
	    for(i = 20; i < 60; i=i+10)
	    	bitrates.add(Integer.toString(i));

	
	    ArrayAdapter<String> bitrateAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, bitrates);
	    bitrateAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
	    bitrate_spinner.setAdapter(bitrateAdapter);
	    
	    int bitrateSpinnerPostion = bitrateAdapter.getPosition(Integer.toString( ConfigDatabase.mVideoBitrate / BITRATE_MBPS));
	    bitrate_spinner.setSelection(bitrateSpinnerPostion, false);        
	}

    private void PrepareAudioSourceSelection()
    {
	    int i;
	    Spinner spinner = (Spinner) findViewById(R.id.audio_source);
	    spinner.setOnItemSelectedListener(this);
	    List<String> sources = new ArrayList<String>();
	
	    sources.add(ConfigDatabase.AUDSRC_MIC);
	    sources.add(ConfigDatabase.AUDSRC_SYSTEM_AUDIO);

	
	    ArrayAdapter<String> Adapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, sources);
	    Adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
	    spinner.setAdapter(Adapter);
	    
	    int SpinnerPostion = Adapter.getPosition(ConfigDatabase.mAudioSource);
	    spinner.setSelection(SpinnerPostion, false);        
	}
    
	@Override
	public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2,
			long arg3) {
		 switch (arg0.getId()) {
		 	case R.id.segment_duration:
		 	{
		 		int position = arg0.getSelectedItemPosition();
		 		String latencny = arg0.getAdapter().getItem(position).toString();
		 		ConfigDatabase.mSegmentDuration = Integer.parseInt(latencny);
		 		ConfigDatabase.savePreferences(getApplicationContext(), ConfigDatabase.KEY_SEGMENT_DURATION, ConfigDatabase.mSegmentDuration);
		 	}
		 	break;
		 	case R.id.audio_source:
		 	{
		 		int position = arg0.getSelectedItemPosition();
		 		String audio_source = arg0.getAdapter().getItem(position).toString();
		 		ConfigDatabase.mAudioSource = audio_source;
		 		ConfigDatabase.savePreferences(getApplicationContext(), ConfigDatabase.KEY_AUDIO_SOURCE, ConfigDatabase.mAudioSource);
		 	}
		 	break;
		 	
		 	case R.id.video_bitrate:
		 	{
		 		int position = arg0.getSelectedItemPosition();
		 		String bitrate = arg0.getAdapter().getItem(position).toString();
		 		ConfigDatabase.mVideoBitrate = Integer.parseInt(bitrate) * BITRATE_MBPS;
		 		ConfigDatabase.savePreferences(getApplicationContext(), ConfigDatabase.KEY_VIDEO_BITRATE, ConfigDatabase.mVideoBitrate);
		 	}
		 	break;	
		 	case R.id.video_resolution:
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
