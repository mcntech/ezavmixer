package com.mcntech.udpplayer;

import java.io.IOException;
import java.nio.ByteBuffer;


import android.app.Activity;
import android.content.Context;
import android.graphics.SurfaceTexture;
import android.media.MediaCodec;
import android.media.MediaCodec.BufferInfo;
import android.media.MediaCodecInfo;

import android.media.MediaFormat;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.Surface;
import android.view.TextureView;
import android.widget.BaseAdapter;
import android.widget.LinearLayout;
import android.widget.ListView;


import com.mcntech.udpplayer.VrRenderDb.DecPipeBase;
import com.mcntech.udpplayer.Configure;
import com.mcntech.udpplayer.UdpPlayerApi;
import com.mcntech.udpplayer.UdpPlayerApi.ProgramHandler;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;


public class DecodePipe  implements DecPipeBase, ProgramHandler, TextureView.SurfaceTextureListener {
	
	public final String LOG_TAG = "DecodePipe";
	String                        mUrl;
	int                           mStrmId;
	String 						  mCodec;
	private PlayerThread          mVidPlayer = null;
	Handler                       mHandler = null;
	TextureView                   mVideoTexView = null;
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

    private final Object             mPlayLock = new Object();
    private boolean                  mExitPlayerLoop = false;
    int                              mMaxVidWidth =  1920;
    int                              mMaxVidHeight = 1080;
    int                              currentapiVersion = android.os.Build.VERSION.SDK_INT;

    private boolean                  mfSendCsd0DuringInit = false;
    private boolean                  mfAvcUHdSupported = false;
    LinearLayout                     mStatsLayout;
    boolean mfHevcSupported = false;

	ProgramHandler mProgramHandler;

	class AudioStream
	{
		int PID;
		int CodecType;
	}

	public DecodePipe(Activity activity, String url, int strmId, TextureView textureView, int maxVidWidth, int maxVidHeight) {

		mHandler = new LocalHandler();
		mVideoTexView = textureView;
		Context context = activity.getApplicationContext();
		Configure.loadSavedPreferences(context, false);
		mUrl = url;//Configure.mRtspUrl1;
		//mCodec = codec;
		mStrmId = strmId;
		mMaxVidWidth = maxVidWidth;
		mMaxVidHeight = maxVidHeight;
		mfAvcUHdSupported  = CodecInfo.isSupportedLevel("video/avc", MediaCodecInfo.CodecProfileLevel.AVCLevel51 );
		mfHevcSupported  = CodecInfo.isMimeTypeAvailable("video/hevc");
/*		if(mfAvcUHdSupported) {
			mMaxVidWidth = 3840;
			mMaxVidHeight = 2160;
		}*/
		mBuff = ByteBuffer.allocateDirect(maxBuffSize);
		mVideoTexView.setSurfaceTextureListener(this);
		UdpPlayerApi.registerProgramHandler(mUrl, mStrmId,this);
		UdpPlayerApi.subscribeProgram(mUrl, mStrmId);
	}

	@Override
	public void onPsiPmtChange(String message) {
		Log.d(LOG_TAG, "MainActivity::onPsiChange");
		JSONObject pgm = null;
		try {
			pgm = new JSONObject(message);
			if(pgm != null) {
				int vidPid = 0;

				JSONArray esList = pgm.getJSONArray("streams");
				for (int j = 0; j < esList.length(); j++) {
					JSONObject es = esList.getJSONObject(j);
					String codec = es.getString("codec");
					vidPid = es.getInt("pid");
					if(codec.compareToIgnoreCase("mpeg2") == 0 ||
							codec.compareToIgnoreCase("h264") == 0 ||
							codec.compareToIgnoreCase("h265") == 0){
						mCodec = codec;
						// Todo : set codec type and post msg to start decode;
						// TODO : mRemoteNodeList.add(node);
						//mHandler.sendEmptyMessage(PLAYER_CMD_INIT);
					}
				}
			}

		} catch (JSONException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			return;
		}

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
				new Thread(new Runnable() {
					@Override
					public void run() {
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
						//OnyxPlayerApi.deinitialize();
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
			if(mCodec.compareTo("H265") == 0) // HEVC
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
			if(mCodec.compareTo("H265") == 0) // HEVC
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

			try {
				if(mCodec.compareTo("H265") == 0) {
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

			UdpPlayerApi.subscribeStream(mUrl, mStrmId);
			
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
							sampleSize = UdpPlayerApi.getVideoFrame(mUrl, mStrmId,  mBuff, mBuff.capacity(),  100 * 1000);
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
							while ((info.presentationTimeUs  > sysclk) && !Thread.interrupted() && !mExitPlayerLoop) {
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
	public void onSurfaceTextureAvailable(SurfaceTexture surface, int width,
			int height) {	 	
		mVideoSurface = new Surface(surface);
		mHandler.sendEmptyMessage(PLAYER_CMD_INIT);	 	
	}

	@Override
	public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width,
			int height) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
		mHandler.sendEmptyMessage(PLAYER_CMD_DEINIT);
		return false;
	}

	@Override
	public void onSurfaceTextureUpdated(SurfaceTexture surface) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public Handler getHandler() {
		return mHandler;
	}

	@Override
	public SurfaceTexture getSurfaceTexture() {
		// TODO Auto-generated method stub
		return null;
	}

}