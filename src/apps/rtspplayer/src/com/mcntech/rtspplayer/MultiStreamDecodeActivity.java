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

public class MultiStreamDecodeActivity extends Activity  {
	
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
		setContentView(R.layout.player);
			mOverlaySv = (SurfaceView) findViewById(R.id.overlay_surface);
			mOverlaySv.setZOrderMediaOverlay(true);
			mOverlaySv.getHolder().setFormat(PixelFormat.TRANSLUCENT);
			mOverlay =  new Overlay(this.getApplicationContext(), mOverlaySv);
			mOverlaySv.setOnClickListener(new View.OnClickListener() {					
				@Override
				public void onClick(View v) {
					doSettings();
				}
			});
		mStatsLayout = (LinearLayout)findViewById(R.id.stats_layout);
		
		
		if(Configure.mEnableStats) {
			mStatsOverlay =  new StatsOverlay(this);
		} else {
			mStatsLayout.setVisibility(4);
		}
				
		mVideoSurfaceHolder = mVideoSv.getHolder();
		mVideoSurfaceHolder.addCallback(this);
		
		if(mUseStaticLayout) {			
			// Do nothing
		} else {
			setContentView(mVideoSv);
		}
		
		mBuff = ByteBuffer.allocateDirect(maxBuffSize);

		boolean retry = true;//blocking

		//DeviceController.initialize(instance,retry, role, mAudFramesPerPeriod, mAudNumPeriods, mAudDelayUs);
		
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
	   super.onPause();
	   if(Configure.mEnableLogo && mOverlay != null)
		   mOverlay.onPause();
	   if(Configure.mEnableStats && mStatsOverlay != null)
		   mStatsOverlay.onPause();	   
   }
  
   void doSettings(){
       Intent intent = new Intent(this, Settings.class);
       startActivity(intent);
   }


@Override
public void surfaceCreated(SurfaceHolder holder) {
	// TODO Auto-generated method stub
	
}


@Override
public void surfaceChanged(SurfaceHolder holder, int format, int width,
		int height) {
	// TODO Auto-generated method stub
	
}


@Override
public void surfaceDestroyed(SurfaceHolder holder) {
	// TODO Auto-generated method stub
	
}
}