package com.mcntech.rtspplayer;


import com.mcntech.rtspplyer.R;
import com.mcntech.rtspplayer.StatsGraphView.SamplesBuffer;

import android.app.Activity;
import android.content.Context;
import android.graphics.PixelFormat;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.SurfaceView;
import android.widget.LinearLayout;
import android.widget.TextView;

public class StatsOverlay  {
    private Thread              mOverlayThread = null;
    private boolean             mRunning = false;
    private static final int    UPDATE_STEPS = 30;
    TextView                    mAudCodec = null;
    TextView                    mAudBitrate = null;
    TextView                    mAudJitter = null;    
    TextView                    mVidCodec = null;
    TextView                    mVidBitrate = null;
    TextView                    mVidJitter = null;    
    
	private SamplesBuffer       mSamplesBuffer[];
	private static final int    SAMPLES_IN_SCREEN = 1000;
	private static final int    NUM_GRAPH_ELEMENTS = 4;
    Activity                    mActivity = null;
    StatsGraphView              mGraphView;
    
    public StatsOverlay(Activity context) {
        
 
    	mAudCodec = (TextView) context.findViewById(R.id.aud_codec);
    	mAudBitrate = (TextView) context.findViewById(R.id.aud_bitrate); 
    	mAudJitter =(TextView) context.findViewById(R.id.aud_jitter); 
    	
    	mVidCodec = (TextView) context.findViewById(R.id.vid_codec);
    	mVidBitrate = (TextView) context.findViewById(R.id.vid_bitrate); 
    	mVidJitter =(TextView) context.findViewById(R.id.vid_jitter); 
    	
        mActivity = context;

		// Test Graph
        mSamplesBuffer=new SamplesBuffer[NUM_GRAPH_ELEMENTS];
		for(int i=0; i < NUM_GRAPH_ELEMENTS;i++) {
			mSamplesBuffer[i] = new SamplesBuffer(SAMPLES_IN_SCREEN, true);
		}
        LinearLayout StatsLayout = (LinearLayout)mActivity.findViewById(R.id.stats_layout);
        mGraphView = new StatsGraphView(mActivity, mSamplesBuffer);
        mGraphView.setZOrderMediaOverlay(true);
        mGraphView.getHolder().setFormat(PixelFormat.TRANSLUCENT);
        mGraphView.setEGLConfigChooser(false);
        StatsLayout.addView(mGraphView);
        mGraphView.EnableRenderer();

		
        new Thread(new Runnable() {
            public void run() {
            	startOverlay();//onDraw();
            }
        }).start();

    }
    
    private void startOverlay() {
	  //final Surface surface = mHolder.getSurface();
	  mOverlayThread = new Thread() {
	        @Override
	        public void run() {
	            while (mRunning) {
	                long startWhen = System.nanoTime();
/*	                for (int i = 0; i < UPDATE_STEPS; i++) {
	                    if (!mRunning) return;
	                    UpdateOverlay(0, i);
	                }
	                for (int i = UPDATE_STEPS; i > 0; i--) {
	                    if (!mRunning) return;
	                    UpdateOverlay(0, i);
	                }
*/	  
	            	try {
						sleep(100);
					} catch (InterruptedException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
	                long duration = System.nanoTime() - startWhen;
	                double framesPerSec = 1000000000.0 / (duration / (UPDATE_STEPS * 2.0));
	                Log.d("Overlay", "Update at " + framesPerSec + " fps");
	                updateStats(mActivity);
	            }
	        }
	    };
	    mRunning = true;
	    mOverlayThread.setName("StatsOverlay");
	    mOverlayThread.start();    
   }
	
    
    Handler mUpdateHandler = new Handler()
	{

		@Override
		public void handleMessage(Message msg) {
			mGraphView.updateData(currentTime);
			mGraphView.invalidate();
			mGraphView.requestRender();
			super.handleMessage(msg);
		}
		
	};
	
    long currentTime = 0;
	void updateData()
	{
		for(int j=0;j<5;j++) {
				double sinValue = Math.sin((double)(((currentTime/4)%360+j*45)*Math.PI/180.));
				short sampleValue = (short)(80.*sinValue);
				mSamplesBuffer[j].addSample(sampleValue, currentTime);
				mUpdateHandler.sendEmptyMessage(0);
		}
		currentTime+=1;
	}
    
    public void updateStats(Context context) {
/*    	
        if(mWifiMgr != null) {
        	WifiInfo wifiInfo = mWifiMgr.getConnectionInfo(); 
		   mApRssi = 0;
		   mLinkSpeed = 0;
		   try { 
			   mApRssi = wifiInfo.getRssi();
			   mLinkSpeed = wifiInfo.getLinkSpeed(); 
		   } catch (NullPointerException e) { 
	 
		   }
		   mActivity.runOnUiThread(new Runnable() {
               @Override
               public void run() {
            	   mTextViewApRssi.setText(Integer.toString(mApRssi) + "dBm");
            	   mTextViewLinkSpeed.setText(Integer.toString(mLinkSpeed) + "Mbps");
              	   mTextViewBufferLevel.setText(Integer.toString(SinglePlayerActivity.mFramesInBuff) + "Frames");
               }
           });
	   }    
*/    }
    /**
     * Signals the bounce-thread to stop, and waits for it to do so.
     */
    private void stopOverlay() {
        Log.d("Overlay", "Stopping StatsOverlay thread");
        if(mGraphView != null) {
        	//mGraphView.
        }
        mRunning = false;      // tell thread to stop
        if(mOverlayThread != null){
	        try {
	        	mOverlayThread.join();
	        } catch (InterruptedException ignored) {}
        }
        mOverlayThread = null;
    }
    
    public void onPause() {
	   stopOverlay();
    }
}