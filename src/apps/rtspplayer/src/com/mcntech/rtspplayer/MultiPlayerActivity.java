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
		int value = b.getInt("windows");
		String[] urls = b.getStringArray("urls");
		
		//mHandler = new LocalHandler();
		Configure.loadSavedPreferences(this, false);
		switch(value) {
			case 1:
			{
				setContentView(R.layout.activity_multi_player_1_1);
				TextureView mTextureView1 = (TextureView)findViewById(R.id.multi_player_surface1);
				DecodePipe decPipe1 = new DecodePipe(this, urls[0], mTextureView1, 3840, 2160);		
			}
				break;
			case 4:
			{
				setContentView(R.layout.activity_multi_player_2_2);
				TextureView textureView1 = (TextureView)findViewById(R.id.multi_player_surface_2_2_1);
				TextureView textureView2 = (TextureView)findViewById(R.id.multi_player_surface_2_2_2);
				TextureView textureView3 = (TextureView)findViewById(R.id.multi_player_surface_2_2_3);
				TextureView textureView4 = (TextureView)findViewById(R.id.multi_player_surface_2_2_4);
				DecodePipe decPipe1 = new DecodePipe(this, urls[0], textureView1, 1920, 1080);
				DecodePipe decPipe2 = new DecodePipe(this, urls[1], textureView2, 1920, 1080);
				DecodePipe decPipe3 = new DecodePipe(this, urls[2], textureView3, 1920, 1080);
				DecodePipe decPipe4 = new DecodePipe(this, urls[3], textureView4, 1920, 1080);
			}
				break;
	
			case 9:
				setContentView(R.layout.activity_multi_player_3_3);
				break;
	
			case 16:
				setContentView(R.layout.activity_multi_player_4_4);
				break;
	
			case 64:
				setContentView(R.layout.activity_multi_player_8_8);
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
  
 
   
}