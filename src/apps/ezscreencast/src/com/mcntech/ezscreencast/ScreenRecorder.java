package com.mcntech.ezscreencast;

import android.content.Context;
import android.hardware.display.DisplayManager;
import android.hardware.display.VirtualDisplay;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.media.MediaMuxer;
import android.media.MediaRecorder;
import android.media.projection.MediaProjection;
import android.os.Environment;
import android.util.Log;
import android.view.Surface;
import android.widget.Toast;

import java.io.File;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.concurrent.atomic.AtomicBoolean;


public class ScreenRecorder extends Thread {
    private static final String TAG = "ScreenRecorder";

    private int mWidth;
    private int mHeight;
    private int mBitRate;
    private int mFramerate;
    private int mDpi;
    private String mDstPath;
    private MediaProjection mMediaProjection;
    // parameters for the encoder
    private static final String MIME_TYPE = "video/avc"; // H.264 Advanced Video Coding
    private static final int IFRAME_INTERVAL = 1; // 10 seconds between I-frames
    private static final int TIMEOUT_US = 10000;
    
    private MediaCodec mEncoder;
    private Surface mSurface;
    private MediaMuxer mMuxer;
    private boolean mMuxerStarted = false;
    private int mVideoTrackIndex = -1;
    private AtomicBoolean mQuit = new AtomicBoolean(false);
    private MediaCodec.BufferInfo mBufferInfo = new MediaCodec.BufferInfo();
    private VirtualDisplay mVirtualDisplay;
    
    private boolean mEnableFileSave = false;
    byte[] mSpsDdata;
    byte[] mPpsDdata;
    
    private AudioRecord maudRecorder=null;  
    public int BufferSize;                    // Length of the chunks read from the hardware audio buffer  
    private Thread Record_Thread=null; // The thread filling up the audio buffer (queue)  
    private boolean isRecording = false;  

    //private static final int AUDIO_SOURCE=android.media.MediaRecorder.AudioSource.MIC;  
    private int mAudSrc;
    private int mChannelCfg;
    private int mAudSampleRate = 44100;  
    private int mAudioBitrate = 64000;
    protected MediaCodec mAudioCodec;
    private MediaCodec.BufferInfo mAudBufferInfo = new MediaCodec.BufferInfo();
    byte[] mAdtsData;
    
    private static final String AUD_ENC_MIME_TYPE = "audio/mp4a-latm";
    //private int mAudFormat = android.media.AudioFormat.ENCODING_PCM_16BIT;
    private int mAudFormat = android.media.AudioFormat.ENCODING_PCM_16BIT;
    private long mStartPtsUs = 0;
    private long mStartExtVClkUs = 0;
    private Context mContext = null;
    private boolean mfSlaveToExtClock = false;
    MpdSession mMpdSession = null;
    public ScreenRecorder(Context context, MpdSession mpdSession, int width, int height, int framerate, int bitrate, int dpi, MediaProjection mp, String dstPath) {
        super(TAG);
        mContext = context;
        mMpdSession = mpdSession; 
        mWidth = width;
        mHeight = height;
        mBitRate = bitrate;
        mFramerate = framerate;
        mDpi = dpi;
        mMediaProjection = mp;
        mDstPath = dstPath;
        
        if(ConfigDatabase.mAudioSource.equals(ConfigDatabase.AUDSRC_MIC)) {
        	mChannelCfg = android.media.AudioFormat.CHANNEL_IN_MONO;
        	mAudSrc = android.media.MediaRecorder.AudioSource.MIC;  
        } else if (ConfigDatabase.mAudioSource.equals(ConfigDatabase.AUDSRC_SYSTEM_AUDIO)) {
        	mChannelCfg = android.media.AudioFormat.CHANNEL_IN_STEREO;
        	mAudSrc = android.media.MediaRecorder.AudioSource.REMOTE_SUBMIX;  
        } else {
        	Log.d(TAG, "ScreenRecorder:Unknown audio source ");
        }
    }


    /**
     * stop task
     */
    public final void quit() {
        mQuit.set(true);
    }

