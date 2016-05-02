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



public class VrDecodePipe  implements GLSurfaceView.Renderer {
	public static final String TAG = "VrDecodePipe";
	public final String LOG_TAG = "VrDecodePipe";
	String                        mUrl;                   
	private PlayerThread          mVidPlayer = null;
	Handler                       mHandler;
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
    
    final int                        PLAYER_CMD_RUN = 1;
    final int                        PLAYER_CMD_STOP = 2;
    final int                        PLAYER_CMD_INIT = 3;
    final int                        PLAYER_CMD_DEINIT = 4;    
    
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
    
    
    private FloatBuffer mVertexBuffer;
    private FloatBuffer m43VertexBuffer;
    private FloatBuffer mTexCoordBuffer;
    
    private final static float SNAPSHOT_SCALE = 65.5f;
    private final static float RATIO = 4.0f/3.0f;
    private final static float DISTANCE = 135.0f;
    
    // x, y,
    private final float mVertexData[] =
            {
                    -SNAPSHOT_SCALE,  -SNAPSHOT_SCALE * 1.5f,
                    -SNAPSHOT_SCALE, SNAPSHOT_SCALE  * 1.5f,
                    SNAPSHOT_SCALE, SNAPSHOT_SCALE  * 1.5f,
                    SNAPSHOT_SCALE, -SNAPSHOT_SCALE  * 1.5f
            };

    private final float m43VertexData[] =
            {
                    -SNAPSHOT_SCALE *RATIO,  -SNAPSHOT_SCALE,
                    -SNAPSHOT_SCALE *RATIO, SNAPSHOT_SCALE,
                    SNAPSHOT_SCALE *RATIO, SNAPSHOT_SCALE,
                    SNAPSHOT_SCALE *RATIO, -SNAPSHOT_SCALE
            };

    // u,v
    private final float mTexCoordData[] =
            {
                    0.0f, 0.0f,
                    0.0f, 1.0f,
                    1.0f, 1.0f,
                    1.0f, 0.0f
            };
    private List<Snapshot> mDots;
    
    private Quaternion mCameraQuat;
    private float[] mViewMatrix = new float[16];
    private float[] mProjectionMatrix = new float[16];
    private SensorFusion mSensorFusion;
    
    private final static int CAMERA = 0;
    private final static int SNAPSHOT = 1;

    private int[] mProgram = new int[2];
    private int[] mVertexShader = new int[2];
    private int[] mFragmentShader = new int[2];
    private int[] mPositionHandler = new int[2];
    private int[] mTexCoordHandler = new int[2];
    private int[] mTextureHandler = new int[2];
    private int[] mAlphaHandler = new int[2];
    private int[] mMVPMatrixHandler = new int[2];
    
    
    private SurfaceTexture mCameraSurfaceTex;
    private int mCameraTextureId;
    private Snapshot mCameraBillboardLeft;
    private Snapshot mCameraBillboardRight;
    //private Snapshot mViewfinderBillboard;
    private Context mContext;
    private Quaternion mTempQuaternion;
    private float[] mMVPMatrix = new float[16];
    
