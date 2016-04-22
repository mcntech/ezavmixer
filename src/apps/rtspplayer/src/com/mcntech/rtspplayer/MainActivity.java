package com.mcntech.rtspplayer;

import java.util.ArrayList;

import com.mcntech.rtspplayer.OnyxPlayerApi.RemoteNodeHandler;
import com.mcntech.rtspplyer.R;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.TextView.OnEditorActionListener;
import android.view.View.OnClickListener;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.BaseAdapter;

public class MainActivity extends Activity implements OnItemSelectedListener {
	
	CheckBox mEnLogoCheckBox;
	CheckBox mEnStatsCheckBox;
	//EditText mRtspUrl1;
	
	RemoteNodeHandler mNodeHandler;
	public static ListView mRemoteNodeListView = null;
	public static ArrayList<RemoteNode> mRemoteNodeList = null;
	public static ArrayAdapter<RemoteNode> mListAdapter = null;
	
	/*EditText mAudDealy;*/
    @Override
    protected void onCreate(Bundle savedInstanceState) {
    	int i;
    	super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        
        mRemoteNodeList = new ArrayList<RemoteNode>(); //CodecModel.mOnyxRemoteNodeList;
		mRemoteNodeListView =  (ListView)findViewById(R.id.listRemoteNodes);
		
		mListAdapter = new ArrayAdapter<RemoteNode>(getApplicationContext(), 
				android.R.layout.simple_list_item_checked, mRemoteNodeList); 
		mRemoteNodeListView.setAdapter(mListAdapter ); 
		mRemoteNodeListView.setChoiceMode(ListView.CHOICE_MODE_MULTIPLE);
		mRemoteNodeListView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
		    @Override
		    public void onItemClick(AdapterView<?> parent, View item,
		            int position, long id)
		    {
		    	//boolean checked = false;
			RemoteNode node = (RemoteNode)mListAdapter.getItem(position); 
			//doEditRemoteNode(node);
		    }
		});
	
			
		mNodeHandler = new RemoteNodeHandler(){
			@Override
			public void onDiscoverRemoteNode(String url) {
				// TODO Auto-generated method stub
				Log.d("rtspplayer", "MainActivity::onDiscoverRemoteNode");
				RemoteNode node = new RemoteNode(url);
				mRemoteNodeList.add(node);
				        //mListAdapter.notifyDataSetChanged();
				runOnUiThread(new Runnable() {
					@Override
                    public void run() {
						((BaseAdapter)((ListView)findViewById(R.id.listRemoteNodes)).getAdapter()).notifyDataSetChanged();
					}
				});
			}
			@Override
			public void onConnectRemoteNode(String url) {
/*
				Log.d(LOG_TAG, "transition:onStartPlay");
	 			mHandler.sendEmptyMessage(PLAYER_CMD_RUN);
	 			if(Configure.mEnableVideo)
	 				waitForVideoPlay();
*/	 			
			}

			@Override
			public void onDisconnectRemoteNode(String url) {
/*
				Log.d(LOG_TAG, "transition:onStopPlay:Begin");
	 			mHandler.sendEmptyMessage(PLAYER_CMD_STOP);
	 			if(Configure.mEnableVideo)
	 				waitForVideoStop();
	 			Log.d(LOG_TAG, "transition:onStopPlay:End");
*/				
			}

			@Override
			public void onStatusRemoteNode(String url, String message) {
				// TODO Auto-generated method stub
				
			}

			@Override
			public void onRemoteNodeError(String url, String message) {
				// TODO Auto-generated method stub
				
			}

			@Override
			public void onNetworkDisconnected() {
				// TODO Auto-generated method stub
				
			}


	 	}; 
	 	

			
		boolean isSystemApp = (getApplicationInfo().flags
				  & (ApplicationInfo.FLAG_SYSTEM | ApplicationInfo.FLAG_UPDATED_SYSTEM_APP)) != 0;
        Configure.loadSavedPreferences(this, isSystemApp);
                
 /*        Stats Enable 
        mEnStatsCheckBox = (CheckBox) findViewById(R.id.enable_stats);
        mEnStatsCheckBox.setOnClickListener(new OnClickListener() {
		        public void onClick(View v) {
		        	 Configure.mEnableStats = mEnStatsCheckBox.isChecked();
		        	 Configure.savePreferences(getApplicationContext(), Configure.KEY_ENABLE_STATS, Configure.mEnableStats);
		        }
        	}); 
        mEnStatsCheckBox.setChecked(Configure.mEnableStats);*/
        
/*        
        mAudDealy = (EditText) findViewById(R.id.editAudDelay);
        mAudDealy.setText(String.valueOf(Configure.mAudioDelay));
        mAudDealy.setOnEditorActionListener( new DoneOnEditorActionListener());
*/        
 /*       mRtspUrl1 = (EditText) findViewById(R.id.editRtspUrl1);
        mRtspUrl1.setText(String.valueOf(Configure.mRtspUrl1));
        mRtspUrl1.setOnEditorActionListener( new DoneOnEditorUrlListener());
        */
	 	OnyxPlayerApi.setDeviceHandler(mNodeHandler);	
		new Thread(new Runnable() {
			@Override
			public void run() {
				OnyxPlayerApi.initialize();
				OnyxPlayerApi.onvifDiscvrStart(0);
			}
		}).start();
  
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
/*     class DoneOnEditorUrlListener implements OnEditorActionListener {
			@Override
			public boolean onEditorAction(TextView v, int actionId,
					KeyEvent event) {
               if (actionId == EditorInfo.IME_ACTION_DONE) {
                   InputMethodManager imm = (InputMethodManager)v.getContext().getSystemService(Context.INPUT_METHOD_SERVICE);
                   imm.hideSoftInputFromWindow(v.getWindowToken(), 0);
            	   if(v.getText().length() > 0) {
            		   Configure.mRtspUrl1 = v.getText().toString();
            		   Configure.savePreferences(getApplicationContext(), Configure.KEY_RTSP_URL_1, Configure.mRtspUrl1);
            	   }
                   return true;
               }
				return false;
			}
     }    
*/     
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
	
	  void doSettings(){
	       Intent intent = new Intent(this, Settings.class);
	       startActivity(intent);
	   }
	  
	   public void startMultiPlayer(int numWindows, int res){
	       Intent intent = new Intent(this, MultiPlayerActivity.class);
	       
	       Bundle b = new Bundle();
	       b.putInt("windows", numWindows);

	       int numUrls = mRemoteNodeList.size();
	       if(numUrls > numWindows)
	    	   numUrls = numWindows;
	       String[] urlList = new String[numUrls];
	       for(int i=0; i < numUrls; i++) {
		       RemoteNode node  = mRemoteNodeList.get(i);
		       if(node != null) {
		    	   urlList[i] = node.getRtspStream(res);
		       }
	       }
	       b.putStringArray("urls", urlList);
	       
	       intent.putExtras(b);
	       startActivity(intent);
	   }
	   public void start_1_1(View v){
		   startMultiPlayer(1, RemoteNode.VID_RES_720P);
	   }
	   public void start_2_2(View v){
		   startMultiPlayer(4, RemoteNode.VID_RES_720P);
	   }
	   public void start_3_3(View v){
		   startMultiPlayer(9, RemoteNode.VID_RES_720P);
	   }
	   public void start_4_4(View v){
		   startMultiPlayer(16, RemoteNode.VID_RES_480P);
	   }
	   public void start_6_360(View v){
		   startMultiPlayer(6, RemoteNode.VID_RES_480P);
	   }
	   @Override
	    public void onBackPressed() {
		   System.exit(2);
	   }
}
