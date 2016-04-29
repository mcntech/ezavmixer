package com.mcntech.sphereview;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.util.Arrays;
import java.util.List;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;


import android.app.Activity;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.SurfaceTexture;
import android.media.MediaCodec;
import android.media.MediaCodec.BufferInfo;
import android.media.MediaCodecInfo;

import android.media.MediaFormat;
import android.opengl.GLES11Ext;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.opengl.GLUtils;
import android.opengl.Matrix;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.Surface;
import android.view.TextureView;
import android.widget.LinearLayout;


import com.mcntech.rtspplayer.CodecInfo;
import com.mcntech.rtspplayer.Configure;
import com.mcntech.rtspplayer.OnyxPlayerApi;
import com.mcntech.rtspplyer.R;



public class VrDecodeToTexture  {
	public static final String TAG = "VrDecodeToTexture";
	public final String LOG_TAG = "VrDecodeToTexture";
	String                        mUrl;                   
	private PlayerThread          mVidPlayer = null;
	Handler                       mHandler;
	Surface                       mVideoSurface = null;
	
	ByteBuffer                    mBuff;
	long                          mPts;
	public static int             mFramesInBuff = 0;
	public static int             mFramesRendered = 0;

	//Video Parameters
	int                              maxBuffSize = (4 * 1024 * 1024);
	private MediaCodec               mDecoder = null;
	final long                       MAX_VIDEO_SYNC_THRESHOLD_US = 10000000;
	final long                       MAX_AUDIO_SYNC_THRESHOLD_US = 10000000;
	

    private boolean                  mfPlaying = false;
    
    public static final int                        PLAYER_CMD_RUN = 1;
    public static final int                        PLAYER_CMD_STOP = 2;
    public static final int                 PLAYER_CMD_INIT = 3;
    public static final int                 PLAYER_CMD_DEINIT = 4;    
    
    private final Object             mPlayLock = new Object();
    private boolean                  mExitPlayerLoop = false;
    private int                      mCodecType = 1;
    int                              mMaxVidWidth =  1920;
    int                              mMaxVidHeight = 1080;
    int                              currentapiVersion = android.os.Build.VERSION.SDK_INT;

    private boolean                  mfSendCsd0DuringInit = false;
    private boolean                  mfAvcUHdSupported = false;
    LinearLayout                     mStatsLayout;
    boolean mfHevcSupported = false;
    
    
    
	public VrDecodeToTexture(Activity activity, String url, int maxVidWidth, int maxVidHeight) {

		mHandler = new LocalHandler();

		mUrl = url;
		mMaxVidWidth = maxVidWidth;
		mMaxVidHeight = maxVidHeight;
		mfAvcUHdSupported  = CodecInfo.isSupportedLevel("video/avc", MediaCodecInfo.CodecProfileLevel.AVCLevel51 );
		mfHevcSupported  = CodecInfo.isMimeTypeAvailable("video/hevc");
/*		if(mfAvcUHdSupported) {
			mMaxVidWidth = 3840;
			mMaxVidHeight = 2160;
		}*/
		mBuff = ByteBuffer.allocateDirect(maxBuffSize);
		//mVideoTexView.setSurfaceTextureListener(this);
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
	 				Log.d(LOG_TAG, "transition:PLAYER_CMD_RUN");	 				
				} else {
					Log.d(LOG_TAG, "transition:PLAYER_CMD_RUN ignored...");
				}
			} else if(what == PLAYER_CMD_STOP) {
				mExitPlayerLoop = true;
 				Log.d(LOG_TAG, "transition:PLAYER_CMD_STOP");
		    }  else if(what == PLAYER_CMD_INIT) {
		    	mVideoSurface = null;// todo: get it from arg
				new Thread(new Runnable() {
					@Override
					public void run() {
						//OnyxPlayerApi.initialize();
						OnyxPlayerApi.addServer(mUrl);
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
						OnyxPlayerApi.deinitialize();
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
			mCodecType = OnyxPlayerApi.getVidCodecType(mUrl);
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

			OnyxPlayerApi.startServer(mUrl);
			
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
					mFramesInBuff = OnyxPlayerApi.getNumAvailVideoFrames(mUrl);
					if (!isEOS && mFramesInBuff > 0) {
						sampleSize = OnyxPlayerApi.getVideoFrame(mUrl, mBuff, mBuff.capacity(),  100 * 1000);
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
				mFramesInBuff = OnyxPlayerApi.getNumAvailVideoFrames(mUrl);
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
							sampleSize = OnyxPlayerApi.getVideoFrame(mUrl, mBuff, mBuff.capacity(),  100 * 1000);
						}
						mPts = OnyxPlayerApi.getVideoPts(mUrl);// + 500000; // video pipeline delay
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
				long sysclk = OnyxPlayerApi.getClockUs(mUrl);
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
							while ((info.presentationTimeUs  > sysclk) && !Thread.interrupted() && !mExitPlayerLoop) {
								try {
									sleep(10);
								} catch (InterruptedException e) {
									e.printStackTrace();
									interrupt();
									break;
								}
								sysclk = OnyxPlayerApi.getClockUs(mUrl);
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
}