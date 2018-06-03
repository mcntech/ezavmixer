package com.mcntech.udpplayer;

import android.app.Activity;
import android.content.Context;
import com.mcntech.udpplayer.AudDecApi;
import com.mcntech.udpplayer.AudDecApi.BufferInfo;
import android.media.MediaFormat;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.view.Surface;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import static android.media.MediaFormat.MIMETYPE_AUDIO_AAC;
import static android.media.MediaFormat.MIMETYPE_AUDIO_AC3;
import static android.media.MediaFormat.MIMETYPE_AUDIO_MPEG;


public class AudDecPipeJd implements UdpPlayerApi.FormatHandler, VrRenderDb.AudDecPipeBase {

	public final int PLAYER_CMD_RUN = 1;
	public final int PLAYER_CMD_STOP = 2;
	public final int PLAYER_CMD_INIT = 3;
	public final int PLAYER_CMD_DEINIT = 4;
	public final int PLAYER_CMD_REINIT = 5;



	public final String LOG_TAG = "AudDecPipe";
	String                        mUrl;
	String 						  mCodec = null;
	public int                    mAudPid = 0;
	int                           mPcrPid = 0;
	private PlayerThread          mPlayerThrd = null;
	Handler                       mHandler = null;
	int							  mNumChannels;
	int							  mSamplerate;
	ByteBuffer                    mBuff;

	ByteBuffer					  mDecInBuff;
	ByteBuffer					  mDecOutBuff;
	ByteBuffer					  mFreqOutBuff;
	long                          mPts;
	public  int             	  mFramesInBuff = 0;
	public  int             	  mFramesRendered = 0;

	//Video Parameters
	int                              maxBuffSize = (64 * 1024);
	private AudDecApi               mDecoder = null;
	final long                       MAX_AUDIO_SYNC_THRESHOLD_US = 10000000;


    private boolean                  mfPlaying = false;
	private boolean                  mfStreamAvailable = false;
    private final Object             mPlayLock = new Object();
    private boolean                  mExitPlayerLoop = false;
    int                              currentapiVersion = android.os.Build.VERSION.SDK_INT;
	public AudRenderInterface      	 mRender = null;

	boolean mfMp2Supported = false;
	boolean mfAc3Supported = false;
	boolean mfAacSupported = false;


