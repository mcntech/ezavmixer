package com.mcntech.sphereview;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.util.ArrayList;
import java.util.List;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;


import android.app.Activity;
import android.content.Context;
import android.graphics.SurfaceTexture;

import android.opengl.GLES11Ext;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.opengl.Matrix;
import android.os.Handler;
import android.util.Log;

import com.mcntech.rtspplyer.R;
import com.mcntech.sphereview.VrRenderDb.VideoFeed;

public class VrEyeRender  implements GLSurfaceView.Renderer {
	public static final String TAG = "VrEyeRender";        
    private FloatBuffer mVertexBuffer;
    private FloatBuffer mTexCoordBuffer;
    
    private final static float SNAPSHOT_SCALE = 65.5f;
    private final static float DISTANCE = 135.0f;
    
    // x, y,
    private final float mVertexData[] =
            {
				-SNAPSHOT_SCALE,  -SNAPSHOT_SCALE * 1.5f,
				-SNAPSHOT_SCALE, SNAPSHOT_SCALE  * 1.5f,
				SNAPSHOT_SCALE, SNAPSHOT_SCALE  * 1.5f,
				SNAPSHOT_SCALE, -SNAPSHOT_SCALE  * 1.5f
            };


    // u,v
    private final float mTexCoordData[] =
            {
                0.0f, 0.0f,
                0.0f, 1.0f,
                1.0f, 1.0f,
                1.0f, 0.0f
            };

    
    private Quaternion mCameraQuat;
    private float[] mViewMatrix = new float[16];
    private float[] mProjectionMatrix = new float[16];
    private SensorFusion mSensorFusion;
    
    private final static int CAMERA = 0;
    private final static int SNAPSHOT = 1;
    private final static int STITCH = 2;
    
    private final static int EYE_LEFT = 0;
    private final static int EYE_RIGHT = 1;

    private int mProgramStitch;
    private int mVertexShaderStitch;
    private int mFragmentShaderStitch;
    private int[] mPositionHandlerStitch= new int[1];
    private int[] mTextureHandlerStitch = new int[2];
    private int[] mTexCoordHandlerStitch = new int[2];
    private int[] mAlphaHandlerStitch = new int[1];
    private int[] mMVPMatrixHandlerStitch = new int[1];
    
    private int   mHandleStitchX;
    
    private CameraStitch            mLeftEye = null;
    private CameraStitch            mRightEye = null;
    
	private float mStitchX = 1.0f;
	private float mIncr = -0.01f;
	
    //private Snapshot mViewfinderBillboard;
    private Context mContext;
    private float[] mMVPMatrix = new float[16];
    
    private int[] mTextureIdListLeft = null;
    private int[] mTextureIdListRight = null;
    
	public VrEyeRender(Activity activity,  int maxVidWidth, int maxVidHeight) {

		Context context = activity.getApplicationContext();
		mSensorFusion = new SensorFusion(context);
	    mCameraQuat = new Quaternion();
	    mContext = activity;

		for(int i=0; i < VrRenderDb.mVideoFeeds.size(); i++){
			VideoFeed videoFeed = VrRenderDb.mVideoFeeds.get(i);
			videoFeed.decodePipe = new VrDecodeToTexture(activity, videoFeed.mUrl, maxVidWidth, maxVidHeight);
		}
	}

	private float[] matrixFromEuler(float rx, float ry, float rz, float tx,
			float ty, float tz) {
		Quaternion quat = new Quaternion();
		quat.fromEuler(rx, ry, rz);
		float[] matrix = quat.getMatrix();

		Matrix.translateM(matrix, 0, tx, ty, tz);

		return matrix;
	}

	
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

    public int [][] getActiveTexturesForEye(int [] textures,  int startTextureId, int nEyeId)
    {
		int id1 = startTextureId % textures.length;
    	int id2 = (textures.length + startTextureId - 1) % textures.length;
    	
		int   textureIdStitch1;
		int   textureIdStitch2;

		textureIdStitch1 = textures[id1];
		textureIdStitch2 = textures[id2];

		int [][]activeTextures = new int[3][2];
		activeTextures[1][0] = textureIdStitch1;
		activeTextures[1][1] = textureIdStitch2;
		
		return activeTextures;
	}

