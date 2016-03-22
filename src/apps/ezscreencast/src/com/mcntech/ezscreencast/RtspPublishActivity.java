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
import java.util.ArrayList;
import java.util.List;

public class RtspPublishActivity extends Activity implements BaseSession, View.OnClickListener {
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
	public static ListView mRemoteNodeListView = null;
	public static ArrayList<OnyxRemoteNode> mOnyxRemoteNodeList = null;
	ArrayAdapter<OnyxRemoteNode> mListAdapter;
	
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d(TAG, "onCreate: ");
		boolean isSystemApp = (getApplicationInfo().flags
				  & (ApplicationInfo.FLAG_SYSTEM | ApplicationInfo.FLAG_UPDATED_SYSTEM_APP)) != 0;
		CodecModel.loadSavedPreferences(this, isSystemApp);
        
		setContentView(R.layout.activity_rtsp_publish);
		mBtnStart = (Button) findViewById(R.id.button_startstop);
		mBtnStart.setText("Start");
		mBtnStart.setOnClickListener(this);
		mBtnStart.setTag(BTN_ID_START);
				
        mMediaProjectionManager = (MediaProjectionManager) getSystemService(MEDIA_PROJECTION_SERVICE);
		
        mOnyxRemoteNodeList = CodecModel.mOnyxRemoteNodeList;
        mRemoteNodeListView =  (ListView)findViewById(R.id.listRemoteNodes);

	    mListAdapter = new ArrayAdapter<OnyxRemoteNode>(getApplicationContext(), 
				android.R.layout.simple_list_item_checked, mOnyxRemoteNodeList); 
		mRemoteNodeListView.setAdapter(mListAdapter ); 
		mRemoteNodeListView.setChoiceMode(ListView.CHOICE_MODE_MULTIPLE);
		mRemoteNodeListView.setOnItemClickListener(new AdapterView.OnItemClickListener()
		{
		    @Override
		    public void onItemClick(AdapterView<?> parent, View item,
		            int position, long id)
		    {
		    	//boolean checked = false;
		    	OnyxRemoteNode node = (OnyxRemoteNode)mListAdapter.getItem(position); 
		    	doEditRemoteNode(node);
		    }
		});

        mDeviceHandler = new RemoteNodeHandler(){
    		@Override
    		public void onConnectRemoteNode(final String url) {
    			boolean newDevice = true;
    			if(mOnyxRemoteNodeList == null)
    				return;
    			// TODO Update status
    	    }

    		@Override
    		public void onDisconnectRemoteNode(String deviceid) {
    			final String url = deviceid;
    			if(mOnyxRemoteNodeList == null)
    				return;
    			runOnUiThread(new Runnable() {
                    public void run(){
                		for(int i=0;i<mOnyxRemoteNodeList.size();i++){
                			if(mOnyxRemoteNodeList.get(i).mNickname == url){
                				//mOnyxRemoteNodeList.remove(i);
                				// TODO : Upda
                			}
                		}
                    }
                });    			
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
    	//ConfigDatabase.loadSavedPreferences(this, isSystemApp);
    	int recordCount = new PublishSrversModel(this).count();
    	if(recordCount <= 0){
    		// Initialize Database
        	OnyxRemoteNode node1 = new OnyxRemoteNode(DEF_SERVER1);
        	new PublishSrversModel(this).createRow(node1);
    	}
        mHandle = OnyxApi.initialize(OnyxApi.PROTOCOL_RTSP);
        OnyxApi.setRemoteNodeHandler(mDeviceHandler);
        
        
        List<OnyxRemoteNode> RemoteNodes = new PublishSrversModel(this).read();
        
        if (RemoteNodes.size() > 0) {
            for (OnyxRemoteNode obj : RemoteNodes) {
            	mOnyxRemoteNodeList.add(obj);
            }
        }
    }
    
    
    
	public boolean startSession(boolean enableAud, boolean enabeVid) {

		boolean result = true;
		RtspModel rtsp = mRtsp;
		String jPublishId = rtsp.mPublishId;
		String jswitchId = rtsp.mSwcitchId;
		String jinputId = rtsp.mInputId;
		String jInputType = rtsp.mInputType;
		String jUrl = rtsp.mInputUrl;	
		
		String mError = "";
		
		OnyxApi.CreateInputStream(mHandle, jinputId, jInputType, jUrl);
		OnyxApi.CreateSwitch(mHandle, jswitchId);
		OnyxApi.ConnectSwitchInput(mHandle, jswitchId, jinputId);
		
		OnyxApi.CreateRtspServer(mHandle, jPublishId, jswitchId);
		
		boolean fIsLive = CodecModel.mIsLiveStream;
		int nBitrate = CodecModel.mVideoBitrate;
		int nWidth = CodecModel.getVideoWidth();
		int nHeight = CodecModel.getVideoHeight();
		int nFramerate = CodecModel.getVideoFramerate();
		int segmentDurationMs = CodecModel.mSegmentDuration * 1000;
		String strMimeType = CodecModel.mMuxType;
		String strCodecType = CodecModel.mVidCodecType;
		
		result = OnyxApi.StartSwitch(mHandle, jswitchId);
		result= OnyxApi.StartRtspServer(mHandle, jPublishId);
		if(!result) {
			mError = "Failed to StartPublishStream";
			return result;
		}
		
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
    	} else if(requestCode == NODE_LIST_DIALOG_CODE){
    		mListAdapter.notifyDataSetChanged();
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
	        MpdModel mMpdSession = new MpdModel();
	        int mpdSessionId = 0;
	        OnyxRemoteNode node = new PublishSrversModel(this).read(DEF_SERVER1);
	        mMpdSession.setNodeParams(node, mpdSessionId);
	        
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