	public VrDecodePipe(Activity activity, String url, int maxVidWidth, int maxVidHeight) {

		Context context = activity.getApplicationContext();
       mSensorFusion = new SensorFusion(context);
        mCameraQuat = new Quaternion();
        mContext = activity;
        mTempQuaternion = new Quaternion();
	        
		mHandler = new LocalHandler();
		//mVideoTexView = textureView;

		Configure.loadSavedPreferences(context, false);
		mUrl = url;//Configure.mRtspUrl1;
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


	    private float[] matrixFromEuler(float rx, float ry, float rz, float tx, float ty, float tz) {
	        Quaternion quat = new Quaternion();
	        quat.fromEuler(rx,ry,rz);
	        float[] matrix = quat.getMatrix();

	        Matrix.translateM(matrix, 0, tx, ty, tz);

	        return matrix;
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

	
	
	//===============================================================================
    /**
     * Helper function to compile a shader.
     *
     * @param shaderType The shader type.
     * @param shaderSource The shader source code.
     * @return An OpenGL handle to the shader.
     */
    public static int compileShader(final int shaderType, final String shaderSource) {
        int shaderHandle = GLES20.glCreateShader(shaderType);

        if (shaderHandle != 0) {
            // Pass in the shader source.
            GLES20.glShaderSource(shaderHandle, shaderSource);

            // Compile the shader.
            GLES20.glCompileShader(shaderHandle);

            // Get the compilation status.
            final int[] compileStatus = new int[1];
            GLES20.glGetShaderiv(shaderHandle, GLES20.GL_COMPILE_STATUS, compileStatus, 0);

            // If the compilation failed, delete the shader.
            if (compileStatus[0] == 0) {
                Log.e(TAG, "Error compiling shader: " + GLES20.glGetShaderInfoLog(shaderHandle));
                GLES20.glDeleteShader(shaderHandle);
                shaderHandle = 0;
            }
        }

        if (shaderHandle == 0) {
            throw new RuntimeException("Error creating shader.");
        }

        return shaderHandle;
    }
    
    

    /**
     * Stores the information about each snapshot displayed in the sphere
     */
    private class Snapshot {
        private float[]mModelMatrix;
        private int mTextureData;
        private Bitmap mBitmapToLoad;
        private boolean mIsFourToThree;
        private int mMode;
        private boolean mIsVisible = true;
        private float mAlpha = 1.0f;
        private float mAutoAlphaX;
        private float mAutoAlphaY;

        public Snapshot() {
            mIsFourToThree = true;
            mMode = SNAPSHOT;
        }

        public Snapshot(boolean isFourToThree) {
            mIsFourToThree = isFourToThree;
            mMode = SNAPSHOT;
        }

        public void setVisible(boolean visible) {
            mIsVisible = visible;
        }

        /**
         * Sets whether to use the CAMERA shaders or the SNAPSHOT shaders
         * @param mode CAMERA or SNAPSHOT
         */
        public void setMode(int mode) {
            mMode = mode;
        }

        public void setTexture(Bitmap tex) {
            mBitmapToLoad = tex;
        }

        public void setTextureId(int id) {
            mTextureData = id;
        }

        public void setAlpha(float alpha) {
            mAlpha = alpha;
        }

        public void setAutoAlphaAngle(float x, float y) {
            mAutoAlphaX = x;
            mAutoAlphaY = y;
        }

        public float getAutoAlphaX() {
            return mAutoAlphaX;
        }

        public float getAutoAlphaY() {
            return mAutoAlphaY;
        }

        private void loadTexture() {
            // Load the snapshot bitmap as a texture to bind to our GLES20 program
            int texture[] = new int[1];

            GLES20.glGenTextures(1, texture, 0);
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, texture[0]);

            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D,
                    GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_NEAREST);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D,
                    GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_NEAREST);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D,
                    GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_CLAMP_TO_EDGE);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D,
                    GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_CLAMP_TO_EDGE);

            GLUtils.texImage2D(GLES20.GL_TEXTURE_2D, 0, mBitmapToLoad, 0);
            //tex.recycle();

            if(texture[0] == 0){
                Log.e(TAG, "Unable to attribute texture to quad");
            }

            mTextureData = texture[0];
            mBitmapToLoad = null;
        }

        public void draw() {
            if (!mIsVisible) return;

            if (mBitmapToLoad != null) {
                loadTexture();
            }

            GLES20.glUseProgram(mProgram[mMode]);
            if (mIsFourToThree) {
                m43VertexBuffer.position(0);
            } else {
                mVertexBuffer.position(0);
            }
            mTexCoordBuffer.position(0);

            GLES20.glEnableVertexAttribArray(mTexCoordHandler[mMode]);
            GLES20.glEnableVertexAttribArray(mPositionHandler[mMode]);

            if (mIsFourToThree) {
                GLES20.glVertexAttribPointer(mPositionHandler[mMode],
                        2, GLES20.GL_FLOAT, false, 8, m43VertexBuffer);
            } else {
                GLES20.glVertexAttribPointer(mPositionHandler[mMode],
                        2, GLES20.GL_FLOAT, false, 8, mVertexBuffer);
            }
            GLES20.glVertexAttribPointer(mTexCoordHandler[mMode], 2,
                    GLES20.GL_FLOAT, false, 8, mTexCoordBuffer);

            // This multiplies the view matrix by the model matrix, and stores the
            // result in the MVP matrix (which currently contains model * view).
            Matrix.multiplyMM(mMVPMatrix, 0, mViewMatrix, 0, mModelMatrix, 0);

            // This multiplies the modelview matrix by the projection matrix, and stores
            // the result in the MVP matrix (which now contains model * view * projection).
            Matrix.multiplyMM(mMVPMatrix, 0, mProjectionMatrix, 0, mMVPMatrix, 0);

            // Pass in the combined matrix.
            GLES20.glUniformMatrix4fv(mMVPMatrixHandler[mMode], 1, false, mMVPMatrix, 0);

            GLES20.glUniform1f(mAlphaHandler[mMode], mAlpha);

            GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, mTextureData);

            GLES20.glUniform1i(mTextureHandler[mMode], 0);

            GLES20.glDrawArrays(GLES20.GL_TRIANGLE_FAN, 0, 4);
        }
    }
	@Override
	public void onSurfaceCreated(GL10 gl, EGLConfig config) {

        GLES20.glClearColor(0.5f, 0.5f, 0.5f, 0.5f);
        
        // Initialize plane vertex data and texcoords data
        ByteBuffer bb_data = ByteBuffer.allocateDirect(mVertexData.length * 4);
        bb_data.order(ByteOrder.nativeOrder());
        ByteBuffer bb_43data = ByteBuffer.allocateDirect(m43VertexData.length * 4);
        bb_43data.order(ByteOrder.nativeOrder());
        ByteBuffer bb_texture = ByteBuffer.allocateDirect(mTexCoordData.length * 4);
        bb_texture.order(ByteOrder.nativeOrder());
        
        mVertexBuffer = bb_data.asFloatBuffer();
        mVertexBuffer.put(mVertexData);
        mVertexBuffer.position(0);
        m43VertexBuffer = bb_43data.asFloatBuffer();
        m43VertexBuffer.put(m43VertexData);
        m43VertexBuffer.position(0);
        mTexCoordBuffer = bb_texture.asFloatBuffer();
        mTexCoordBuffer.put(mTexCoordData);
        mTexCoordBuffer.position(0);
        
        // Simple GLSL vertex/fragment, as GLES2 doesn't have the classical fixed pipeline
        final String vertexShader =
                "uniform mat4 u_MVPMatrix; \n"
                        + "attribute vec4 a_Position;     \n"
                        + "attribute vec2 a_TexCoordinate;\n"
                        + "varying vec2 v_TexCoordinate;  \n"
                        + "void main()                    \n"
                        + "{                              \n"
                        + "   v_TexCoordinate = a_TexCoordinate;\n"
                        + "   gl_Position = u_MVPMatrix * a_Position;   \n"
                        + "}                              \n";

        final String fragmentShader =
                        "precision mediump float;       \n"
                        + "uniform sampler2D u_Texture;   \n"
                        + "varying vec2 v_TexCoordinate;  \n"
                        + "uniform float f_Alpha;\n"
                        + "void main()                    \n"
                        + "{                              \n"
                        + "   gl_FragColor = texture2D(u_Texture, v_TexCoordinate);\n"
                        + "   gl_FragColor.a = gl_FragColor.a * f_Alpha;"
                        + "}                              \n";

        // As the camera preview is stored in the OES external slot, we need a different shader
        final String camPreviewShader = "#extension GL_OES_EGL_image_external : require\n"
                + "precision mediump float;       \n"
                + "uniform samplerExternalOES u_Texture;   \n"
                + "varying vec2 v_TexCoordinate;  \n"
                + "uniform float f_Alpha;\n"
                + "void main()                    \n"
                + "{                              \n"
                + "   gl_FragColor = texture2D(u_Texture, v_TexCoordinate);\n"
                + "   gl_FragColor.a = gl_FragColor.a * f_Alpha;"
                + "}                              \n";

        mVertexShader[CAMERA] = compileShader(GLES20.GL_VERTEX_SHADER, vertexShader);
        mFragmentShader[CAMERA] = compileShader(GLES20.GL_FRAGMENT_SHADER, camPreviewShader);

        mVertexShader[SNAPSHOT] = compileShader(GLES20.GL_VERTEX_SHADER, vertexShader);
        mFragmentShader[SNAPSHOT] = compileShader(GLES20.GL_FRAGMENT_SHADER, fragmentShader);
        
        
        // create the program and bind the shader attributes
        for (int i = 0; i < 2; i++) {
            mProgram[i] = GLES20.glCreateProgram();
            GLES20.glAttachShader(mProgram[i], mFragmentShader[i]);
            GLES20.glAttachShader(mProgram[i], mVertexShader[i]);
            GLES20.glLinkProgram(mProgram[i]);

            int[] linkStatus = new int[1];
            GLES20.glGetProgramiv(mProgram[i], GLES20.GL_LINK_STATUS, linkStatus, 0);

            if (linkStatus[0] == 0) {
                throw new RuntimeException("Error linking shaders");
            }
            mPositionHandler[i]     = GLES20.glGetAttribLocation(mProgram[i], "a_Position");
            mTexCoordHandler[i]     = GLES20.glGetAttribLocation(mProgram[i], "a_TexCoordinate");
            mMVPMatrixHandler[i]    = GLES20.glGetUniformLocation(mProgram[i], "u_MVPMatrix");
            mTextureHandler[i]      = GLES20.glGetUniformLocation(mProgram[i], "u_Texture");
            mAlphaHandler[i]      = GLES20.glGetUniformLocation(mProgram[i], "f_Alpha");
        }

		initCameraBillboard();
	}

	@Override
	public void onSurfaceChanged(GL10 gl, int width, int height) {
        // Set the OpenGL viewport to the same size as the surface.
        GLES20.glViewport(0, 0, width, height);

        // We use here a field of view of 40, which is mostly fine for a camera app representation
        final float hfov = 90f;

        // Create a new perspective projection matrix. The height will stay the same
        // while the width will vary as per aspect ratio.
        //final float ratio = 640.0f / 480.0f;
        final float ratio = 1280.0f / 720.0f;
        final float near = 0.1f;
        final float far = 1500.0f;
        final float left = (float) Math.tan(hfov * Math.PI / 360.0f) * near;
        final float right = -left;
        final float bottom = ratio * right / 1.0f;
        final float top = ratio * left / 1.0f;

        Matrix.frustumM(mProjectionMatrix, 0, left, right, bottom, top, near, far);
	}

	
	   private void initCameraBillboard() {
	        int texture[] = new int[1];

	        GLES20.glGenTextures(1, texture, 0);
	        mCameraTextureId = texture[0];

	        if (mCameraTextureId == 0) {
	            throw new RuntimeException("CAMERA TEXTURE ID == 0");
	        }

	        GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, mCameraTextureId);
	        // Can't do mipmapping with camera source
	        GLES20.glTexParameterf(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MIN_FILTER,
	                GLES20.GL_LINEAR);
	        GLES20.glTexParameterf(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MAG_FILTER,
	                GLES20.GL_LINEAR);
	        // Clamp to edge is the only option
	        GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_WRAP_S,
	                GLES20.GL_CLAMP_TO_EDGE);
	        GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_WRAP_T,
	                GLES20.GL_CLAMP_TO_EDGE);

	        mCameraSurfaceTex = new SurfaceTexture(mCameraTextureId);
	        //mCameraSurfaceTex.setDefaultBufferSize(640, 480);
	        mCameraSurfaceTex.setDefaultBufferSize(1280, 720);

	        mCameraBillboardLeft = new Snapshot(false);
	        mCameraBillboardLeft.setTextureId(mCameraTextureId);
	        mCameraBillboardLeft.setMode(CAMERA);

	        mCameraBillboardRight = new Snapshot(false);
	        mCameraBillboardRight.setTextureId(mCameraTextureId);
	        mCameraBillboardRight.setMode(CAMERA);

	        // Setup viewfinder billboard
	        //mViewfinderBillboard = new Snapshot(false);
	       // mViewfinderBillboard.setTexture(BitmapFactory.decodeResource(mContext.getResources(),
	        //        R.drawable.ic_picsphere_viewfinder));
	        
			mVideoSurface = new Surface(mCameraSurfaceTex);
			mHandler.sendEmptyMessage(PLAYER_CMD_INIT);	
	    }
	@Override
	public void onDrawFrame(GL10 gl) {
		// TODO Auto-generated method stub
		mCameraSurfaceTex.updateTexImage();
		
		
	       GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT | GLES20.GL_DEPTH_BUFFER_BIT);

	        GLES20.glBlendFunc(GLES20.GL_SRC_ALPHA, GLES20.GL_ONE_MINUS_SRC_ALPHA);
	        GLES20.glEnable(GLES20.GL_BLEND);

	        // Update camera view matrix
	        float[] orientation = mSensorFusion.getFusedOrientation();

	        // Convert angles to degrees
	        float rX = (float) (orientation[1] * 180.0f/Math.PI);
	        float rY = (float) (orientation[0] * 180.0f/Math.PI);
	        float rZ = (float) (orientation[2] * 180.0f/Math.PI);

	        // Update quaternion from euler angles out of orientation
	        mCameraQuat.fromEuler( rX, 180.0f-rZ, rY);
	        mCameraQuat = mCameraQuat.getConjugate();
	        mCameraQuat.normalise();
	        mViewMatrix = mCameraQuat.getMatrix();

	        // Update camera billboard
	        mCameraBillboardLeft.mModelMatrix = mCameraQuat.getMatrix();
	        
	        Matrix.invertM(mCameraBillboardLeft.mModelMatrix, 0, mCameraBillboardLeft.mModelMatrix, 0);
	        Matrix.translateM(mCameraBillboardLeft.mModelMatrix, 0, -67.5f, 0.0f, -DISTANCE);
	        //Matrix.rotateM(mCameraBillboard.mModelMatrix, 0, -90, 0, 0, 1);
	        Matrix.rotateM(mCameraBillboardLeft.mModelMatrix, 0, 180, 0, 0, 1);

	        mCameraBillboardLeft.draw();
	        
	        // Update camera billboard
	        mCameraBillboardRight.mModelMatrix = mCameraQuat.getMatrix();
	        
	        Matrix.invertM(mCameraBillboardRight.mModelMatrix, 0, mCameraBillboardRight.mModelMatrix, 0);
	        Matrix.translateM(mCameraBillboardRight.mModelMatrix, 0, 67.5f, 0.0f, -DISTANCE);
	        //Matrix.rotateM(mCameraBillboard.mModelMatrix, 0, -90, 0, 0, 1);
	        Matrix.rotateM(mCameraBillboardRight.mModelMatrix, 0, 180, 0, 0, 1);

	        mCameraBillboardRight.draw();

	}
}