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

	String mUrl = null;
	int mPlayOption = 0;

	RemoteNodeHandler mNodeHandler;
	public static ListView mRemoteNodeListView = null;
	public static ArrayList<RemoteNode> mRemoteNodeList = null;
	public static ArrayAdapter<RemoteNode> mListAdapter = null;
	
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
			public void onPsiPatChange(String url, String message) {
				Log.d(LOG_TAG, "MainActivity::onPsiPatChange");
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
							int pgmNum = 0;
							int pid = 0;
							JSONObject pgm = psi.getJSONObject(i);

							pgmNum = pgm.getInt("program");
							pid = pgm.getInt("pid");
							if(pid > 0 &&  pgmNum > 0) {
								RemoteNode node = new RemoteNode(url, pgmNum);
								mRemoteNodeList.add(node);
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
			public void onRemoteNodeError(final String url,final String message)
			{

			}
	 	};

        Configure.loadSavedPreferences(this, false);

		Bundle b = getIntent().getExtras();

		if(b != null) {
			mPlayOption = b.getInt("option");
			if(mPlayOption == 1) // Play URL
				mUrl = b.getString("url");
		}

	 	UdpPlayerApi.setDeviceHandler(mNodeHandler);	
		new Thread(new Runnable() {
			@Override
			public void run() {
				UdpPlayerApi.initialize();

				if(mPlayOption ==1 && mUrl != null) {
					UdpPlayerApi.addServer(mUrl);
					UdpPlayerApi.startServer(mUrl);
				}
			}
		}).start();

		if(mPlayOption == 0) {
			chooseFile();
		}
     }

	public void selfRestart(View v) {
		System.exit(2);
	}

	@Override
	public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2,
			long arg3) {
		//RemoteNode node = (RemoteNode)mListAdapter.getItem(position);
		
	}

	@Override
	public void onNothingSelected(AdapterView<?> arg0) {
		// TODO Auto-generated method stub
		
	}
	
	  void doSettings(){
	       Intent intent = new Intent(this, Settings.class);
	       startActivity(intent);
	   }

	   int getNumSelectedPgms()
	   {

		   int numUrls = mRemoteNodeList.size();
		   int numSelected = 0;
		   for(int i=0; i < numUrls; i++) {
			   RemoteNode node = mRemoteNodeList.get(i);
			   if (node != null && mRemoteNodeListView.isItemChecked(i)) {
				   numSelected++;
			   }
		   }
		   return numSelected;
		}

	public void initVrRenderDb(int numWindows, int res, boolean stereo){
		int numUrls = mRemoteNodeList.size();
		boolean fPgmsSelected = false;
		if (numUrls  == 0) {
			Toast.makeText(this, "No Programs", Toast.LENGTH_LONG).show();
			return;
		}

	   int numSelectedPgms = getNumSelectedPgms();

		if(numSelectedPgms > 0) {
			fPgmsSelected = true;
			if(numWindows > numSelectedPgms)
				numWindows = numSelectedPgms;
		} else {
			if(numWindows > numUrls)
				numWindows = numUrls;
		}
		VrRenderDb.init();

	   for(int i=0; i < numUrls && numWindows > 0; i++) {
		   RemoteNode node  = mRemoteNodeList.get(i);
		   if(node != null && (!fPgmsSelected || mRemoteNodeListView.isItemChecked(i))) {
			   String url = node.toString();
			   VrRenderDb.addFeed(node, 0);
			   numWindows--;
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
	   
	   void chooseFile()
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
					File storage = getExternalStorageDirectory();
					if(storage != null) {
						//String path = storage.getAbsolutePath() + "/" + displayName;
						//String path = "/mnt/sdcard/movies/" + displayName;
						String path = "/storage/emulated/0/Download/" + displayName;
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
	        } else {
	        	System.exit(0);
			}
	    }
}
