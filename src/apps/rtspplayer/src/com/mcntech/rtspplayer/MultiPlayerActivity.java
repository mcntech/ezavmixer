package com.mcntech.rtspplayer;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.List;

import org.json.JSONException;

import android.app.Activity;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.graphics.PixelFormat;
import android.media.AudioManager;
import android.media.MediaCodec;
import android.media.MediaCodec.BufferInfo;
import android.media.MediaCodecInfo;

import android.media.MediaFormat;
import android.opengl.GLES20;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.TextureView;
import android.view.View;
import android.view.MotionEvent;
import android.view.View.MeasureSpec;
import android.widget.LinearLayout;

import com.mcntech.rtspplyer.R;
import com.mcntech.rtspplayer.Configure;
import com.mcntech.rtspplayer.OnyxPlayerApi;
import com.mcntech.rtspplayer.OnyxPlayerApi.RemoteNodeHandler;
import com.mcntech.rtspplayer.Settings;

import com.android.grafika.gles.EglCore;
import com.android.grafika.gles.WindowSurface;

public class MultiPlayerActivity extends Activity  {
	
	public final String LOG_TAG = "rtsp";
	List<DecodeCtx>               mListDecCtx;                   
	RemoteNodeHandler             mNodeHandler;
	Handler                       mHandler;
	
	public class DecodeCtx {
		TextureView               mVideoSv = null;
		String                    mUrl;                		
	    DecodePipe                mDecodePipe;
	    boolean                   mVisible;
	}

    //LinearLayout                     mStatsLayout;
    
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		Bundle b = getIntent().getExtras();
		int gridId = b.getInt("windows");
		String[] urls = b.getStringArray("urls");
		
		//mHandler = new LocalHandler();
		Configure.loadSavedPreferences(this, false);
		switch(gridId) {
			case 1:
			{
				setContentView(R.layout.activity_multi_player_1_1);
				for(int i=0; i < urls.length; i++){
					TextureView textureView = getTexture(gridId, i);
					DecodePipe decPipe1 = new DecodePipe(this, urls[i], textureView, 3840, 2160);
				}
			}
				break;
			case 4:
			{
				setContentView(R.layout.activity_multi_player_2_2);
				for(int i=0; i < urls.length; i++){
					TextureView textureView = getTexture(gridId, i);
					DecodePipe decPipe1 = new DecodePipe(this, urls[i], textureView, 1280, 720);
				}
			}
				break;
	
			case 9:
			{
				setContentView(R.layout.activity_multi_player_3_3);

				for(int i=0; i < urls.length; i++){
					TextureView textureView = getTexture(gridId, i);
					DecodePipe decPipe1 = new DecodePipe(this, urls[i], textureView, 1280, 720);
				}
			}
				break;
	
			case 16:
				setContentView(R.layout.activity_multi_player_4_4);
				for(int i=0; i < urls.length; i++){
					TextureView textureView = getTexture(gridId, i);
					DecodePipe decPipe1 = new DecodePipe(this, urls[i], textureView, 640, 360);
				}				
				break;
		}
		//mStatsLayout = (LinearLayout)findViewById(R.id.stats_layout);

		
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

   }
  
   @Override
   public void onBackPressed() {
	   System.exit(2);
  }
   
}