    public void updateTexturesForHeadViewLocation()
    {
    	int startTextureId = 0;
    	mStitchX = 0;
    	if(mTextureIdListLeft.length > 1) {
	        float[] orientation = mSensorFusion.getFusedOrientation();
			float rY = 180.0f - (float) (orientation[0] * 180.0f / Math.PI);
			float fov = 360 / mTextureIdListLeft.length;
			float offset = rY / fov;
			startTextureId = (int) (offset);
			mStitchX = offset - startTextureId;
    	}
    	
    	int [][]textures = getActiveTexturesForEye(mTextureIdListLeft, startTextureId, VrRenderDb.ID_EYE_LEFT);
    	mLeftEye.setActiveTextures(textures);
    	textures = getActiveTexturesForEye(mTextureIdListRight, startTextureId, VrRenderDb.ID_EYE_RIGHT);
    	mRightEye.setActiveTextures(textures);
        //Log.d(TAG, "HeadView: start" + mStartTextureId + " id1=" + id1 + " id2=" + id2 + " stitch=" + mStitchX);
    }
   

    /**
     * Stores the information about each camera displayed in the sphere
     */
    private class CameraStitch {
		private float[] mModelMatrix;
		
		private int mMode;
		private boolean mIsVisible = true;
		private float mAlpha = 1.0f;
		private float mAutoAlphaX;
		private float mAutoAlphaY;
		
		private int   mEye;
		private float mPosX;
		private float mPosY;
		private float mPosZ;
		private int   mActiveTextures[][];
		public static  final int ROW_TOP = 0;
		public static  final int ROW_MIDDLE = 1;
		public static  final int ROW_BOTTOM = 2;
		
        public CameraStitch(int Eye, float PosX, float PosY, float PosZ) {
            mPosX=PosX;
            mPosY=PosY;
            mPosZ=PosZ;
            mEye = Eye;
            mMode = CAMERA;
            mActiveTextures = new int[3][2];
        }

        public void setActiveTextures(int textures[][])
        {
        	mActiveTextures = textures;
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

        public void draw() {
            if (!mIsVisible) return;

            GLES20.glUseProgram(mProgramStitch);
            mVertexBuffer.position(0);
            mTexCoordBuffer.position(0);

            GLES20.glEnableVertexAttribArray(mTexCoordHandlerStitch[0]);
            GLES20.glEnableVertexAttribArray(mTexCoordHandlerStitch[1]);
            
            GLES20.glEnableVertexAttribArray(mPositionHandlerStitch[0]);

            GLES20.glVertexAttribPointer(mPositionHandlerStitch[0],
                    2, GLES20.GL_FLOAT, false, 8, mVertexBuffer);
            GLES20.glVertexAttribPointer(mTexCoordHandlerStitch[0], 2,
                    GLES20.GL_FLOAT, false, 8, mTexCoordBuffer);
            GLES20.glVertexAttribPointer(mTexCoordHandlerStitch[1], 2,
                    GLES20.GL_FLOAT, false, 8, mTexCoordBuffer);
            
            // This multiplies the view matrix by the model matrix, and stores the
            // result in the MVP matrix (which currently contains model * view).
            Matrix.multiplyMM(mMVPMatrix, 0, mViewMatrix, 0, mModelMatrix, 0);

            // This multiplies the modelview matrix by the projection matrix, and stores
            // the result in the MVP matrix (which now contains model * view * projection).
            Matrix.multiplyMM(mMVPMatrix, 0, mProjectionMatrix, 0, mMVPMatrix, 0);

            // Pass in the combined matrix.
            GLES20.glUniformMatrix4fv(mMVPMatrixHandlerStitch[0], 1, false, mMVPMatrix, 0);

            GLES20.glUniform1f(mAlphaHandlerStitch[0], mAlpha);
            GLES20.glUniform1f(mHandleStitchX, mStitchX);
    
            GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
            GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, mActiveTextures[ROW_MIDDLE][0]);
            GLES20.glUniform1i(mTextureHandlerStitch[0], 0);

            GLES20.glActiveTexture(GLES20.GL_TEXTURE4);
            GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, mActiveTextures[ROW_MIDDLE][1]);
            GLES20.glUniform1i(mTextureHandlerStitch[1], 4);
            
