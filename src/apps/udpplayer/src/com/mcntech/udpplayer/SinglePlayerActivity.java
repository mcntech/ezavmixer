package com.mcntech.udpplayer;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.ArrayList;

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
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.MotionEvent;
import android.view.View.MeasureSpec;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ListView;

import com.mcntech.udpplayer.Configure;
import com.mcntech.udpplayer.UdpPlayerApi;
import com.mcntech.udpplayer.Settings;
import com.mcntech.udpplayer.UdpPlayerApi.RemoteNodeHandler;

import com.android.grafika.gles.EglCore;
import com.android.grafika.gles.WindowSurface;

public class SinglePlayerActivity extends Activity implements SurfaceHolder.Callback {
	
	public final String LOG_TAG = "rtsp";
	String                        mUrl;
	int                           mStrmId;
	String                        mNewUrl = null;
	private PlayerThread          mVidPlayer = null;
	RemoteNodeHandler             mNodeHandler;
	Handler                       mHandler;
	SurfaceView                   mVideoSv = null;
	Surface                       mVideoSurface = null;
	SurfaceHolder                 mVideoSurfaceHolder;
	
	SurfaceView                   mOverlaySv;
	Overlay                       mOverlay = null;

	SurfaceView                   mStatsSv;
	StatsOverlay                  mStatsOverlay = null;
	
	ByteBuffer                    mBuff;
	long                          mPts;
	public static int             mFramesInBuff = 0;
	public static int             mFramesRendered = 0;
	// Audio Parameters
    static final int          SAMPLE_RATE = 44100;
    static final int          SAMPLE_INTERVAL = 20; // milliseconds
    static final int          SAMPLE_SIZE = 2; // bytes per sample
    static final int          BUF_SIZE = SAMPLE_INTERVAL*SAMPLE_INTERVAL*SAMPLE_SIZE*2;
    static final int          MIN_AUDIO_FRAMES_PER_PERIOD = 256;// Using 256 to make it align with wavepack frame setting for live mode

	//Video Parameters
	int                              maxBuffSize = (4 * 1024 * 1024);
	private MediaCodec               mDecoder = null;
	final long                       MAX_VIDEO_SYNC_THRESHOLD_US = 10000000;
	final long                       MAX_AUDIO_SYNC_THRESHOLD_US = 10000000;
	
    private final boolean            mUseStaticLayout = true;
    private boolean                  mfPlaying = false;
    
    final int                        PLAYER_CMD_RUN = 1;
    final int                        PLAYER_CMD_STOP = 2;
    final int                        PLAYER_CMD_INIT = 3;
    final int                        PLAYER_CMD_DEINIT = 4;    
    
    private final Object             mPlayLock = new Object();
    private final Object             mPUrlLock = new Object();
    
    private boolean                  mExitPlayerLoop = false;
    private int                      mCodecType = 1;
    int                              mMaxVidWidth =  3840;
    int                              mMaxVidHeight = 2160;
    int                              currentapiVersion = android.os.Build.VERSION.SDK_INT;

    private boolean                  mfSendCsd0DuringInit = false;
    private boolean                  mfAvcUHdSupported = false;
    private boolean                  mfHevcSupported = false;
    private boolean                  mAudioLowLatency = false;
    private int                      mAudioSampleRate = 44100;
    private int                      mAudFramesPerPeriod = 1024;
    private int                      mAudNumPeriods = 3;
    private int                      mVideoDelay = 0; // micro secs
    private int                      mAudDelayUs = 0;
    private int                      mDispWidth = 0;
    private int                      mDispHeight = 0;
    private boolean                  mEnableRuiTouch = true;
    int                              mRole = 9;
    LinearLayout                     mStatsLayout;
    LinearLayout                     mChannelsLayout;
    
	public static ListView mRemoteNodeListView = null;
	public static ArrayList<RemoteNode> mRemoteNodeList = null;
	public static ArrayAdapter<RemoteNode> mListAdapter = null;
 
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		mHandler = new LocalHandler();
		
