package com.mcntech.rtspplayer;

import java.util.List;


import android.app.Activity;
import android.content.ClipData;
import android.content.ClipDescription;

import android.graphics.Rect;

import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.DragEvent;
import android.view.TextureView;
import android.view.View;

import android.widget.ImageView;

import com.mcntech.rtspplyer.R;
import com.mcntech.rtspplayer.Configure;
import com.mcntech.rtspplayer.OnyxPlayerApi;
import com.mcntech.rtspplayer.OnyxPlayerApi.RemoteNodeHandler;
import com.mcntech.sphereview.VrDecodeToTexture;
import com.mcntech.sphereview.VrRenderDb;
import com.mcntech.sphereview.VrRenderDb.VideoFeed;

public class MultiPlayerActivity  extends Activity implements View.OnDragListener, View.OnLongClickListener  {
	
	public final String LOG_TAG = "rtsp";
	RemoteNodeHandler             mNodeHandler;
	Handler                       mHandler;
	ImageView                     img;
	int                           mLayoutId;
	

    //LinearLayout                     mStatsLayout;
    
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		mLayoutId = VrRenderDb.mVideoFeeds.size();
		int maxDecWidth = 0;
		int maxDecHeight = 0;
		//mHandler = new LocalHandler();
		Configure.loadSavedPreferences(this, false);

		switch(mLayoutId) {
			case 1:
			{
				setContentView(R.layout.activity_multi_player_1_1);
					maxDecWidth = 3840;
					maxDecHeight = 2160;
			}
			break;
			case 4:
			{
				setContentView(R.layout.activity_multi_player_2_2);
				maxDecWidth = 1280;
				maxDecHeight = 720;
			}
				break;
	
			case 9:
			{
				setContentView(R.layout.activity_multi_player_3_3);
				maxDecWidth = 1280;
				maxDecHeight = 720;
			}
				break;
	
			case 16:
				setContentView(R.layout.activity_multi_player_4_4);
				maxDecWidth = 640;
				maxDecHeight = 360;
				break;

		}
		//mStatsLayout = (LinearLayout)findViewById(R.id.stats_layout);
		for(int i=0; i < VrRenderDb.mVideoFeeds.size(); i++){
			VideoFeed videoFeed = VrRenderDb.mVideoFeeds.get(i);
			TextureView textureView = getTexture(mLayoutId, i);
			videoFeed.decodePipe = new DecodePipe(this, videoFeed.mUrl, textureView, maxDecWidth, maxDecHeight);
			videoFeed.textureId = i;
			textureView.setOnLongClickListener(this);
			textureView.setOnDragListener(this);
		}

		
		new Thread(new Runnable() {
			@Override
			public void run() {
				//OnyxPlayerApi.initialize();
				//OnyxPlayerApi.onvifDiscvrStart(0);
			}
		}).start();
		
		mNodeHandler = new RemoteNodeHandler(){

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

			@Override
			public void onDiscoverRemoteNode(String url) {
				// TODO Auto-generated method stub
				Log.d(LOG_TAG, "transition:onStartPlay");
				
			}
	 	}; 
	 	
