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

    LinearLayout                     mStatsLayout;
    
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		//mHandler = new LocalHandler();
		Configure.loadSavedPreferences(this, false);
		setContentView(R.layout.activity_multi_player);
		mStatsLayout = (LinearLayout)findViewById(R.id.stats_layout);
		TextureView mTextureView1 = (TextureView)findViewById(R.id.multi_player_surface1);
		String url = Configure.mRtspUrl1;
		DecodePipe decPipe1 = new DecodePipe(this, url, mTextureView1);
		
		new Thread(new Runnable() {
			@Override
			public void run() {
				OnyxPlayerApi.onvifDiscvrStart(0);
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
  
   void doSettings(){
       Intent intent = new Intent(this, Settings.class);
       startActivity(intent);
   }

}