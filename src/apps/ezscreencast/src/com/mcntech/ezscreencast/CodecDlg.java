package com.mcntech.ezscreencast;

import android.app.Activity;
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

public class CodecDlg extends Activity implements OnItemSelectedListener {
	
	CheckBox mEnAudCheckBox;
	CheckBox mEnVidCheckBox;
	CheckBox mIsLiveCheckBox;	
	final int BITRATE_MBPS = 1000000;
	
    @Override
    protected void onCreate(Bundle savedInstanceState) {
    	int i;
    	super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_settings);
		boolean isSystemApp = (getApplicationInfo().flags
				  & (ApplicationInfo.FLAG_SYSTEM | ApplicationInfo.FLAG_UPDATED_SYSTEM_APP)) != 0;
        CodecModel.loadSavedPreferences(this, isSystemApp);
        
        PrepareAudioSourceSelection();
        PrepareBitrateSelection();
        PrepareMuxTypeSelection();
        PrepareVidCodecTypeSelection();
        PrepareAudCodecTypeSelection();        
        PrepareSegmentDurationSelection();
        PrepareVdeioResolutionSelection();
        
        /* Display a list of checkboxes */
        mEnAudCheckBox = (CheckBox) findViewById(R.id.enable_audio);
        mEnAudCheckBox.setOnClickListener(new OnClickListener() {
		        public void onClick(View v) {
		        	CodecModel.mEnableAudio = mEnAudCheckBox.isChecked();
		        	CodecModel.savePreferences(getApplicationContext(), CodecModel.KEY_ENABLE_AUDIO, CodecModel.mEnableAudio);
		        }
        	});
        mEnAudCheckBox.setChecked(CodecModel.mEnableAudio);
        mEnAudCheckBox.setEnabled(CodecModel.mSystemApp);
        
        /* Display a list of checkboxes */
        mEnVidCheckBox = (CheckBox) findViewById(R.id.enable_video);
        mEnVidCheckBox.setOnClickListener(new OnClickListener() {
		        public void onClick(View v) {
		        	 CodecModel.mEnableVideo = mEnVidCheckBox.isChecked();
		        	 CodecModel.savePreferences(getApplicationContext(), CodecModel.KEY_ENABLE_VIDEO, CodecModel.mEnableVideo);
		        }
        	}); 
        mEnVidCheckBox.setChecked(CodecModel.mEnableVideo);
        
        mIsLiveCheckBox = (CheckBox) findViewById(R.id.is_live_stream);
        mIsLiveCheckBox.setOnClickListener(new OnClickListener() {
	        public void onClick(View v) {
	        	 CodecModel.mIsLiveStream = mIsLiveCheckBox.isChecked();
	        	 CodecModel.savePreferences(getApplicationContext(), CodecModel.KEY_IS_LIVE_STREAM, CodecModel.mIsLiveStream);
	        }
    	}); 
        mIsLiveCheckBox.setChecked(CodecModel.mIsLiveStream);
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
	    
