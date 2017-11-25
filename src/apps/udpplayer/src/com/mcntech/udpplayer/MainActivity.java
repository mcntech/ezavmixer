package com.mcntech.udpplayer;

import java.io.File;
import java.util.ArrayList;

import com.mcntech.udpplayer.UdpPlayerApi.RemoteNodeHandler;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.provider.MediaStore;
import android.provider.OpenableColumns;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.CheckBox;

import android.widget.ListView;
import android.widget.TextView;
import android.widget.TextView.OnEditorActionListener;
import android.widget.Toast;


import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.BaseAdapter;
import org.json.JSONObject;
import org.json.JSONArray;
import org.json.JSONException;

import static android.os.Environment.getExternalStorageDirectory;


public class MainActivity extends Activity implements OnItemSelectedListener {
	public static final String LOG_TAG = "udpplayer";
	
	final int PICKFILE_RESULT_CODE = 1;
	Uri mUri = null;
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

        getServer();	
		mNodeHandler = new RemoteNodeHandler(){
			@Override
			public void onPsiChange(String url, String message) {
				Log.d(LOG_TAG, "MainActivity::onPsiChange");
				JSONArray psi = null;
				try {
					psi = new JSONArray(message);
				} catch (JSONException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
					return;
				}
				if(psi != null) {
					for (int i = 0; i < psi.length(); i++) {
						try {
							int vidPid = 0;
							JSONObject pgm = psi.getJSONObject(i);

							if(pgm != null){
								JSONArray esList = pgm.getJSONArray("streams");
								for (int j = 0; j < esList.length(); j++) {
									JSONObject es = esList.getJSONObject(j);
									String codec = es.getString("codec");
									vidPid = es.getInt("pid");
									if(codec.compareToIgnoreCase("mpeg2") == 0 ||
											codec.compareToIgnoreCase("h264") == 0 || 
											codec.compareToIgnoreCase("h265") == 0){

										RemoteNode node = new RemoteNode(url, codec, vidPid);
										mRemoteNodeList.add(node);
									}
								}
							}
						} catch (JSONException e) {
							// TODO Auto-generated catch block
							e.printStackTrace();
						}
					}	
				}

				
				runOnUiThread(new Runnable() {
					@Override
                    public void run() {
						// TODO: update webview
						((BaseAdapter)((ListView)findViewById(R.id.listRemoteNodes)).getAdapter()).notifyDataSetChanged();
					}
				});
				
			}

			@Override
			public void onRemoteNodeError(String url, String message) {
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
	 	UdpPlayerApi.setDeviceHandler(mNodeHandler);	
		new Thread(new Runnable() {
			@Override
			public void run() {
				UdpPlayerApi.initialize();
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
	  
	   public void initVrRenderDb(int numWindows, int res, boolean stereo){
		   int eyeId;
		   VrRenderDb.init();
	       int numUrls = mRemoteNodeList.size();
	       if(numUrls > numWindows)
	    	   numUrls = numWindows;
	       else if (numUrls  == 0) {
	    	   Toast.makeText(this, "No Cameras", Toast.LENGTH_LONG).show();
	    	   return;
	       }
	       
	       for(int i=0; i < numUrls; i++) {
		       RemoteNode node  = mRemoteNodeList.get(i);
		       if(node != null) {
		    	   String url = node.getRtspStream();
		    	   VrRenderDb.addFeed(node, 0);
		       }
	       }
	       initEyeIds(stereo);
	   }

	   public void initEyeIds(boolean stereo){
		   int eyeId;
		   ArrayList<VrRenderDb.VideoFeed> feedRenderList = VrRenderDb.getVideoFeeds();
		   if(feedRenderList != null) {
		       if(stereo) {
		    	   eyeId = VrRenderDb.ID_EYE_LEFT;
		       } else {
		    	   eyeId = VrRenderDb.ID_EYE_LEFT | VrRenderDb.ID_EYE_RIGHT;
		       }
		       for(VrRenderDb.VideoFeed feed : feedRenderList) {
		    	   feed.mIdEye  = eyeId;
		    	   if(stereo) {
			    	   // Toggle ID
			    	   if(eyeId == VrRenderDb.ID_EYE_LEFT)
			    		   eyeId = VrRenderDb.ID_EYE_RIGHT;
			    	   else
			    		   eyeId = VrRenderDb.ID_EYE_LEFT;
		    	   }
		       }
		   }
	   }

	   public void startMultiPlayer(int numWindows, int res){
		   initVrRenderDb(numWindows, res, false);
		   Intent intent = new Intent(this, MultiPlayerActivity.class);
	       startActivity(intent);
	   }

	   public void startPlayerMonoForArrangement(int numWindows, int res){

		   initVrRenderDb(numWindows, res, false);
		   Intent intent = new Intent(this, MultiPlayerActivity.class);
	       startActivity(intent);
	   }

	   public void start_1_1(View v){
		   startMultiPlayer(1, 0);
	   }
	   public void start_2_2(View v){
		   startMultiPlayer(4, 0);
	   }
	   public void start_3_3(View v){
		   startMultiPlayer(9, 0);
	   }
	   public void start_4_4(View v){
		   startMultiPlayer(16, 0);
	   }


	   public void multi_mono_tile_arrange(View v){
		   startPlayerMonoForArrangement(6, 0);
	   }
	   
	   @Override
	    public void onBackPressed() {
		   System.exit(2);
	   }
	   
	   void getServer()
	   {
	       Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
		   intent.addCategory(Intent.CATEGORY_OPENABLE);
		   intent.setType("video/*");

		   startActivityForResult(Intent.createChooser(intent, "Select a TS file"), PICKFILE_RESULT_CODE);
	   }

		public String getPath(Uri uri) {
			Cursor cursor = null;
			try {
				cursor = getContentResolver().query(uri, null, null, null, null);
				if (cursor != null && cursor.moveToFirst()) {
					String displayName = cursor.getString(cursor.getColumnIndex(OpenableColumns.DISPLAY_NAME));
					//int sizeIndex = cursor.getColumnIndex(OpenableColumns.SIZE);

					//int column_index = cursor.getColumnIndexOrThrow(MediaStore.Video.Media.DATA);
					//return cursor.getString(column_index);
					File storage = getExternalStorageDirectory();
					if(storage != null) {
						//String path = storage.getAbsolutePath() + "/" + displayName;
						String path = "/mnt/sdcard/movies/" + displayName;
						return path;
					}
				} else
					return null;
			} finally {
				if (cursor != null) {
					cursor.close();
				}
			}
			return null;
		}

	   @Override
	    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
	        super.onActivityResult(requestCode, resultCode, data);
	        if(requestCode==PICKFILE_RESULT_CODE && resultCode==RESULT_OK) {
	            mUri = data.getData();
				//UdpPlayerApi.addServer(mUri.getPath());
				new Thread(new Runnable() {
					@Override
					public void run() {
						String filemanagerstring = mUri.getPath();
						String selectedFilePath = getPath(mUri);
						if(selectedFilePath != null) {
							UdpPlayerApi.addServer(selectedFilePath);
							UdpPlayerApi.startServer(selectedFilePath);
						}
					}
				}).start();
	        }
	    }
}