	 	OnyxPlayerApi.setDeviceHandler(mNodeHandler);
	}
	
	private void StopStreaming()	
	{
		for(int i=0; i < VrRenderDb.mVideoFeeds.size(); i++){
			VideoFeed videoFeed = VrRenderDb.mVideoFeeds.get(i);
			if(videoFeed.decodePipe != null){
				Handler handler = videoFeed.decodePipe.getHandler();
				if(handler != null)
					handler.sendMessage(handler.obtainMessage(VrDecodeToTexture.PLAYER_CMD_STOP, 0));
			}
		}
	}
	
	public String findUrlForTexture(TextureView textureView)
	{
		for(VideoFeed videoFeed: VrRenderDb.mVideoFeeds){
			if(getTexture(mLayoutId, videoFeed.textureId) == textureView)
				return videoFeed.mUrl;
		}
		return null;
	}
	TextureView getTexture(int layoutId, int windowId) {
		switch(layoutId) {
			case 1:
			{
				switch(windowId){
					case 0: return (TextureView)findViewById(R.id.multi_player_surface_1_1_1);
				}
			}
			case 4:
			{
				switch(windowId){
				case 0: return (TextureView)findViewById(R.id.multi_player_surface_2_2_1);
				case 1: return (TextureView)findViewById(R.id.multi_player_surface_2_2_2);
				case 2: return  (TextureView)findViewById(R.id.multi_player_surface_2_2_3);
				case 3: return  (TextureView)findViewById(R.id.multi_player_surface_2_2_4);
				}
			}
				
			case 9:
			{
				switch(windowId){
				case 0: return (TextureView)findViewById(R.id.multi_player_surface_3_3_1);
				case 1: return (TextureView)findViewById(R.id.multi_player_surface_3_3_2);
				case 2: return  (TextureView)findViewById(R.id.multi_player_surface_3_3_3);
				case 3: return  (TextureView)findViewById(R.id.multi_player_surface_3_3_4);
				case 4: return  (TextureView)findViewById(R.id.multi_player_surface_3_3_5);
				case 5: return  (TextureView)findViewById(R.id.multi_player_surface_3_3_6);
				case 6: return  (TextureView)findViewById(R.id.multi_player_surface_3_3_7);
				case 7: return  (TextureView)findViewById(R.id.multi_player_surface_3_3_8);
				case 8: return  (TextureView)findViewById(R.id.multi_player_surface_3_3_9);	
				}
			}
			break;
				
			case 16:
			{
				switch(windowId){
				case 0: return (TextureView)findViewById(R.id.multi_player_surface_4_4_1);
				case 1: return (TextureView)findViewById(R.id.multi_player_surface_4_4_2);
				case 2: return  (TextureView)findViewById(R.id.multi_player_surface_4_4_3);
				case 3: return  (TextureView)findViewById(R.id.multi_player_surface_4_4_4);
				case 4: return  (TextureView)findViewById(R.id.multi_player_surface_4_4_5);
				case 5: return  (TextureView)findViewById(R.id.multi_player_surface_4_4_6);
				case 6: return  (TextureView)findViewById(R.id.multi_player_surface_4_4_7);
				case 7: return  (TextureView)findViewById(R.id.multi_player_surface_4_4_8);
				case 8: return  (TextureView)findViewById(R.id.multi_player_surface_4_4_9);	
				case 9: return (TextureView)findViewById(R.id.multi_player_surface_4_4_10);
				case 10: return (TextureView)findViewById(R.id.multi_player_surface_4_4_11);
				case 11: return  (TextureView)findViewById(R.id.multi_player_surface_4_4_12);
				case 12: return  (TextureView)findViewById(R.id.multi_player_surface_4_4_13);
				case 13: return  (TextureView)findViewById(R.id.multi_player_surface_4_4_14);
				case 14: return  (TextureView)findViewById(R.id.multi_player_surface_4_4_15);
				case 15: return  (TextureView)findViewById(R.id.multi_player_surface_4_4_16);
				}
			}
			break;
		}
		return null;
	}
	
    void makeStreamVisible(String url) {
    	// Get Current Visibe
    	
    	// Start Playing url
    	
    	// install callback for notifying first frame
    	
    		// on callback make Current invisible and make url visible, stop current
    }
    
	protected void onDestroy() {
		super.onDestroy();
	}
	
   @Override
    protected void onPause() {
	   StopStreaming();
	   super.onPause();

   }
  
   @Override
   public void onBackPressed() {
	   System.exit(2);
  }

   @Override
public boolean onDrag(View v, DragEvent event) {
	int action = event.getAction();

	if(action == DragEvent.ACTION_DRAG_STARTED) {
		String url = "";
		String position = "";
		float x =event.getX();
		float y =event.getY();
		TextureView textureView = (TextureView)v;
		url = findUrlForTexture(textureView);
		//Toast toast = Toast.makeText(this, "Drag : Action= Drag Start: " + " Item= " + url + " X=" + x + " Y=" + y, Toast.LENGTH_SHORT );
		//toast.show();
		//Log.d(LOG_TAG, "Drag : Action= Drag Start: " + " Item= " + url + " X=" + x + " Y=" + y );
		
	} else if(action == DragEvent.ACTION_DROP) {
		String urlDest = "";
		String urlSrc = "";

		TextureView textureView = (TextureView)v;
		urlDest = findUrlForTexture(textureView);
		//Toast toast = Toast.makeText(this, "Drag : Action= Drag Stop " + " Item= " + url + " X=" + x + " Y=" + y, Toast.LENGTH_SHORT );
		//toast.show();
		ClipData clipData = event.getClipData();
		ClipData.Item item = clipData.getItemAt(0);
		if(item != null)
			urlSrc = item.getText().toString();
		Log.d(LOG_TAG, "Drag : Action= Drag Stop " + " src= " + urlSrc  + " Dest=" + urlDest);
		if(VrRenderDb.moveUrl(urlSrc, urlDest)) {
			recreate();
		}
	}
	return true;
}
   @Override
   public boolean onLongClick(View v) {
      //ClipData.Item item = new ClipData.Item((CharSequence)v.getTag());
	   String url = "";
	  
      String[] mimeTypes = {ClipDescription.MIMETYPE_TEXT_PLAIN};
      TextureView textureView = (TextureView)v;
		url = findUrlForTexture(textureView);
      //ClipData dragData = new ClipData(v.getTag().toString(),mimeTypes, item);
		ClipData.Item item = new ClipData.Item(url);
      ClipData clipData = new ClipData("move",mimeTypes, item);
      View.DragShadowBuilder myShadow = new View.DragShadowBuilder(img);
      
      v.startDrag(clipData,myShadow,null,0);
      return true;
   }
      
}