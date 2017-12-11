package com.mcntech.udpplayer;

import android.app.Activity;
import android.content.Context;
import android.media.MediaCodec;
import android.media.MediaCodec.BufferInfo;
import android.media.MediaFormat;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.view.Surface;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.IOException;
import java.nio.ByteBuffer;

import static android.media.MediaFormat.MIMETYPE_AUDIO_AAC;
import static android.media.MediaFormat.MIMETYPE_AUDIO_AC3;
import static android.media.MediaFormat.MIMETYPE_AUDIO_MPEG;


public class AudDecPipe implements UdpPlayerApi.FormatHandler {

	public final int PLAYER_CMD_RUN = 1;
	public final int PLAYER_CMD_STOP = 2;
	public final int PLAYER_CMD_INIT = 3;
	public final int PLAYER_CMD_DEINIT = 4;
	public final int PLAYER_CMD_REINIT = 5;

	public final String LOG_TAG = "AudDecPipe";
	String                        mUrl;
	String 						  mCodec = null;
	int                           mAudPid = 0;
	int                           mPcrPid = 0;
	private PlayerThread          mPlayerThrd = null;
	Handler                       mHandler = null;
	int							  mNumChannels;
	int							  mSamplerate;
	ByteBuffer                    mBuff;
	ByteBuffer                    mOutBuff;
	long                          mPts;
	public  int             	  mFramesInBuff = 0;
	public  int             	  mFramesRendered = 0;

	//Video Parameters
	int                              maxBuffSize = (64 * 1024);
	private MediaCodec               mDecoder = null;
	final long                       MAX_AUDIO_SYNC_THRESHOLD_US = 10000000;


    private boolean                  mfPlaying = false;
	private boolean                  mfStreamAvailable = false;
    private final Object             mPlayLock = new Object();
    private boolean                  mExitPlayerLoop = false;
    int                              currentapiVersion = android.os.Build.VERSION.SDK_INT;
	public AudRenderInterface      	 mRender = null;

	public AudDecPipe(Activity activity, String url, int strmPid, int clkPid, String codec, AudRenderInterface render) {
		Log.d(LOG_TAG, "AudDecPipe:" + url + ":" + strmPid);
		mHandler = new LocalHandler();
		Context context = activity.getApplicationContext();
		mUrl = url;
		mCodec = codec;
		mAudPid = strmPid;
		mPcrPid = clkPid;
		mBuff = ByteBuffer.allocateDirect(maxBuffSize);
		mOutBuff = ByteBuffer.allocate(maxBuffSize);

		UdpPlayerApi.registerFormatHandler(mUrl, mAudPid,this);
		UdpPlayerApi.subscribeStream(mUrl, mAudPid);
		mRender = render;
	}
	@Override
	public void onFormatChange(String message) {

		JSONObject fmt = null;
		try {
			fmt = new JSONObject(message);
			if(fmt != null) {
				mNumChannels =  fmt.getInt("channels");
				mSamplerate = fmt.getInt("samplerate");
				if (mNumChannels > 0 && mNumChannels <= 6 && mSamplerate > 0 && mSamplerate <= 48000) {
					Log.d(LOG_TAG, "MainActivity::onFormatChange");
					mfStreamAvailable = true;
					if(mfPlaying)
						mHandler.sendEmptyMessage(PLAYER_CMD_REINIT);
					else
						mHandler.sendEmptyMessage(PLAYER_CMD_INIT);

				}
			}

		} catch (JSONException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			return;
		}

	}

	private class LocalHandler extends Handler {

