package com.mcntech.ezscreencast;

import android.app.ActionBar.LayoutParams;
import android.app.Activity;
import android.content.DialogInterface.OnClickListener;
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
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.mcntech.ezscreencast.OnyxApi.RemoteNodeHandler;
import com.mcntech.ezscreencast.R;

import java.io.File;
import java.util.ArrayList;

public class MainActivity extends Activity implements View.OnClickListener {
	private static final String TAG = "ezscreencast";
    private static final int MEDIAPROJECTION_REQUEST_CODE = 1;
    private static final int SETTING_DIALOG_CODE = 2;
    private static final int NODE_LIST_DIALOG_CODE = 3;
    
    private static final int BTN_ID_START = 1;
    
    private MediaProjectionManager mMediaProjectionManager;
    private ScreenRecorder mRecorder;
    private Button mBtnStart;
    
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
		ConfigDatabase.loadSavedPreferences(this, isSystemApp);
	    LinearLayout mediaLayout = new LinearLayout(this);
	    mediaLayout.setOrientation(LinearLayout.VERTICAL);
		setContentView(mediaLayout);        
		
		LinearLayout.LayoutParams btnParams = new LinearLayout.LayoutParams(LayoutParams.FILL_PARENT, 
                LayoutParams.WRAP_CONTENT);
		btnParams.setMargins(40, 40, 40, 40);
		mBtnStart =  new Button(mediaLayout.getContext());
		mBtnStart.setTag(BTN_ID_START);
		mBtnStart.setOnClickListener(this);
		mBtnStart.setText("Start Broadcast");
		mBtnStart.setLayoutParams(btnParams);
		mediaLayout.addView(mBtnStart);		

		LinearLayout.LayoutParams lableParams = new LinearLayout.LayoutParams(LayoutParams.FILL_PARENT, 
                LayoutParams.WRAP_CONTENT);
		TextView label =  new TextView(mediaLayout.getContext());
		label.setText("Publishing Servers");
		lableParams.setMargins(40, 40, 40, 40);
		label.setLayoutParams(lableParams);
		mediaLayout.addView(label);

        mMediaProjectionManager = (MediaProjectionManager) getSystemService(MEDIA_PROJECTION_SERVICE);
		
        mOnyxRemoteNodeList = ConfigDatabase.mOnyxRemoteNodeList;
	    mRemoteNodeListView =  new ListView(mediaLayout.getContext());
	    mRemoteNodeListView.setLayoutParams(new
				LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT,
						ViewGroup.LayoutParams.WRAP_CONTENT)); 
	    mListAdapter = new ArrayAdapter<OnyxRemoteNode>(mediaLayout.getContext(), 
						android.R.layout.simple_list_item_checked, mOnyxRemoteNodeList); 
		mRemoteNodeListView.setAdapter(mListAdapter ); 
		mRemoteNodeListView.setChoiceMode(ListView.CHOICE_MODE_MULTIPLE);
		mRemoteNodeListView.setOnItemClickListener(new AdapterView.OnItemClickListener()
        {
            @Override
            public void onItemClick(AdapterView<?> parent, View item,
                    int position, long id)
            {
            	boolean checked = false;
            	OnyxRemoteNode node = (OnyxRemoteNode)mListAdapter.getItem(position); 
            	if(mRemoteNodeListView.isItemChecked(position)){
        			mRemoteNodeListView.setItemChecked(position, checked);
            	}else{
        			mRemoteNodeListView.setItemChecked(position, !checked);
            	}
            }
        });
		
		mediaLayout.addView(mRemoteNodeListView);	        
        
        mDeviceHandler = new RemoteNodeHandler(){
    		@Override
    		public void onConnectRemoteNode(final OnyxRemoteNode device, boolean updated) {
    			boolean newDevice = true;
    			if(mOnyxRemoteNodeList == null)
    				return;
    	    }

    		@Override
    		public void onRemoveRemoteNode(String deviceid) {
    			final String url = deviceid;
    			if(mOnyxRemoteNodeList == null)
    				return;
    			runOnUiThread(new Runnable() {
                    public void run(){
                		for(int i=0;i<mOnyxRemoteNodeList.size();i++){
                			if(mOnyxRemoteNodeList.get(i).mUrl == url)
                				mOnyxRemoteNodeList.remove(i);
                		}
                    }
                });    			
    		}

    		@Override
    		public void onNetworkDisconnected() {
    			// TODO Auto-generated method stub
    			
    		}

    		@Override
    		public void onRemoteNodeError(final String url,final String message) {
    			// TODO Auto-generated method stub			
    		}
    	};        
    	//ConfigDatabase.loadSavedPreferences(this, isSystemApp);
        OnyxApi.initialize(true, ConfigDatabase.mLatency);
        OnyxApi.setRemoteNodeHandler(mDeviceHandler);
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
            case R.id.action_nodelist:
                doServers();
                return true;
            default:
                return super.onOptionsItemSelected(item);
        }
    }
  
    public void doSettings() {
        Intent intent = new Intent(this, ConfigDialog.class);
        startActivity(intent);
    } 
    
    public void doServers() {
        Intent intent = new Intent(this, RemoteNodeDialog.class);
        intent.setType("text/plain");
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
	        final int width = ConfigDatabase.getVideoWidth();
	        final int height = ConfigDatabase.getVideoHeight();
	        final int framerate = ConfigDatabase.getVideoFramerate();
	        File file = new File(Environment.getExternalStorageDirectory(),
	                "record-" + width + "x" + height + "-" + System.currentTimeMillis() + ".mp4");
	        final int bitrate = ConfigDatabase.mVideoBitrate;
	        mRecorder = new ScreenRecorder(width, height, framerate, bitrate, 1, mediaProjection, file.getAbsolutePath());
	        //OnyxApi.initialize(true);
	        mRecorder.start();
	        mBtnStart.setText("Stop Firecast");
	        Toast.makeText(this, "Firecast is running...", Toast.LENGTH_SHORT).show();
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
	
	public void refreshDeviceView(int i){
		if(mRemoteNodeListView == null)
			return;
		OnyxRemoteNode device = (OnyxRemoteNode) mRemoteNodeListView.getItemAtPosition(i);
		final int deviceIndex = i;
		final boolean checked = OnyxApi.isRemoteNodeActive(device.mUrl);
		runOnUiThread(new Runnable() {
			public void run() {	
				mRemoteNodeListView.setItemChecked(deviceIndex, checked);
			}
		});
	} 
}