    @Override
    public void run() {
        try {
            try {
                prepareEncoder();
                if(mEnableFileSave) {
                	mMuxer = new MediaMuxer(mDstPath, MediaMuxer.OutputFormat.MUXER_OUTPUT_MPEG_4);
                }
                if(ConfigDatabase.mEnableVideo || ConfigDatabase.mEnableAudio) {
                	boolean result = OnyxApi.startSession(mMpdSession, ConfigDatabase.mEnableAudio, ConfigDatabase.mEnableVideo);
                	if(!result) {
                		Toast.makeText(mContext, OnyxApi.mError, Toast.LENGTH_SHORT).show();
                		return;
                	}
                	Log.d(TAG, "ezscreencast startSession: Waiting for 2 secs");
                	try {
						Thread.sleep(2000);
					} catch (InterruptedException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
                	Log.d(TAG, "ezscreencast startSession: Waiting complete ");
                }
            } catch (IOException e) {
                throw new RuntimeException(e);
            }
            mVirtualDisplay = mMediaProjection.createVirtualDisplay(TAG + "-display",
                    mWidth, mHeight, mDpi, DisplayManager.VIRTUAL_DISPLAY_FLAG_PUBLIC,
                    mSurface, null, null);
            Log.d(TAG, "created virtual display: " + mVirtualDisplay);

            if(ConfigDatabase.mEnableAudio) {
            	satrtAudRecording();
            }
            recordVirtualDisplay();
        } finally {
            release();
        }
    }
    
    protected void satrtAudRecording(){  
        //super.onResume();  
        BufferSize=AudioRecord.getMinBufferSize(mAudSampleRate, mChannelCfg, mAudFormat);                 
        isRecording=true;  
        Record_Thread=new Thread(new Runnable() {  
        	public void run() {  
        		recordAudio();  
       }  
     },"AudioRecord Thread");  
        Record_Thread.start();  
   }  
    
    void dbgInterleave2Mono16bit(
    		byte []in_L,
    		byte []in_R,
    		byte []out,
            int num_samples)
		{
			for (int i = 0; i < num_samples; ++i) {
				int j = i * 4;
				int k = i * 2;
				out[j] = in_L[k];
				out[j+1] = in_L[k+1];
				out[j+2] = in_R[k];
				out[i+3] = in_R[k+1];
		}
	}
    
    /**
     *  Add ADTS header at the beginning of each and every AAC packet.
     *  This is needed as MediaCodec encoder generates a packet of raw
     *  AAC data.
     *
     *  Note the packetLen must count in the ADTS header itself.
     **/
    private void addADTStoPacket(byte[] packet, int packetLen) {
        int profile = 2;  //AAC LC
                          //39=MediaCodecInfo.CodecProfileLevel.AACObjectELD;
        int freqIdx = 4;  //44.1KHz
        int chanCfg = 2;  //CPE

        // fill in ADTS data
        packet[0] = (byte)0xFF;
        packet[1] = (byte)0xF9;
        packet[2] = (byte)(((profile-1)<<6) + (freqIdx<<2) +(chanCfg>>2));
        packet[3] = (byte)(((chanCfg&3)<<6) + (packetLen>>11));
        packet[4] = (byte)((packetLen&0x7FF) >> 3);
        packet[5] = (byte)(((packetLen&7)<<5) + 0x1F);
        packet[6] = (byte)0xFC;
    }
    
    private void prepareAudioEncoder(){
    	final MediaFormat audioFormat = MediaFormat.createAudioFormat(AUD_ENC_MIME_TYPE, mAudSampleRate, 1);
		audioFormat.setInteger(MediaFormat.KEY_AAC_PROFILE, MediaCodecInfo.CodecProfileLevel.AACObjectLC);
		audioFormat.setInteger(MediaFormat.KEY_CHANNEL_MASK, AudioFormat.CHANNEL_IN_MONO);
		audioFormat.setInteger(MediaFormat.KEY_BIT_RATE, mAudioBitrate);
		audioFormat.setInteger(MediaFormat.KEY_CHANNEL_COUNT, 1);
		
		try {
			mAudioCodec = MediaCodec.createEncoderByType(AUD_ENC_MIME_TYPE);
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		mAudioCodec.configure(audioFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
		mAudioCodec.start();
    }
    
    private void encodeToAudioTrack(int index) {
       	long fcVClkUs = 0;
       	long pts = 0;
       	long prevPts = 0;
        ByteBuffer encodedData = mAudioCodec.getOutputBuffer(index);

        if ((mAudBufferInfo.flags & MediaCodec.BUFFER_FLAG_CODEC_CONFIG) != 0) {
            // The codec config data was pulled out and fed to the muxer when we got
            // the INFO_OUTPUT_FORMAT_CHANGED status.
            // Ignore it.
            Log.d(TAG, "ignoring BUFFER_FLAG_CODEC_CONFIG");
            mAudBufferInfo.size = 0;
        }
        if (mAudBufferInfo.size == 0) {
            Log.d(TAG, "info.size == 0, drop it.");
            encodedData = null;
        }
 
        
        if (encodedData != null) {
            encodedData.position(mAudBufferInfo.offset);
            encodedData.limit(mAudBufferInfo.offset + mAudBufferInfo.size);
            //if(mEnableFileSave) {
            //	mMuxer.writeSampleData(mVideoTrackIndex, encodedData, mBufferInfo);
            //}
            if(ConfigDatabase.mEnableAudio) {
            	byte[] audBytes;
            	int prependLen = 0;
            	int payloadLen = mAudBufferInfo.size;

              	fcVClkUs = OnyxApi.getClockUs();
                if(mStartPtsUs == 0)
                	mStartPtsUs = mBufferInfo.presentationTimeUs;
            	
                pts = (mAudBufferInfo.presentationTimeUs - mStartPtsUs);
 
                if(pts < prevPts) {
                	Log.d(TAG, "pts_error pts=" + pts +" prevPts= " + prevPts);
                }
                prevPts=pts;
                //Log.d(TAG, "got buffer, info: size=" + mBufferInfo.size
                //        + ", pts(ms)=" + (mBufferInfo.presentationTimeUs - mStartPtsUs)/ 1000
                //        + " FcClk=" + (fcVClkUs - mStartFcVClkUs ) / 1000);
                
	            if((mBufferInfo.flags & MediaCodec.BUFFER_FLAG_KEY_FRAME) != 0) {
	            	prependLen = mAdtsData.length;
	            	audBytes = new byte[prependLen + payloadLen];
	            	System.arraycopy(mAdtsData, 0, audBytes, 0, mAdtsData.length);
	            } else {
	            	audBytes = new byte[payloadLen];	            	
	            }
	            // transfer bytes from this buffer into the given destination array
	            encodedData.get(audBytes, prependLen, payloadLen);
		        OnyxApi.sendAudioData("input0", audBytes, prependLen + payloadLen, pts, mBufferInfo.flags);
            }
            //Log.i(TAG, "sent " + mBufferInfo.size + " bytes to muxer...Flags=0x" + Integer.toHexString(mBufferInfo.flags));
        }
    }

    
    /*  
     * This runs in a separate thread reading the data from the AR buffer and dumping it  
     * into the queue (circular buffer) for processing (in java or C).  
     */  
    public void recordAudio(){  
    	int numBytes;
        byte[] AudioBytes=new byte[BufferSize]; //Array containing the audio data bytes  
        byte[] AudioData=new byte[BufferSize*2]; //Array containing the audio samples  
        final int TIMEOUT_USEC = 10000;	// 10[msec]
        try {  
        	 maudRecorder = new AudioRecord(mAudSrc,mAudSampleRate,mChannelCfg,mAudFormat,BufferSize);  
             try {  
            	 maudRecorder.startRecording();  
             } catch (IllegalStateException e){  
                   System.out.println("This didn't work very well");  
                   return;  
             }  
        } catch(IllegalArgumentException e){  
			System.out.println("This didn't work very well");  
			return;  
        }
         prepareAudioEncoder();
         int len = 0, inBufferIndex = 0, outBufferIndex = 0;
         while (isRecording && !Thread.interrupted()) {  
        	 inBufferIndex = mAudioCodec.dequeueInputBuffer(TIMEOUT_USEC); 
             if (inBufferIndex>=0) { 
            	 ByteBuffer buf = mAudioCodec.getInputBuffer(inBufferIndex); 
            	 len = maudRecorder.read(buf, BufferSize); 
            	 
				 if (len ==  AudioRecord.ERROR_INVALID_OPERATION || len == AudioRecord.ERROR_BAD_VALUE) { 
				    Log.e(TAG,"An error occured with the AudioRecord API !"); 
				} else { 
	            	 long presentationTimeUs = System.nanoTime()/1000;
					mAudioCodec.queueInputBuffer(inBufferIndex, 0, len, presentationTimeUs, 0); 
				} 
             }
             
             outBufferIndex = mAudioCodec.dequeueOutputBuffer(mAudBufferInfo, TIMEOUT_USEC);
        	
             if (outBufferIndex >= 0) {
            	 encodeToAudioTrack(outBufferIndex);
            	 mAudioCodec.releaseOutputBuffer(outBufferIndex, false);
	        } else if (outBufferIndex == MediaCodec.INFO_TRY_AGAIN_LATER) {
                try {
                    // wait 10ms
                    Thread.sleep(10);
                } catch (InterruptedException e) {
                }
	        }        	
         }  
         Log.d("MyActivity", "Record_Thread stopped");  
    }  
    
    private void stopAudRecording() {
        if (null != maudRecorder) {
            isRecording = false;  
            boolean retry = true;  
            while (retry) {  
                 try {  
                      Record_Thread.join();  
                      retry = false;  
                 } catch (InterruptedException e) {}     
            }  
        	maudRecorder.stop();
        	//maudRecorder.reset();
        	maudRecorder.release();
        	maudRecorder = null;
        }
    }
    
    private void recordVirtualDisplay() {
    	//int tid=android.os.Process.myTid();
    	//android.os.Process.setThreadPriority(android.os.Process.THREAD_PRIORITY_AUDIO);
        while (!mQuit.get()) {
            int index = mEncoder.dequeueOutputBuffer(mBufferInfo, TIMEOUT_US);
            //Log.i(TAG, "dequeue output buffer index=" + index);
            if (index == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
            	Log.i(TAG, "resetOutputFormat");
                resetOutputFormat();

            } else if (index == MediaCodec.INFO_TRY_AGAIN_LATER) {
                //Log.d(TAG, "retrieving buffers time out!");
                try {
                    // wait 10ms
                    Thread.sleep(10);
                } catch (InterruptedException e) {
                }
            } else if (index >= 0) {

                if (mEnableFileSave  && !mMuxerStarted) {
                    throw new IllegalStateException("MediaMuxer dose not call addTrack(format) ");
                }
                encodeToVideoTrack(index);
                mEncoder.releaseOutputBuffer(index, false);
                OnyxApi.UpdateStatus();
            }
        }
    }

    private void encodeToVideoTrack(int index) {
       	long fcVClkUs = 0;
       	long pts = 0;
       	long prevPts = 0;
        ByteBuffer encodedData = mEncoder.getOutputBuffer(index);

        if ((mBufferInfo.flags & MediaCodec.BUFFER_FLAG_CODEC_CONFIG) != 0) {
            // The codec config data was pulled out and fed to the muxer when we got
            // the INFO_OUTPUT_FORMAT_CHANGED status.
            // Ignore it.
            Log.d(TAG, "ignoring BUFFER_FLAG_CODEC_CONFIG");
            mBufferInfo.size = 0;
        }
        if (mBufferInfo.size == 0) {
            Log.d(TAG, "info.size == 0, drop it.");
            encodedData = null;
        }
 
        
        if (encodedData != null) {
            encodedData.position(mBufferInfo.offset);
            encodedData.limit(mBufferInfo.offset + mBufferInfo.size);
            if(mEnableFileSave) {
            	mMuxer.writeSampleData(mVideoTrackIndex, encodedData, mBufferInfo);
            }
         // Retrieve all bytes in the buffer
            if(ConfigDatabase.mEnableVideo) {
            	byte[] vidBytes;
            	int prependLen = 0;
            	int payloadLen = mBufferInfo.size;
            	//encodedData.clear();

              	fcVClkUs = OnyxApi.getClockUs();
                if(mStartPtsUs == 0)
                	mStartPtsUs = mBufferInfo.presentationTimeUs;
                if(mStartExtVClkUs == 0)
                	mStartExtVClkUs = fcVClkUs;
            	
                if(mfSlaveToExtClock) {
	                pts =  (fcVClkUs - mStartExtVClkUs);
                } else {
                	pts = (mBufferInfo.presentationTimeUs - mStartPtsUs);
                }
 
                if(pts < prevPts) {
                	Log.d(TAG, "pts_error pts=" + pts +" prevPts= " + prevPts);
                }
                prevPts=pts;
                
	            if((mBufferInfo.flags & MediaCodec.BUFFER_FLAG_KEY_FRAME) != 0) {
	            	prependLen = mSpsDdata.length + mPpsDdata.length;
	            	vidBytes = new byte[prependLen + payloadLen];
	            	System.arraycopy(mSpsDdata, 0, vidBytes, 0, mSpsDdata.length);
	            	System.arraycopy(mPpsDdata, 0, vidBytes, mSpsDdata.length, mPpsDdata.length);
	            } else {
	            	vidBytes = new byte[payloadLen];	            	
	            }
	            // transfer bytes from this buffer into the given destination array
	            encodedData.get(vidBytes, prependLen, payloadLen);
		        OnyxApi.sendVideoData("input0", vidBytes, prependLen + payloadLen, pts, mBufferInfo.flags);
            }
            //Log.i(TAG, "sent " + mBufferInfo.size + " bytes to muxer...Flags=0x" + Integer.toHexString(mBufferInfo.flags));
        }
    }

    private void resetOutputFormat() {
        // should happen before receiving buffers, and should only happen once
        if (mEnableFileSave && mMuxerStarted) {
            throw new IllegalStateException("output format already changed!");
        }
        MediaFormat newFormat = mEncoder.getOutputFormat();

        Log.i(TAG, "output format changed.\n new format: " + newFormat.toString());
        if(mEnableFileSave) {
            mVideoTrackIndex = mMuxer.addTrack(newFormat);
        	mMuxer.start();
        	mMuxerStarted = true;
        }
        
        ByteBuffer bufferSps = newFormat.getByteBuffer("csd-0");
        mSpsDdata = new byte[bufferSps.limit()];
        bufferSps.get(mSpsDdata);
        ByteBuffer bufferPps = newFormat.getByteBuffer("csd-1");
        mPpsDdata = new byte[bufferPps.limit()];
        bufferPps.get(mPpsDdata);
        Log.i(TAG, "started media muxer, videoIndex=" + mVideoTrackIndex);
    }

    private void prepareEncoder() throws IOException {

        MediaFormat format = MediaFormat.createVideoFormat(MIME_TYPE, mWidth, mHeight);
        format.setInteger(MediaFormat.KEY_COLOR_FORMAT,
                MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface);
        format.setInteger(MediaFormat.KEY_BIT_RATE, mBitRate);
        format.setInteger(MediaFormat.KEY_FRAME_RATE, mFramerate);
        format.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, IFRAME_INTERVAL);

        Log.d(TAG, "created video format: " + format);
        mEncoder = MediaCodec.createEncoderByType(MIME_TYPE);
        mEncoder.configure(format, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
        mSurface = mEncoder.createInputSurface();
        Log.d(TAG, "created input surface: " + mSurface);
        mEncoder.start();
    }

    private void release() {
        if (mEncoder != null) {
            mEncoder.stop();
            mEncoder.release();
            mEncoder = null;
        }
        if (mVirtualDisplay != null) {
            mVirtualDisplay.release();
        }
        if (mMediaProjection != null) {
            mMediaProjection.stop();
        }
        if (mMuxer != null) {
            mMuxer.stop();
            mMuxer.release();
            mMuxer = null;
        }
        
        if(ConfigDatabase.mEnableAudio) {
        	stopAudRecording();
        }
        
        if(ConfigDatabase.mEnableVideo) {
        	OnyxApi.stopSession();
         	//OnyxApi.deinitialize();
        }
        //finish();
        System.exit(0);
    }
}