		LocalHandler() {
			super(Looper.getMainLooper());
		}
		@Override
         public void handleMessage(Message msg) {
             int what = msg.what;
			if(what == PLAYER_CMD_RUN) {
				if(!mfPlaying) {
					mExitPlayerLoop = false;
					mPlayerThrd = new PlayerThread();
					mPlayerThrd.start();
	 				Log.d(LOG_TAG, "transition:PLAYER_CMD_RUN");
				} else {
					Log.d(LOG_TAG, "transition:PLAYER_CMD_RUN ignored...");
				}
			} else if(what == PLAYER_CMD_STOP) {
				mExitPlayerLoop = true;
 				Log.d(LOG_TAG, "transition:PLAYER_CMD_STOP");
		    }  else if(what == PLAYER_CMD_INIT) {
				if( mfStreamAvailable) {
					new Thread(new Runnable() {
						@Override
						public void run() {
							mHandler.sendEmptyMessage(PLAYER_CMD_RUN);
						}
					}).start();
					Log.d(LOG_TAG, "transition:PLAYER_CMD_INIT");
				}
		    }   else if(what == PLAYER_CMD_DEINIT) {
 				Log.d(LOG_TAG, "transition:PLAYER_CMD_DEINIT");
 				new Thread(new Runnable() {
					@Override
					public void run() {
			 			if(Configure.mEnableVideo) {
			 				mExitPlayerLoop = true;
							waitForPlayStop();
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
    public void waitForPlayStop() {
        synchronized (mPlayLock) {
 	       while (mfPlaying) {
                try {
                    mPlayLock.wait();
                } catch (InterruptedException ie) {
                	Log.d(LOG_TAG, "transition:waitForPlayStop exception ");
                }
            }
        }
    }

    public void waitForPlayStart() {
        synchronized (mPlayLock) {
            while (!mfPlaying) {
                try {
                    mPlayLock.wait();
                } catch (InterruptedException ie) {
                	Log.d(LOG_TAG, "transition:waitForPlayStart exception ");
                }
            }
        }
    }
    

	private class PlayerThread extends Thread {
		private Surface surface;

		public PlayerThread() {
			
		}

		boolean InitDecoder( MediaCodec decoder) {
			MediaFormat format = null;
			if (mCodec.compareTo("AC3") == 0)
				format = MediaFormat.createAudioFormat(MIMETYPE_AUDIO_AC3, 48000, 2);
			else if (mCodec.compareTo("AAC") == 0)
				format = MediaFormat.createAudioFormat(MIMETYPE_AUDIO_AAC, 48000, 2);
			if (mCodec.compareTo("MP2") == 0)
				format = MediaFormat.createAudioFormat(MIMETYPE_AUDIO_MPEG , mSamplerate, mNumChannels);
			Log.d(LOG_TAG, "decoder configure");
			if (format != null) {
				decoder.configure(format, surface, null, 0);
			} else {
				Log.d(LOG_TAG, "decoder configure: failed!!!");
			}
			return true;
		}

		@Override
		public void run() {

			try {
				if(mCodec.compareTo("MP2") == 0) {
					Log.d(LOG_TAG, "decoder create audio/mp2");
					mDecoder = MediaCodec.createDecoderByType(MIMETYPE_AUDIO_MPEG);
				} else if(mCodec.compareTo("AAC") == 0){
					Log.d(LOG_TAG, "decoder create audio/aac");
					mDecoder = MediaCodec.createDecoderByType(MIMETYPE_AUDIO_AAC);
				}
				else if(mCodec.compareTo("AC3") == 0){
					Log.d(LOG_TAG, "decoder create audio/ac3");
					mDecoder = MediaCodec.createDecoderByType(MIMETYPE_AUDIO_AC3);
				}
			} catch (IOException e1) {
				// TODO Auto-generated catch block
				e1.printStackTrace();
			}
			if(mDecoder == null) {
				Log.d(LOG_TAG, "createDecoderByType Failed");
				return;
			}

			//UdpPlayerApi.subscribeStream(mUrl, mVidPid);
			
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
			InitDecoder(mDecoder);
			if(!mExitPlayerLoop) {
				try{
					Log.d(LOG_TAG, "mDecoder.start");
					mDecoder.start();
				} catch (IllegalStateException e) {
					Log.d(LOG_TAG, "IllegalStateException");
					e.printStackTrace();
				}
			}
			
			while (!Thread.interrupted() && !mExitPlayerLoop) {
				mFramesInBuff = UdpPlayerApi.getNumAvailAudioFrames(mUrl, mAudPid);
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
							sampleSize = UdpPlayerApi.getVideoFrame(mUrl, mAudPid,  mBuff, mBuff.capacity(),  100 * 1000);
						}
						mPts = UdpPlayerApi.getVideoPts(mUrl, mAudPid);// + 500000; // video pipeline delay
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
				long sysclk = UdpPlayerApi.getClockUs(mUrl, mPcrPid);
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
						if(info.presentationTimeUs > sysclk + MAX_AUDIO_SYNC_THRESHOLD_US) {
							Log.d(LOG_TAG, "FreeRun strm=" + mAudPid + " pts=" + info.presentationTimeUs + " sysClk="+ sysclk);
						} else {
							while ((info.presentationTimeUs  > sysclk) && !Thread.interrupted() && !mExitPlayerLoop) {
								try {
									sleep(10);
								} catch (InterruptedException e) {
									e.printStackTrace();
									interrupt();
									break;
								}
								sysclk = UdpPlayerApi.getClockUs(mUrl, mPcrPid);
							}
						}
						//Log.d(LOG_TAG, "releaseOutputBuffer:Begin surfacevalid=" + surface.isValid());
						if(mRender != null) {
							ByteBuffer outBuffer = mDecoder.getOutputBuffer(outIndex);
							outBuffer.get(mOutBuff.array());
							mRender.RenderAudio(mAudPid, outBuffer, info.presentationTimeUs);
						}
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
		
	//@Override
	public Handler getHandler() {
		return mHandler;
	}

}