package com.mcntech.ezscreencast;

import android.app.Activity;
import android.content.pm.ApplicationInfo;
import android.content.Intent;
import android.media.projection.MediaProjection;
import android.media.projection.MediaProjectionManager;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ListView;
import android.widget.Toast;

import com.mcntech.ezscreencast.OnyxApi.RemoteNodeHandler;
import com.mcntech.ezscreencast.R;

import java.io.File;


public class RtspActivity extends Activity implements BaseSession, View.OnClickListener {
	private static final String TAG = "ezscreencast";
    private static final int MEDIAPROJECTION_REQUEST_CODE = 1;
    private static final int SETTING_DIALOG_CODE = 2;
    private static final int NODE_LIST_DIALOG_CODE = 3;
    private static final String DEF_SERVER1 = "educast server-1";
    private static final int BTN_ID_START = 1;
    
    private MediaProjectionManager mMediaProjectionManager;
    private ScreenRecorder mRecorder;
    private Button mBtnStart;
    private RtspModel mRtsp = null;
    private static long mHandle = 0;
    
    RemoteNodeHandler mDeviceHandler;
	
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d(TAG, "onCreate: ");
		boolean isSystemApp = (getApplicationInfo().flags
				  & (ApplicationInfo.FLAG_SYSTEM | ApplicationInfo.FLAG_UPDATED_SYSTEM_APP)) != 0;
		CodecModel.loadSavedPreferences(this, isSystemApp);
        
		setContentView(R.layout.activity_rtsp);
		mBtnStart = (Button) findViewById(R.id.button_startstop);
		mBtnStart.setText("Start");
		mBtnStart.setOnClickListener(this);
		mBtnStart.setTag(BTN_ID_START);
		mBtnStart.setEnabled(true);	
        mMediaProjectionManager = (MediaProjectionManager) getSystemService(MEDIA_PROJECTION_SERVICE);
		
        mDeviceHandler = new RemoteNodeHandler(){
    		@Override
    		public void onConnectRemoteNode(final String url) {
    	    }

    		@Override
    		public void onDisconnectRemoteNode(String deviceid) {
    			final String url = deviceid;
    		}

    		@Override
    		public void onStatusRemoteNode(final String url, String Msg) {
    			// TODO Auto-generated method stub
    			Log.d(TAG, url + ":" + Msg);
    		}

    		@Override
    		public void onRemoteNodeError(final String url,final String message) {
    			// TODO Auto-generated method stub			
    		}

			@Override
			public void onNetworkDisconnected() {
				// TODO Auto-generated method stub
				
			}
    	};        