            GLES20.glDrawArrays(GLES20.GL_TRIANGLE_FAN, 0, 4);
        }
    }
 	@Override
	public void onSurfaceCreated(GL10 gl, EGLConfig config) {

        GLES20.glClearColor(0.5f, 0.5f, 0.5f, 0.5f);
        
        // Initialize plane vertex data and texcoords data
        ByteBuffer bb_data = ByteBuffer.allocateDirect(mVertexData.length * 4);
        bb_data.order(ByteOrder.nativeOrder());
        ByteBuffer bb_texture = ByteBuffer.allocateDirect(mTexCoordData.length * 4);
        bb_texture.order(ByteOrder.nativeOrder());
        
        mVertexBuffer = bb_data.asFloatBuffer();
        mVertexBuffer.put(mVertexData);
        mVertexBuffer.position(0);
        mTexCoordBuffer = bb_texture.asFloatBuffer();
        mTexCoordBuffer.put(mTexCoordData);
        mTexCoordBuffer.position(0);
        
        // Simple GLSL vertex/fragment, as GLES2 doesn't have the classical fixed pipeline
        final String camStitchvertexShader =
                "uniform mat4 u_MVPMatrix; \n"
                        + "attribute vec4 a_Position;     \n"
                        + "attribute vec2 a_TexCoordinate1;\n"
                        + "attribute vec2 a_TexCoordinate2;\n"                        
                        + "varying vec2 v_TexCoordinate1;  \n"
                        + "varying vec2 v_TexCoordinate2;  \n"                        
                        + "void main()                    \n"
                        + "{                              \n"
                        + "   v_TexCoordinate1 = a_TexCoordinate1;\n"
                        + "   v_TexCoordinate2 = a_TexCoordinate2;\n"                        
                        + "   gl_Position = u_MVPMatrix * a_Position;   \n"
                        + "}                              \n";
        
        final String camStitchFragmentShader = "#extension GL_OES_EGL_image_external : require\n"
                + "precision mediump float;       \n"
                + "uniform samplerExternalOES u_Texture1;   \n"
                + "uniform samplerExternalOES u_Texture2;   \n"                
                + "varying vec2 v_TexCoordinate1;  \n"
                + "varying vec2 v_TexCoordinate2;  \n"                
                + "uniform float f_Alpha;\n"
                + "uniform float f_StitchX;\n"
                + "vec2 texcoord;\n"
                + "void main()                    \n"
                + "{                              \n"
                + "   if(v_TexCoordinate1.x < f_StitchX) { texcoord = vec2(1.0 - f_StitchX + v_TexCoordinate1.x , v_TexCoordinate1.y);gl_FragColor = texture2D(u_Texture1, texcoord);} else {texcoord = vec2(v_TexCoordinate2.x - f_StitchX, v_TexCoordinate2.y); gl_FragColor = texture2D(u_Texture2, texcoord);}\n"
                + "   gl_FragColor.a = gl_FragColor.a * f_Alpha;"
                + "}                              \n";

	        mVertexShaderStitch = compileShader(GLES20.GL_VERTEX_SHADER, camStitchvertexShader);
	        mFragmentShaderStitch = compileShader(GLES20.GL_FRAGMENT_SHADER, camStitchFragmentShader);
	   

            mProgramStitch = GLES20.glCreateProgram();
            GLES20.glAttachShader(mProgramStitch, mVertexShaderStitch);
            GLES20.glAttachShader(mProgramStitch, mFragmentShaderStitch);
            GLES20.glLinkProgram(mProgramStitch);

            int[] linkStatus = new int[1];
            GLES20.glGetProgramiv(mProgramStitch, GLES20.GL_LINK_STATUS, linkStatus, 0);

            if (linkStatus[0] == 0) {
                throw new RuntimeException("Error linking shaders");
            }
            mPositionHandlerStitch[0]     = GLES20.glGetAttribLocation(mProgramStitch, "a_Position");
            
            // TODO: Change to a loop
            mTexCoordHandlerStitch[0]     = GLES20.glGetAttribLocation(mProgramStitch, "a_TexCoordinate1");
            mTexCoordHandlerStitch[1]     = GLES20.glGetAttribLocation(mProgramStitch, "a_TexCoordinate2");
            mTextureHandlerStitch[0]      = GLES20.glGetUniformLocation(mProgramStitch, "u_Texture1");
            mTextureHandlerStitch[1]      = GLES20.glGetUniformLocation(mProgramStitch, "u_Texture2");
            
            mMVPMatrixHandlerStitch[0]    = GLES20.glGetUniformLocation(mProgramStitch, "u_MVPMatrix");
    
            mAlphaHandlerStitch[0]      = GLES20.glGetUniformLocation(mProgramStitch, "f_Alpha");
            mHandleStitchX      = GLES20.glGetUniformLocation(mProgramStitch, "f_StitchX");

            initCameraStitch(VrRenderDb.mVideoFeeds);

            mLeftEye = new CameraStitch(EYE_LEFT, SNAPSHOT_SCALE, 0, -DISTANCE);
            mRightEye = new CameraStitch(EYE_RIGHT,-SNAPSHOT_SCALE, 0, -DISTANCE);

            mTextureIdListLeft = initTexturesForEye(VrRenderDb.mVideoFeeds, VrRenderDb.ID_EYE_LEFT);
            mTextureIdListRight = initTexturesForEye(VrRenderDb.mVideoFeeds, VrRenderDb.ID_EYE_RIGHT);

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

	private int[] initTexturesForEye(List<VideoFeed> videoFeeds,  int eyeId)
	{
		int []textureList = null;
        int numEyeFeeds = VrRenderDb.getFeedCountForEye(eyeId);
        if(numEyeFeeds > 0) {
	        textureList = new int[numEyeFeeds];
	
			int idx = 0;
			for(VideoFeed videoFeed : videoFeeds) {
				int texture = videoFeed.textureId;
				if((videoFeed.mIdEye & eyeId) != 0) {
					textureList[idx] = texture;
					idx++;
				}
			}
        }
		return textureList;
	}
	
	private void initCameraStitch(List<VideoFeed> videoFeeds) {

		int i = 0;
		for(VideoFeed videoFeed : videoFeeds) {
			int texture[] = new int[1];
	
			GLES20.glGenTextures(1, texture, 0);
			int TextureId = texture[0];
			
			if (TextureId == 0) {
				throw new RuntimeException("CAMERA TEXTURE ID == 0");
			}
			videoFeed.textureId = TextureId;

			
			GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, TextureId);

			GLES20.glTexParameterf(GLES11Ext.GL_TEXTURE_EXTERNAL_OES,
					GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
			GLES20.glTexParameterf(GLES11Ext.GL_TEXTURE_EXTERNAL_OES,
					GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
			// Clamp to edge is the only option
			GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES,
					GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_CLAMP_TO_EDGE);
			GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES,
					GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_CLAMP_TO_EDGE);
	
			GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, 0);
	
			SurfaceTexture CameraSurfaceTex = new SurfaceTexture(TextureId);
			// mCameraSurfaceTex.setDefaultBufferSize(640, 480);
			CameraSurfaceTex.setDefaultBufferSize(1280, 720);
		
			Handler handler = videoFeed.decodePipe.getHandler();
			handler.sendMessage(handler.obtainMessage(
					VrDecodeToTexture.PLAYER_CMD_INIT, CameraSurfaceTex));
			i++;
		}
	}
	
	@Override
	public void onDrawFrame(GL10 gl) {
		for (VideoFeed videoFeed : VrRenderDb.mVideoFeeds) {

			if(videoFeed.decodePipe != null) {
				SurfaceTexture sufraceTexture = videoFeed.decodePipe.getSurfaceTexture();
				if(sufraceTexture != null){
					sufraceTexture.updateTexImage();
				}
			}
		}
		
		GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT | GLES20.GL_DEPTH_BUFFER_BIT);

		GLES20.glBlendFunc(GLES20.GL_SRC_ALPHA, GLES20.GL_ONE_MINUS_SRC_ALPHA);
		GLES20.glEnable(GLES20.GL_BLEND);

		// Update camera view matrix
		float[] orientation = mSensorFusion.getFusedOrientation();

		// Convert angles to degrees
		float rX = (float) (orientation[1] * 180.0f / Math.PI);
		float rY = (float) (orientation[0] * 180.0f / Math.PI);
		float rZ = (float) (orientation[2] * 180.0f / Math.PI);

		// Update quaternion from euler angles out of orientation
		mCameraQuat.fromEuler(rX, 180.0f - rZ, rY);
		mCameraQuat = mCameraQuat.getConjugate();
		mCameraQuat.normalise();
		mViewMatrix = mCameraQuat.getMatrix();

		updateTexturesForHeadViewLocation();
        
		if(mLeftEye != null) {
			mLeftEye.mModelMatrix = mCameraQuat.getMatrix();
			Matrix.invertM(mLeftEye.mModelMatrix, 0,
					mLeftEye.mModelMatrix, 0);
			Matrix.translateM(mLeftEye.mModelMatrix, 0, mLeftEye.mPosX, mLeftEye.mPosY, mLeftEye.mPosZ);
			// Matrix.rotateM(mCameraBillboard.mModelMatrix, 0, -90, 0, 0, 1);
			Matrix.rotateM(mLeftEye.mModelMatrix, 0, 180, 0, 0, 1);
			mLeftEye.draw();
		}

		if(mRightEye != null) {
			mRightEye.mModelMatrix = mCameraQuat.getMatrix();
			Matrix.invertM(mRightEye.mModelMatrix, 0,
					mRightEye.mModelMatrix, 0);
			Matrix.translateM(mRightEye.mModelMatrix, 0, mRightEye.mPosX, mRightEye.mPosY, mRightEye.mPosZ);
			// Matrix.rotateM(mCameraBillboard.mModelMatrix, 0, -90, 0, 0, 1);
			Matrix.rotateM(mRightEye.mModelMatrix, 0, 180, 0, 0, 1);
			mRightEye.draw();
		}
	}
}