	    int SpinnerPostion = dataAdapter.getPosition(Integer.toString( CodecModel.mSegmentDuration));
	    spinner.setSelection(SpinnerPostion, false);        
    }
  
    private void PrepareVdeioResolutionSelection()
    {   
	    Spinner spinner = (Spinner) findViewById(R.id.video_resolution);
	    spinner.setOnItemSelectedListener(this);
	    List<String> list = new ArrayList<String>();
	    list.add(CodecModel.VID_RES_480P);
	    list.add(CodecModel.VID_RES_720P);
	    list.add(CodecModel.VID_RES_1080P);
	    list.add(CodecModel.VID_RES_4K);

	    ArrayAdapter<String> dataAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, list);
	    dataAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
	    spinner.setAdapter(dataAdapter);
	    
	    int selectPostion = dataAdapter.getPosition(CodecModel.mVideoResolution);
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
	    
	    int bitrateSpinnerPostion = bitrateAdapter.getPosition(Integer.toString( CodecModel.mVideoBitrate / BITRATE_MBPS));
	    bitrate_spinner.setSelection(bitrateSpinnerPostion, false);        
	}

    private void PrepareAudioSourceSelection()
    {
	    int i;
	    Spinner spinner = (Spinner) findViewById(R.id.audio_source);
	    spinner.setOnItemSelectedListener(this);
	    List<String> sources = new ArrayList<String>();
	
	    sources.add(CodecModel.AUDSRC_MIC);
	    sources.add(CodecModel.AUDSRC_SYSTEM_AUDIO);

	
	    ArrayAdapter<String> Adapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, sources);
	    Adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
	    spinner.setAdapter(Adapter);
	    
	    int SpinnerPostion = Adapter.getPosition(CodecModel.mAudioSource);
	    spinner.setSelection(SpinnerPostion, false);        
	}

    private void PrepareMuxTypeSelection()
    {
	    int i;
	    Spinner spinner = (Spinner) findViewById(R.id.mux_type);
	    spinner.setOnItemSelectedListener(this);
	    List<String> sources = new ArrayList<String>();
	
	    sources.add(CodecModel.MUX_TYPE_MP4);
	    sources.add(CodecModel.MUX_TYPE_TS);

	
	    ArrayAdapter<String> Adapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, sources);
	    Adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
	    spinner.setAdapter(Adapter);
	    
	    int SpinnerPostion = Adapter.getPosition(CodecModel.mMuxType);
	    spinner.setSelection(SpinnerPostion, false);        
	}

    private void PrepareVidCodecTypeSelection()
    {
	    int i;
	    Spinner spinner = (Spinner) findViewById(R.id.video_codec_type);
	    spinner.setOnItemSelectedListener(this);
	    List<String> sources = new ArrayList<String>();
	
	    sources.add(CodecModel.VID_CODEC_TYPE_H264);
	    sources.add(CodecModel.VID_CODEC_TYPE_HEVC);

	
	    ArrayAdapter<String> Adapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, sources);
	    Adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
	    spinner.setAdapter(Adapter);
	    
	    int SpinnerPostion = Adapter.getPosition(CodecModel.mVidCodecType);
	    spinner.setSelection(SpinnerPostion, false);        
	}

    private void PrepareAudCodecTypeSelection()
    {
	    int i;
	    Spinner spinner = (Spinner) findViewById(R.id.audio_codec_type);
	    spinner.setOnItemSelectedListener(this);
	    List<String> sources = new ArrayList<String>();
	
	    sources.add(CodecModel.AUD_CODEC_TYPE_AAC);

	
	    ArrayAdapter<String> Adapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, sources);
	    Adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
	    spinner.setAdapter(Adapter);
	    
	    int SpinnerPostion = Adapter.getPosition(CodecModel.mAudCodecType);
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
		 		CodecModel.mSegmentDuration = Integer.parseInt(latencny);
		 		CodecModel.savePreferences(getApplicationContext(), CodecModel.KEY_SEGMENT_DURATION, CodecModel.mSegmentDuration);
		 	}
		 	break;
		 	case R.id.audio_source:
		 	{
		 		int position = arg0.getSelectedItemPosition();
		 		String audio_source = arg0.getAdapter().getItem(position).toString();
		 		CodecModel.mAudioSource = audio_source;
		 		CodecModel.savePreferences(getApplicationContext(), CodecModel.KEY_AUDIO_SOURCE, CodecModel.mAudioSource);
		 	}
		 	break;
		 	case R.id.mux_type:
		 	{
		 		int position = arg0.getSelectedItemPosition();
		 		String mux_type = arg0.getAdapter().getItem(position).toString();
		 		CodecModel.mMuxType = mux_type;
		 		CodecModel.savePreferences(getApplicationContext(), CodecModel.KEY_MUX_TYPE, CodecModel.mMuxType);
		 	}
		 	break;

		 	case R.id.audio_codec_type:
		 	{
		 		int position = arg0.getSelectedItemPosition();
		 		String selection = arg0.getAdapter().getItem(position).toString();
		 		CodecModel.mAudCodecType = selection;
		 		CodecModel.savePreferences(getApplicationContext(), CodecModel.KEY_AUD_CODEC_TYPE, CodecModel.mAudCodecType);
		 	}
		 	break;

		 	case R.id.video_codec_type:
		 	{
		 		int position = arg0.getSelectedItemPosition();
		 		String selection = arg0.getAdapter().getItem(position).toString();
		 		CodecModel.mVidCodecType = selection;
		 		CodecModel.savePreferences(getApplicationContext(), CodecModel.KEY_VID_CODEC_TYPE, CodecModel.mVidCodecType);
		 	}
		 	break;
		 	
		 	case R.id.video_bitrate:
		 	{
		 		int position = arg0.getSelectedItemPosition();
		 		String bitrate = arg0.getAdapter().getItem(position).toString();
		 		CodecModel.mVideoBitrate = Integer.parseInt(bitrate) * BITRATE_MBPS;
		 		CodecModel.savePreferences(getApplicationContext(), CodecModel.KEY_VIDEO_BITRATE, CodecModel.mVideoBitrate);
		 	}
		 	break;	
		 	case R.id.video_resolution:
		 	{
		 		int position = arg0.getSelectedItemPosition();
		 		String value = arg0.getAdapter().getItem(position).toString();
		 		CodecModel.mVideoResolution = value;
		 		CodecModel.savePreferences(getApplicationContext(), CodecModel.KEY_VIDEO_RESOLUTION, CodecModel.mVideoResolution);
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