		boolean isSystemApp = (getApplicationInfo().flags
				  & (ApplicationInfo.FLAG_SYSTEM | ApplicationInfo.FLAG_UPDATED_SYSTEM_APP)) != 0;	
		Configure.loadSavedPreferences(this, isSystemApp);
		mfAvcUHdSupported  = CodecInfo.isSupportedLevel("video/avc", MediaCodecInfo.CodecProfileLevel.AVCLevel51 );
		mfHevcSupported  = CodecInfo.isMimeTypeAvailable("video/hevc");
		if(mfAvcUHdSupported) {
			mMaxVidWidth = 3840;
			mMaxVidHeight = 2160;
		}
		
		PackageManager pm = getPackageManager();
		mAudioLowLatency = pm.hasSystemFeature(PackageManager.FEATURE_AUDIO_LOW_LATENCY);
		AudioManager am = (AudioManager) getSystemService(AUDIO_SERVICE);
		String audioSampleRate = am.getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE);
		String framesPerBuffer = am.getProperty(AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER);

		if(audioSampleRate != null) {
			mAudioSampleRate = Integer.parseInt(audioSampleRate);
			if(mAudioSampleRate == 0)
				mAudioSampleRate = 44100; // Force default
		}
		if(framesPerBuffer != null) {
			mAudFramesPerPeriod = Integer.parseInt(framesPerBuffer);
			if(mAudFramesPerPeriod < MIN_AUDIO_FRAMES_PER_PERIOD)
				mAudFramesPerPeriod = MIN_AUDIO_FRAMES_PER_PERIOD;
		}
		mAudDelayUs = Configure.mAudioDelay;
		if(Configure.mEnableAudio) {
			mVideoDelay = (int)((float)(mAudFramesPerPeriod * mAudNumPeriods) / mAudioSampleRate * 1000000) + mAudDelayUs;
		}

		
		if(mUseStaticLayout) {
			setContentView(R.layout.activity_single_player);
			mVideoSv = (SurfaceView) findViewById(R.id.player_surface);
			
			if(Configure.mEnableLogo) {
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
			} else {
				mVideoSv.setOnClickListener(new View.OnClickListener() {					
					@Override
					public void onClick(View v) {
						doSettings();
					}
				});
			}
			mStatsLayout = (LinearLayout)findViewById(R.id.stats_layout);
			mChannelsLayout = (LinearLayout)findViewById(R.id.channel_list_container);
		} else {
			mVideoSv = new SurfaceView(this);
			mVideoSv.setOnClickListener(new View.OnClickListener() {					
				@Override
				public void onClick(View v) {
					doSettings();
				}
			});			
		}
		
		if(Configure.mEnableOnScreenChannel) {	
		    mRemoteNodeList = new ArrayList<RemoteNode>(); //CodecModel.mOnyxRemoteNodeList;
		    UpdateRemoteNodeList();
		    if(mRemoteNodeList.size() > 0) {
		    	mUrl = mRemoteNodeList.get(0).getRtspStream();
		    }
			mRemoteNodeListView = (ListView)findViewById(R.id.channel_list);
			mListAdapter = new ArrayAdapter<RemoteNode>(getApplicationContext(), 
					android.R.layout.simple_list_item_checked, mRemoteNodeList); 
			mRemoteNodeListView.setAdapter(mListAdapter ); 
			mRemoteNodeListView.setChoiceMode(ListView.CHOICE_MODE_MULTIPLE);
			mRemoteNodeListView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
			    @Override
			    public void onItemClick(AdapterView<?> parent, View item,
			            int position, long id) {
			    	RemoteNode node = (RemoteNode)mListAdapter.getItem(position); 
					doNextUrl(node.getRtspStream());
			    }
			});
		} else {
			mChannelsLayout.setVisibility(4);
			mUrl = Configure.mRtspUrl1;
		}
		UpdateUrl(mUrl);
		
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
			public void onPsiPatChange(String url, String message) {
				// TODO Auto-generated method stub
				
			}
			@Override
			public void onPsiPmtChange(String url, String message) {
				// TODO Auto-generated method stub

			}

			public void onRemoteNodeError(final String url,final String message)
			{

			}
	 	}; 
	 	
	 	View.OnTouchListener  onTouchListner = new View.OnTouchListener() {
	 		public boolean onTouch(View v, MotionEvent m) {
	 	    	int pointerCount = m.getPointerCount();
	 	    	
	 	    	if(mDispWidth == 0 || mDispHeight == 0)
	 	    		return false;
	 	    	
	 	    	for (int i = 0; i < pointerCount; i++) 	{
	 	    		int x = (int) m.getX(i) * 1920 / mDispWidth;
	 	    		int y = (int) m.getY(i) * 1080 / mDispHeight;    		
	 	    		int id = m.getPointerId(i);
	 	    		int action = m.getActionMasked();
	 	    		int actionIndex = m.getActionIndex();
	 	    		String actionString;
	 	    		
	 	    		switch (action)
	 	    		{
	 	    			case MotionEvent.ACTION_DOWN:
	 	    				actionString = "DOWN";
	 	    				break;
	 	    			case MotionEvent.ACTION_UP:
	 	    				actionString = "UP";
	 	    				break;	
	 	    			case MotionEvent.ACTION_POINTER_DOWN:
	 	    				actionString = "PNTR DOWN";
	 	    				break;
	 	    			case MotionEvent.ACTION_POINTER_UP:
	 	        			actionString = "PNTR UP";
	 	        			break;
	 	    			case MotionEvent.ACTION_MOVE:
	 	    				actionString = "MOVE";
	 	    				break;
	 	    			default:
	 	    				actionString = "";
	 	    		}
	 	    		
	 	    		String touchStatus = "Action: " + actionString + " Index: " + actionIndex + " ID: " + id + " X: " + x + " Y: " + y;
	 	    		Log.d(LOG_TAG, touchStatus);
	 	    		RuiClient cleint = null;
					try {
						cleint = new RuiClient(actionString,id, x, y);
					} catch (JSONException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
					if(cleint != null)
						cleint.execute();
	 	    	}			
   			    return true;
   			}		
	 	};
	 	
	 	if(mEnableRuiTouch) {
		 	RuiClient.init(this);
		 	mVideoSv.setOnTouchListener(onTouchListner);
	 	}
	 	UdpPlayerApi.setDeviceHandler(mNodeHandler);
	}

	private class LocalHandler extends Handler {	
        @Override
         public void handleMessage(Message msg) {
             int what = msg.what;
			if(what == PLAYER_CMD_RUN) {
				if(!mfPlaying) {
					mExitPlayerLoop = false;
					if(Configure.mEnableVideo) {
	 			    	mVidPlayer = new PlayerThread();
	 			    	mVidPlayer.AttachSurface(mVideoSurface);
	 			    	mVidPlayer.start();
					}
	 				if(mOverlay != null) {
	 					mOverlay.SetLogoAnimation(false);
	 				}
	 				if(mOverlay != null) {
	 					mOverlay.SetLogoAnimation(false);
	 				}
	
	 				Log.d(LOG_TAG, "transition:PLAYER_CMD_RUN");
	 				
	 				
				} else {
					Log.d(LOG_TAG, "transition:PLAYER_CMD_RUN ignored...");
				}
			} else if(what == PLAYER_CMD_STOP) {
				mExitPlayerLoop = true;
 				if(mOverlay != null) {
 					mOverlay.SetLogoAnimation(true);
 				}
 				Log.d(LOG_TAG, "transition:PLAYER_CMD_STOP");
		    }  else if(what == PLAYER_CMD_INIT) {
				new Thread(new Runnable() {
					@Override
					public void run() {
						UdpPlayerApi.initialize();
						UdpPlayerApi.addServer(mUrl);
		 				mHandler.sendEmptyMessage(PLAYER_CMD_RUN);
					}
 				}).start();
 				Log.d(LOG_TAG, "transition:PLAYER_CMD_INIT");
 				
		    }   else if(what == PLAYER_CMD_DEINIT) {
 				Log.d(LOG_TAG, "transition:PLAYER_CMD_DEINIT");
 				new Thread(new Runnable() {
					@Override
					public void run() {
			 			if(Configure.mEnableVideo) {
			 				mExitPlayerLoop = true;
			 				waitForVideoStop();
			 			}
						UdpPlayerApi.deinitialize();
					}
 				}).start();
		    }
		}
	}

    /**
     * Wait for the player to stop.
     * <p>
     * Called from any thread other than the PlayTask thread.
     */
    public void waitForVideoStop() {
        synchronized (mPlayLock) {
 	       while (mfPlaying) {
                try {
                    mPlayLock.wait();
                } catch (InterruptedException ie) {
                	Log.d(LOG_TAG, "transition:waitForVideoStop exception ");
                }
            }
        }
    }

    public void waitForVideoPlay() {
        synchronized (mPlayLock) {
            while (!mfPlaying) {
                try {
                    mPlayLock.wait();
                } catch (InterruptedException ie) {
                	Log.d(LOG_TAG, "transition:waitForVideoPlay exception ");
                }
            }
        }
    }
    
	protected void onDestroy() {
		super.onDestroy();
	}

	@Override
	public void surfaceCreated(SurfaceHolder holder) {
		Log.d(LOG_TAG, "surfaceCreated");
	 	mDispWidth = mVideoSv.getWidth();
	 	mDispHeight = mVideoSv.getHeight();		
	}

	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
		Log.d(LOG_TAG, "surfaceChanged");
		mVideoSurface = holder.getSurface();
		mHandler.sendEmptyMessage(PLAYER_CMD_INIT);
	}

	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {
		Log.d(LOG_TAG, "surfaceDestroyed");
		mVideoSurface = null;
		mHandler.sendEmptyMessage(PLAYER_CMD_DEINIT);
	}

	/**
	* Clears the playback surface to black.
	*/
	
	private void clearSurface(SurfaceHolder holder) {
		Surface surface = holder.getSurface();
		EglCore eglCore = new EglCore();
		WindowSurface win = new WindowSurface(eglCore, surface, false);
		win.makeCurrent();
		GLES20.glClearColor(0, 0, 0, 0);
		GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
		win.swapBuffers();
		win.release();
		eglCore.release();
	}

	enum SERVER_STATE {
		UNINIT,
		SETUP,
		RUNNING,
		ERROR
	}

	void UpdateUrl(String NewUrl){
		//synchronized (mPlayLock) {
			mNewUrl = NewUrl;
		//}
	}
	
	String GetUrl(){
		//synchronized (mPlayLock) {
			return mNewUrl;
		//}
	}

	private class SwitchUrl extends Thread 
	{
		SERVER_STATE mCrntState = SERVER_STATE.UNINIT;
		String mSwitchUrl;
		public SwitchUrl(String newUrl)
		{
			mCrntState = SERVER_STATE.UNINIT;
			mSwitchUrl = newUrl;
		}
		@Override
		public void run() {
			UdpPlayerApi.addServer(mSwitchUrl);
			mCrntState = SERVER_STATE.SETUP;
			int nWaitTime = 3000;
			
			UdpPlayerApi.startServer(mSwitchUrl);
			int nFramesInBuff = UdpPlayerApi.getNumAvailVideoFrames(mSwitchUrl, mStrmId);
			while(nFramesInBuff == 0 && nWaitTime > 0) {
				nFramesInBuff = UdpPlayerApi.getNumAvailVideoFrames(mSwitchUrl, mStrmId);
				if(nFramesInBuff > 0){
					mCrntState = SERVER_STATE.RUNNING;
				} else {
					nWaitTime -= 100;
					try {
						Thread.sleep(100);
					} catch (InterruptedException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
				}
			}
			if (nFramesInBuff == 0 && nWaitTime <= 0) {
				mCrntState = SERVER_STATE.ERROR;
			} else {
				UpdateUrl(mSwitchUrl);
			}
		}
	}
	
	private class PlayerThread extends Thread {
		private Surface surface;

		public PlayerThread() {
			
		}

		public void AttachSurface (Surface surface) {
			this.surface = surface;
		}
		
		boolean InitDecoder( MediaCodec decoder, Surface surface)
		{
			MediaFormat format;
			if(mCodecType == 2 ) // HEVC
				format =  MediaFormat.createVideoFormat("video/hevc", mMaxVidWidth, mMaxVidHeight);// = extractor.getTrackFormat(i);
			else
				format =  MediaFormat.createVideoFormat("video/avc", mMaxVidWidth, mMaxVidHeight);// = extractor.getTrackFormat(i);				
			Log.d(LOG_TAG, "decoder configure");
			decoder.configure(format, surface, null, 0);
			return true;
		}
		
		boolean InitDecoder( MediaCodec decoder, Surface surface, ByteBuffer csd0)
		{
			MediaFormat format;
			if(mCodecType == 2 ) // HEVC
				format =  MediaFormat.createVideoFormat("video/hevc", mMaxVidWidth, mMaxVidHeight);// = extractor.getTrackFormat(i);
			else
				format =  MediaFormat.createVideoFormat("video/avc", mMaxVidWidth, mMaxVidHeight);// = extractor.getTrackFormat(i);				
			format.setByteBuffer("csd-0", csd0);
			Log.d(LOG_TAG, "decoder configure");
			decoder.configure(format, surface, null, 0);
			return true;
		}
		
		@Override
		public void run() {
			mCodecType = UdpPlayerApi.getVidCodecType(mUrl, mStrmId);
			try {
				if(mCodecType == 2 ) {
					Log.d(LOG_TAG, "decoder create video/hevc");
					mDecoder = MediaCodec.createDecoderByType("video/hevc");
					mfSendCsd0DuringInit = false;
				} else {
					Log.d(LOG_TAG, "decoder create video/avc");
					mDecoder = MediaCodec.createDecoderByType("video/avc");
				}
			} catch (IOException e1) {
				// TODO Auto-generated catch block
				e1.printStackTrace();
			}
			if(mDecoder == null) {
				Log.d(LOG_TAG, "createDecoderByType Failed");
				return;
			}

			UdpPlayerApi.startServer(mUrl);
			
			ByteBuffer[] inputBuffers = null;
			ByteBuffer[] outputBuffers = null;
			BufferInfo info = new BufferInfo();
			boolean isEOS = false;
			int sampleSize = 0;
			boolean fFrameAvail = false;
			
			synchronized (mPlayLock) {
				mfPlaying = true;
				mPlayLock.notifyAll();
			}			
			//long startMs = System.currentTimeMillis();
			if(mfSendCsd0DuringInit) {
				while (!Thread.interrupted() && !mExitPlayerLoop) {
					mFramesInBuff = UdpPlayerApi.getNumAvailVideoFrames(mUrl, mStrmId);
					if (!isEOS && mFramesInBuff > 0) {
						sampleSize = UdpPlayerApi.getVideoFrame(mUrl, mStrmId, mBuff, mBuff.capacity(),  100 * 1000);
						if (sampleSize > 0) {
							byte [] arCsd0 = null;
							mBuff.limit(sampleSize);
							mBuff.position(0);
							arCsd0 = extractCsd0(mBuff);
							if(arCsd0 != null && arCsd0.length > 0) {
							ByteBuffer Csd0 = ByteBuffer.wrap(arCsd0);
								Log.d(LOG_TAG, "Initializing decoder with Csd0");
								InitDecoder(mDecoder, surface, Csd0);
								fFrameAvail = true;
								break;
							} else {
								continue;
							}
						}
					} else {
						try {
							sleep(10);
						} catch (InterruptedException e) {
							e.printStackTrace();
							interrupt();
							break;
						}
					}
				}
			} else {
				InitDecoder(mDecoder, surface);
			}
			if(!mExitPlayerLoop) {
				try{
					Log.d(LOG_TAG, "mDecoder.start");
					mDecoder.start();
				} catch (IllegalStateException e) {
					Log.d(LOG_TAG, "IllegalStateException");
					e.printStackTrace();
				}
				if(currentapiVersion <= 20) {
					inputBuffers = mDecoder.getInputBuffers();
					outputBuffers = mDecoder.getOutputBuffers();
				}
			}
			
			while (!Thread.interrupted() && !mExitPlayerLoop) {

				// Check if URL changed
				String url = GetUrl();
				if(!url.equalsIgnoreCase(mUrl)) {
					new closeUrlTask().execute(mUrl);
					mUrl = url;
				}

				mFramesInBuff = UdpPlayerApi.getNumAvailVideoFrames(mUrl, mStrmId);
				if (!isEOS && mFramesInBuff > 0) {

					int inIndex;
					
					try {
						//Log.d(LOG_TAG, "dequeueInputBuffer:Begin");
						inIndex = mDecoder.dequeueInputBuffer(1000);
						//Log.d(LOG_TAG, "dequeueInputBuffer:Begin inIndex = " + inIndex);
					} catch (IllegalStateException e) {
						Log.d(LOG_TAG, "Decoder IllegalStateException");
						break;
					}
					//Log.d(LOG_TAG, "dequeueInputBuffer:End");
					if (inIndex >= 0) {
						ByteBuffer buffer = null;
						if(currentapiVersion <= 20) {
							buffer = inputBuffers[inIndex];
						} else if (currentapiVersion >= 21) {
							// TODO: Use reflection to use with API level <= 20
							buffer = mDecoder.getInputBuffer(inIndex);
						}
						
						if(fFrameAvail) {
							fFrameAvail = false;
						} else {
							sampleSize = UdpPlayerApi.getVideoFrame(mUrl,mStrmId, mBuff, mBuff.capacity(),  100 * 1000);
						}
						mPts = UdpPlayerApi.getVideoPts(mUrl, mStrmId);// + 500000; // video pipeline delay
						if (sampleSize <= 0) {
							// We shouldn't stop the playback at this point, just pass the EOS
							// flag to decoder, we will get it again from the
							// dequeueOutputBuffer
							Log.d(LOG_TAG, "InputBuffer BUFFER_FLAG_END_OF_STREAM");														
							mDecoder.queueInputBuffer(inIndex, 0, 0, 0, MediaCodec.BUFFER_FLAG_END_OF_STREAM);
							isEOS = true;
						} else {	
							mBuff.limit(sampleSize);
							mBuff.position(0);
							buffer.clear();
							buffer.put(mBuff.array(), 0, sampleSize);
							//buffer.put(mBuff, 0, sampleSize);
							buffer.position(0);
							buffer.limit(sampleSize);
														
							try {
								mDecoder.queueInputBuffer(inIndex, 0, sampleSize, mPts, 0);
							} catch (IllegalStateException e) {
								Log.d(LOG_TAG, "Decoder: queueInputBuffer: IllegalStateException");
								break;
							}
						}
					}
					
				} else {
					// TODO : Detect disconnect condition
				}

				//Log.d(LOG_TAG, "dequeueOutputBuffer:Begin isEOS=" + isEOS + " availFrame=" + DeviceController.getNumAvailVideoFrames() + " surfacevalid=" + surface.isValid());
				int outIndex;
				try {
					outIndex = mDecoder.dequeueOutputBuffer(info, 1000);
				}  catch (IllegalStateException e) {
					Log.d(LOG_TAG, "Decoder dequeueOutputBuffer: exception: " + e);	
					break;
				}
				//Log.d(LOG_TAG, "dequeueOutputBuffer:End outIndex=" + outIndex);
				long sysclk = UdpPlayerApi.getClockUs(mUrl, mStrmId);
				switch (outIndex) {
				case MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED:
					Log.d(LOG_TAG, "INFO_OUTPUT_BUFFERS_CHANGED");
					outputBuffers = mDecoder.getOutputBuffers();
					Log.d(LOG_TAG, "decoder getOutputBuffers");
					break;
				case MediaCodec.INFO_OUTPUT_FORMAT_CHANGED:
					Log.d(LOG_TAG, "New format " + mDecoder.getOutputFormat());
					break;
				case MediaCodec.INFO_TRY_AGAIN_LATER:
					//Log.d(LOG_TAG, "dequeueOutputBuffer timed out!");
					break;
				default:
					if(outIndex >= 0) {
						//Log.v(LOG_TAG, " presentationTime= " + (info.presentationTimeUs / 1000) + " fcvclk=" + sysclk / 1000 + " wait=" + (info.presentationTimeUs - sysclk) / 1000);				
						if(info.presentationTimeUs > sysclk + MAX_VIDEO_SYNC_THRESHOLD_US) {
							//Log.d(LOG_TAG, "FreeRun ");
						} else {
							while ((info.presentationTimeUs + 2 * mVideoDelay > sysclk) && !Thread.interrupted() && !mExitPlayerLoop) {
								try {
									sleep(10);
								} catch (InterruptedException e) {
									e.printStackTrace();
									interrupt();
									break;
								}
								sysclk = UdpPlayerApi.getClockUs(mUrl, mStrmId);
							}
						}
						//Log.d(LOG_TAG, "releaseOutputBuffer:Begin surfacevalid=" + surface.isValid());
						mDecoder.releaseOutputBuffer(outIndex, true);
						mFramesRendered++;
						//Log.d(LOG_TAG, "releaseOutputBuffer:End");
					} else {
						Log.d(LOG_TAG, "Invalid outIndex " + outIndex);
					}
					break;
				}

				// All decoded frames have been rendered, we can stop playing now
				if ((info.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0) {
					Log.d(LOG_TAG, "OutputBuffer BUFFER_FLAG_END_OF_STREAM");
					break;
				}
			}
			Log.d(LOG_TAG, "Exiting PlayerThread...");
			try {
				mDecoder.stop();
			} catch (IllegalStateException e) {
				Log.d(LOG_TAG, "Decoder stop: IllegalStateException");					
			}
			mDecoder.release();
			
			if(mUseStaticLayout && mVideoSurfaceHolder != null && mVideoSurface!= null) {
				clearSurface(mVideoSurfaceHolder);
			}
			
            synchronized (mPlayLock) {
            	mfPlaying = false;
                mPlayLock.notifyAll();
            }			
			Log.d(LOG_TAG, "Exited PlayerThread");
		}
	}
		
	int findStartCode(byte []arData, int pos, int limit, int nalu) {
		for(int i=pos; i < limit - 5; i++) {
			if(arData[i] == 0x00 && arData[1+1] == 0x00 && arData[i+2] == 0x00 && arData[i+3] == 0x01){
				if(nalu == 0x00 || arData[i+4] == nalu)
					return i; 
			}
		}
		return -1;
	}
	
	byte [] extractCsd0(ByteBuffer data){
		byte [] arData = data.array();
		byte [] arCsd0 = null;
		if (data.getInt() == 0x00000001) {
			System.out.println("parsing sps/pps");
		} else {
			System.out.println("something is amiss?");
		}
		
		int spsPos = findStartCode(arData, 0, data.limit(), 0x67);
		if(spsPos != -1) {
			int ppsPos = findStartCode(arData, spsPos + 4, data.limit(),0x68);
			if(ppsPos != -1) {
				int ppsEnd = findStartCode(arData, ppsPos+4, data.limit(), 0x00);
				if (ppsEnd == -1)
					ppsEnd = arData.length;
				arCsd0 = new byte[ppsEnd - spsPos];
				System.arraycopy(arData, spsPos, arCsd0, 0, arCsd0.length);
			}
		}
		return arCsd0;
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
   void UpdateRemoteNodeList(){
	   RemoteNode node1 = new RemoteNode("192.168.0.101:8554/v01");
	   mRemoteNodeList.add(node1);
	   RemoteNode node2 = new RemoteNode("192.168.0.101:8554/v02");
	   mRemoteNodeList.add(node2);	   
   }
   void doNextUrl(String url){
	   // Get Next URL
	   new SwitchUrl(url).start();
   }
   void closeCurrentUrl(String url){
	   // Get Next URL
		UdpPlayerApi.stopServer(url);
		try {
			Thread.sleep(300);
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		UdpPlayerApi.removeServer(url);
   }
   
   private class closeUrlTask extends AsyncTask<String, Integer, Long> {

	     protected void onProgressUpdate(Integer... progress) {
	         //setProgressPercent(progress[0]);
	     }

	     protected void onPostExecute(Long result) {
	         //showDialog("Downloaded " + result + " bytes");
	     }

		@Override
		protected Long doInBackground(String... params) {
			String url = params[0];
			UdpPlayerApi.stopServer(url);
			try {
				Thread.sleep(300);
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			UdpPlayerApi.removeServer(url);
			return null;
		}
	 }
}