	public AudDecPipeJd(Activity activity, String url, int strmPid, int clkPid, String codec, AudRenderInterface render) {
		Log.d(LOG_TAG, "AudDecPipe:" + url + ":" + strmPid);
		mHandler = new LocalHandler();
		Context context = activity.getApplicationContext();
		mUrl = url;
		mCodec = codec;
		mAudPid = strmPid;
		mPcrPid = clkPid;
		mBuff = ByteBuffer.allocateDirect(maxBuffSize);

		mDecInBuff = ByteBuffer.allocateDirect(maxBuffSize);
		mDecOutBuff = ByteBuffer.allocateDirect(maxBuffSize);
		mFreqOutBuff = ByteBuffer.allocateDirect(maxBuffSize);
		mfMp2Supported  = CodecInfo.isMimeTypeAvailable(MIMETYPE_AUDIO_MPEG);
		mfAacSupported  = CodecInfo.isMimeTypeAvailable(MIMETYPE_AUDIO_AAC);
		mfAc3Supported  = CodecInfo.isMimeTypeAvailable(MIMETYPE_AUDIO_AC3);

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
						mExitPlayerLoop = true;
						waitForPlayStop();
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

		@Override
		public void run() {

			try {
				if(mCodec.compareToIgnoreCase("MP2") == 0) {
					Log.d(LOG_TAG, "decoder create audio/mp2");
					mDecoder = AudDecApi.createDecoderByType(MIMETYPE_AUDIO_MPEG);
				} else if(mCodec.compareToIgnoreCase("AAC") == 0){
					Log.d(LOG_TAG, "decoder create audio/aac");
					mDecoder = AudDecApi.createDecoderByType(MIMETYPE_AUDIO_AAC);
				}
				else if(mCodec.compareToIgnoreCase("AC3") == 0){
					Log.d(LOG_TAG, "decoder create audio/ac3");
					mDecoder = AudDecApi.createDecoderByType(MIMETYPE_AUDIO_AC3);
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

			BufferInfo info = new BufferInfo();
			boolean isEOS = false;
			int sampleSize = 0;
			boolean fFrameAvail = false;
			
			synchronized (mPlayLock) {
				mfPlaying = true;
				mPlayLock.notifyAll();
			}			
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
				//Log.d(LOG_TAG, "getNumAvailAudioFrames " + mFramesInBuff);
				if (!isEOS && mFramesInBuff > 0) {

					boolean fInputFull;
					
					try {
						fInputFull = mDecoder.isInputFull() == 1;
					} catch (IllegalStateException e) {
						Log.d(LOG_TAG, "Decoder IllegalStateException" + e);
						break;
					}
					//Log.d(LOG_TAG, "fInputFull=" + fInputFull);
					if (!fInputFull) {
						if(fFrameAvail) {
							fFrameAvail = false;
						} else {
							sampleSize = UdpPlayerApi.getAudioFrame(mUrl, mAudPid,  mBuff, mBuff.capacity(),  100 * 1000);
						}
						mPts = UdpPlayerApi.getAudioPts(mUrl, mAudPid);// + 500000; // video pipeline delay
						if (sampleSize <= 0) {
							// We shouldn't stop the playback at this point, just pass the EOS
							// flag to decoder, we will get it again from the
							// dequeueOutputBuffer
							Log.d(LOG_TAG, "InputBuffer BUFFER_FLAG_END_OF_STREAM");														
							//mDecoder.sendInputData(, 0, 0, 0, AudDecApi.BUFFER_FLAG_END_OF_STREAM);
							isEOS = true;
						} else {
							//Log.d(LOG_TAG, "sampleSize = " + sampleSize);
							mBuff.limit(sampleSize);
							mDecInBuff.position(0);
							mDecInBuff.clear();
							mDecInBuff.put(mBuff.array(), 0, sampleSize);
							//buffer.put(mBuff, 0, sampleSize);
							mDecInBuff.position(0);
							mDecInBuff.limit(sampleSize);
														
							try {
								mDecoder.sendInputData(mDecInBuff, sampleSize, mPts, 0);
							} catch (IllegalStateException e) {
								Log.d(LOG_TAG, "Decoder: queueInputBuffer: IllegalStateException");
								break;
							}
						}
					}
					
				}

				//Log.d(LOG_TAG, "isOutputEmpty=" + mExitPlayerLoop);
				boolean fOutPcmEmpty;
				try {
					fOutPcmEmpty = mDecoder.isOutputEmpty() == 1;
				}  catch (IllegalStateException e) {
					Log.d(LOG_TAG, "Decoder isOutputEmpty: exception: " + e);
					break;
				}
				//Log.d(LOG_TAG, "isOutputEmpty=" + fOutPcmEmpty);
				long sysclk = UdpPlayerApi.getClockUs(mUrl, mPcrPid);

				if (!fOutPcmEmpty) {
					//Log.v(LOG_TAG, " presentationTime= " + (info.presentationTimeUs / 1000) + " fcvclk=" + sysclk / 1000 + " wait=" + (info.presentationTimeUs - sysclk) / 1000);
					if(info.presentationTimeUs > sysclk + MAX_AUDIO_SYNC_THRESHOLD_US) {
						//Log.d(LOG_TAG, "FreeRun strm=" + mAudPid + " pts=" + info.presentationTimeUs + " sysClk="+ sysclk);
					} else {
						//while ((info.presentationTimeUs  > sysclk) && !Thread.interrupted() && !mExitPlayerLoop)
						{
							try {
								sleep(10);
							} catch (InterruptedException e) {
								e.printStackTrace();
								//interrupt();
								Log.d(LOG_TAG, "InterruptedException" + e);
								break;
							}
							//sysclk = UdpPlayerApi.getClockUs(mUrl, mPcrPid);
						}

					}

					if(mRender != null) {
						mDecOutBuff.position(0);
						int len = mDecoder.getOutputData(mDecOutBuff, maxBuffSize);
						mDecOutBuff.limit(len);
						mRender.RenderAudio(mAudPid, mDecOutBuff, info.presentationTimeUs);
					}
					mFramesRendered++;
					//Log.d(LOG_TAG, "mFramesRendered:" + mFramesRendered);
				} else {
					//Log.d(LOG_TAG, " fOutPcmEmpty= " + fOutPcmEmpty);
				}
				// All decoded frames have been rendered, we can stop playing now
				if ((info.flags & AudDecApi.BUFFER_FLAG_END_OF_STREAM) != 0) {
					Log.d(LOG_TAG, "OutputBuffer BUFFER_FLAG_END_OF_STREAM");
					break;
				}
			}
			boolean thrstate = Thread.interrupted();
			Log.d(LOG_TAG, "Exiting PlayerThread...thread int state = " + thrstate + " mExitPlayerLoop=" + mExitPlayerLoop);
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

	@Override
	public int getStreamId()
	{
		return mAudPid;
	}

	@Override
	public int getNumChannels()
	{
		return mNumChannels;
	}

	@Override
	public int getFreqData(short data[], int spectWidth)
	{
		//Random rand = new Random();
		//rand.nextBytes(data);
		if( mDecoder != null && mDecoder.isFreqEmpty() != 1) {
			mFreqOutBuff.position(0);
			int nFreqLen = mDecoder.getFreqData(mFreqOutBuff, maxBuffSize);
			mFreqOutBuff.limit(nFreqLen);
			//mFreqOutBuff.get(data, 0, data.length	);
			int j = 0;
			int scale = (nFreqLen/2) / (spectWidth * mNumChannels);;
			mFreqOutBuff.order(ByteOrder.LITTLE_ENDIAN);
			for(int i=0; i < spectWidth * mNumChannels && i < nFreqLen / 2 && j <  nFreqLen/2; i++) {
				data[i] = mFreqOutBuff.getShort(j);
				 j += scale;
			}
		}
		return 0;
	}
}