    	mHandle = OnyxApi.initialize(OnyxApi.PROTOCOL_RTSP);
        OnyxApi.setRemoteNodeHandler(mDeviceHandler);
        Intent captureIntent = mMediaProjectionManager.createScreenCaptureIntent();
        startActivityForResult(captureIntent, MEDIAPROJECTION_REQUEST_CODE);
    }
    
    
    
	public boolean startSession(boolean enableAud, boolean enabeVid) {

		boolean result = true;
		RtspModel rtsp = mRtsp;
		String jPublishId = "v01";//rtsp.mPublishId;
		String jswitchId = rtsp.mSwcitchId;
		String jinputId = rtsp.mInputId;
		String jInputType = rtsp.mInputType;
		String jUrl = rtsp.mInputUrl;	
		String jInterface = "auto";
		String jStreamName = "v01";
		int nLocalServerPort = 8554;
		boolean fEnableMux = false;
		String mError = "";
		
		OnyxApi.CreateInputStream(mHandle, jinputId, jInputType, jUrl);
		OnyxApi.CreateSwitch(mHandle, jswitchId);
		OnyxApi.ConnectSwitchInput(mHandle, jswitchId, jinputId);
		OnyxApi.CreateRtspPublishBridge(mHandle, jPublishId);
		OnyxApi.AddRtspPublishBridgeToMediaSwitch(mHandle, jPublishId, jswitchId);
		
		boolean fIsLive = CodecModel.mIsLiveStream;
		int nBitrate = CodecModel.mVideoBitrate;
		int nWidth = CodecModel.getVideoWidth();
		int nHeight = CodecModel.getVideoHeight();
		int nFramerate = CodecModel.getVideoFramerate();
		int segmentDurationMs = CodecModel.mSegmentDuration * 1000;
		String strMimeType = CodecModel.mMuxType;
		String strCodecType = CodecModel.mVidCodecType;
		
		OnyxApi.EnableRtspLocalServer(mHandle, jswitchId, jInterface, jStreamName, nLocalServerPort, fEnableMux);		
		result = OnyxApi.StartSwitch(mHandle, jswitchId);
		result= OnyxApi.StartRtspPublishBridge(mHandle, jPublishId);
		if(!result) {
			mError = "Failed to StartPublishStream";
			return result;
		}
		OnyxApi.onvifSrvrStart(mHandle);
		mError = "Succcess";
		return result;
	}
    
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
      MenuInflater inflater = getMenuInflater();
      inflater.inflate(R.menu.menu_main, menu);
      return super.onCreateOptionsMenu(menu);
      //return true;
    } 
 
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle presses on the action bar items
        switch (item.getItemId()) {
            case R.id.action_settings:
                doSettings();
                return true;

            default:
                return super.onOptionsItemSelected(item);
        }
    }
  
    public void doSettings() {
        Intent intent = new Intent(this, CodecDlg.class);
        startActivity(intent);
    } 
    
    public void doEditRemoteNode(OnyxRemoteNode node) {
        Intent intent = new Intent(this, MpdDlg.class);
        intent.setType("text/plain");
        intent.putExtra("nickname",node.mNickname); 
        startActivityForResult(intent, NODE_LIST_DIALOG_CODE);
    } 
    
    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
    	if(requestCode == SETTING_DIALOG_CODE){
    		return;
    	} else if(requestCode == MEDIAPROJECTION_REQUEST_CODE){
	        MediaProjection mediaProjection = mMediaProjectionManager.getMediaProjection(resultCode, data);
	        if (mediaProjection == null) {
	            Log.e("@@", "media projection is null");
	            return;
	        }
	        // video size
	        final int width = CodecModel.getVideoWidth();
	        final int height = CodecModel.getVideoHeight();
	        final int framerate = CodecModel.getVideoFramerate();
	        mRtsp = new RtspModel();
	        int sessionId = 0;
	        OnyxRemoteNode node = new PublishSrversModel(this).read(DEF_SERVER1);
	        mRtsp.setNodeParams(node, sessionId);
	        
	        File file = new File(Environment.getExternalStorageDirectory(),
	                "record-" + width + "x" + height + "-" + System.currentTimeMillis() + ".mp4");
	        final int bitrate = CodecModel.mVideoBitrate;

	        mRecorder = new ScreenRecorder(this, this, width, height, framerate, bitrate, 1, mediaProjection, file.getAbsolutePath());

	        mBtnStart.setText("Stop");
	        mRecorder.start();
	
	        Toast.makeText(this, "EzSceencast is running...", Toast.LENGTH_SHORT).show();
	        moveTaskToBack(true);
    	}
    }

    @Override
    public void onClick(View v) {
    	//switch(v.getId()) {
    	switch((Integer)v.getTag()) {
    		//case R.id.button:
    		case BTN_ID_START:
		        if (mRecorder != null) {
		            mRecorder.quit();
		            mRecorder = null;
		        } else {
		            Intent captureIntent = mMediaProjectionManager.createScreenCaptureIntent();
		            startActivityForResult(captureIntent, MEDIAPROJECTION_REQUEST_CODE);
		            //OnyxApi.initialize(true);
		        }
		        break;
    	}
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if(mRecorder != null){
            mRecorder.quit();
            mRecorder = null;
        }
    }   	
}
