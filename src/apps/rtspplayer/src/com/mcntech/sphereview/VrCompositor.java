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

public class VrCompositor  implements GLSurfaceView.Renderer {
	public static final String TAG = "VrCompositor";
	public final String LOG_TAG = "VrCompositor";
	private List<VrDecodeToTexture> mDecodePipes;
        
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

    private int[] mProgram = new int[2];
    private int[] mVertexShader = new int[2];
    private int[] mFragmentShader = new int[2];
    private int[] mPositionHandler = new int[2];
    private int[] mTexCoordHandler = new int[2];
    private int[] mTextureHandler = new int[2];
    private int[] mAlphaHandler = new int[2];
    private int[] mMVPMatrixHandler = new int[2];
    
    
    private int mProgramStitch;
    private int mVertexShaderStitch;
    private int mFragmentShaderStitch;
    private int[] mPositionHandlerStitch= new int[1];
    private int[] mTextureHandlerStitch = new int[2];
    private int[] mTexCoordHandlerStitch = new int[2];
    private int[] mAlphaHandlerStitch = new int[1];
    private int[] mMVPMatrixHandlerStitch = new int[1];
    
	private List<CameraFeed>        mCameraFeeds;
    private CameraStitch            mLeftEye;
    
    //private Snapshot mViewfinderBillboard;
    private Context mContext;
    private float[] mMVPMatrix = new float[16];
    
	public VrCompositor(Activity activity, String[] urls, int maxVidWidth, int maxVidHeight) {

		Context context = activity.getApplicationContext();
		mSensorFusion = new SensorFusion(context);
	    mCameraQuat = new Quaternion();
	    mContext = activity;
	    mDecodePipes = new ArrayList<VrDecodeToTexture>();
		for(int i=0; i < urls.length; i++){
		    VrDecodeToTexture decodePipe = new VrDecodeToTexture(activity, urls[i], maxVidWidth, maxVidHeight);
		    mDecodePipes.add(decodePipe);
		}
		
		//mCameraFeeds =  new ArrayList<CameraFeed>();
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
    

    /**
     * Stores the information about each camera displayed in the sphere
     */
    private class CameraFeed {
		private float[] mModelMatrix;
		private int mTextureId;
		private int mMode;
		private boolean mIsVisible = true;
		private float mAlpha = 1.0f;
		private float mAutoAlphaX;
		private float mAutoAlphaY;
		
		private int   mEye;
		private float mPosX;
		private float mPosY;
		private float mPosZ;
		
        public CameraFeed(int Eye, float PosX, float PosY, float PosZ, int TextureId) {
            mPosX=PosX;
            mPosY=PosY;
            mPosZ=PosZ;
            mEye = Eye;
            mMode = CAMERA;
            mTextureId = TextureId;
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

            GLES20.glUseProgram(mProgram[mMode]);
            mVertexBuffer.position(0);
            mTexCoordBuffer.position(0);

            GLES20.glEnableVertexAttribArray(mTexCoordHandler[mMode]);
            GLES20.glEnableVertexAttribArray(mPositionHandler[mMode]);

            GLES20.glVertexAttribPointer(mPositionHandler[mMode],
                    2, GLES20.GL_FLOAT, false, 8, mVertexBuffer);
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
            GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, mTextureId);

            GLES20.glUniform1i(mTextureHandler[mMode], 0);

            GLES20.glDrawArrays(GLES20.GL_TRIANGLE_FAN, 0, 4);
        }
    }

    /**
     * Stores the information about each camera displayed in the sphere
     */
    private class CameraStitch {
		private float[] mModelMatrix;
		private int mTextureId1;
		private int mTextureId2;
		private int mMode;
		private boolean mIsVisible = true;
		private float mAlpha = 1.0f;
		private float mAutoAlphaX;
		private float mAutoAlphaY;
		
		private int   mEye;
		private float mPosX;
		private float mPosY;
		private float mPosZ;
		
        public CameraStitch(int Eye, float PosX, float PosY, float PosZ, int TextureId1, int TextureId2) {
            mPosX=PosX;
            mPosY=PosY;
            mPosZ=PosZ;
            mEye = Eye;
            mMode = CAMERA;
            mTextureId1 = TextureId1;
            mTextureId2 = TextureId2;
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
            GLES20.glUniformMatrix4fv(mMVPMatrixHandler[mMode], 1, false, mMVPMatrix, 0);

            GLES20.glUniform1f(mAlphaHandler[mMode], mAlpha);

            GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
            GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, mTextureId1);
            GLES20.glUniform1i(mTextureHandlerStitch[0], 0);

            GLES20.glActiveTexture(GLES20.GL_TEXTURE1);
            GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, mTextureId2);
            GLES20.glUniform1i(mTextureHandlerStitch[1], 0);
            
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
                        + "   v_TexCoordinate1 = a_TexCoordinate2;\n"
                        + "   v_TexCoordinate2 = a_TexCoordinate1;\n"                        
                        + "   gl_Position = u_MVPMatrix * a_Position;   \n"
                        + "}                              \n";
        
        final String camStitchFragmentShader = "#extension GL_OES_EGL_image_external : require\n"
                + "precision mediump float;       \n"
                + "uniform samplerExternalOES u_Texture1;   \n"
                + "uniform samplerExternalOES u_Texture2;   \n"                
                + "varying vec2 v_TexCoordinate;  \n"
                + "uniform float f_Alpha;\n"
                + "void main()                    \n"
                + "{                              \n"
                + "   gl_FragColor = texture2D(u_Texture1, v_TexCoordinate) + texture2D(u_Texture2, v_TexCoordinate);\n"
                + "   gl_FragColor.a = gl_FragColor.a * f_Alpha;"
                + "}                              \n";

        mVertexShader[CAMERA] = compileShader(GLES20.GL_VERTEX_SHADER, vertexShader);
        mFragmentShader[CAMERA] = compileShader(GLES20.GL_FRAGMENT_SHADER, camPreviewShader);

        mVertexShader[SNAPSHOT] = compileShader(GLES20.GL_VERTEX_SHADER, vertexShader);
        mFragmentShader[SNAPSHOT] = compileShader(GLES20.GL_FRAGMENT_SHADER, fragmentShader);
        
        mVertexShader[STITCH] = compileShader(GLES20.GL_VERTEX_SHADER, camStitchvertexShader);
        mFragmentShader[STITCH] = compileShader(GLES20.GL_FRAGMENT_SHADER, camStitchFragmentShader);
        
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
        Boolean fEnableStitch = true;
        if(fEnableStitch) {
            mProgramStitch = GLES20.glCreateProgram();
            GLES20.glAttachShader(mProgramStitch, mFragmentShaderStitch);
            GLES20.glAttachShader(mProgramStitch, mVertexShaderStitch);
            GLES20.glLinkProgram(mProgramStitch);

            int[] linkStatus = new int[1];
            GLES20.glGetProgramiv(mProgramStitch, GLES20.GL_LINK_STATUS, linkStatus, 0);

            if (linkStatus[0] == 0) {
                throw new RuntimeException("Error linking shaders");
            }
            mPositionHandlerStitch[0]     = GLES20.glGetAttribLocation(mProgramStitch, "a_Position");
            mTexCoordHandlerStitch[0]     = GLES20.glGetAttribLocation(mProgramStitch, "a_TexCoordinate1");
            mTexCoordHandlerStitch[1]     = GLES20.glGetAttribLocation(mProgramStitch, "a_TexCoordinate2");
            mMVPMatrixHandlerStitch[0]    = GLES20.glGetUniformLocation(mProgramStitch, "u_MVPMatrix");
            mTextureHandlerStitch[0]      = GLES20.glGetUniformLocation(mProgramStitch, "u_Texture1");
            mTextureHandlerStitch[1]      = GLES20.glGetUniformLocation(mProgramStitch, "u_Texture2");
            mAlphaHandlerStitch[0]      = GLES20.glGetUniformLocation(mProgramStitch, "f_Alpha");
        }
        
        int cameraPos = 0;
        for (VrDecodeToTexture decodePipe : mDecodePipes) {
    		initCameraFeed(decodePipe, SNAPSHOT_SCALE + 2 * SNAPSHOT_SCALE * cameraPos, 0, -DISTANCE);
    		cameraPos++;
        }
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

	
	private void initCameraFeed(VrDecodeToTexture decodePipe, float posX,
			float posY, float posZ) {
		int texture[] = new int[1];

		GLES20.glGenTextures(1, texture, 0);
		int TextureId = texture[0];

		if (TextureId == 0) {
			throw new RuntimeException("CAMERA TEXTURE ID == 0");
		}

		GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, TextureId);
		// Can't do mipmapping with camera source
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

		CameraFeed cameraFeedLeft = new CameraFeed(EYE_LEFT, posX, posY, posZ,
				TextureId);
		mCameraFeeds.add(cameraFeedLeft);

		CameraFeed cameraFeedRight = new CameraFeed(EYE_RIGHT, -posX, posY,
				posZ, TextureId);
		mCameraFeeds.add(cameraFeedRight);

		Handler handler = decodePipe.getHandler();
		handler.sendMessage(handler.obtainMessage(
				VrDecodeToTexture.PLAYER_CMD_INIT, CameraSurfaceTex));
	}
	
	
	private void initCameraStitch(List<VrDecodeToTexture> mDecodePipes, float posX, float posY, float posZ) {

		for(VrDecodeToTexture decodePipe : mDecodePipes) {
			int texture[] = new int[1];
	
			GLES20.glGenTextures(1, texture, 0);
			int TextureId = texture[0];
	
			if (TextureId == 0) {
				throw new RuntimeException("CAMERA TEXTURE ID == 0");
			}
	
			GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, TextureId);
			// Can't do mipmapping with camera source
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
	
			CameraFeed cameraFeed = new CameraFeed(EYE_LEFT, posX, posY, posZ,	TextureId);
			mCameraFeeds.add(cameraFeed);
		
			Handler handler = decodePipe.getHandler();
			handler.sendMessage(handler.obtainMessage(
					VrDecodeToTexture.PLAYER_CMD_INIT, CameraSurfaceTex));
		}
	}
	
	@Override
	public void onDrawFrame(GL10 gl) {
		for (VrDecodeToTexture decodePipe : mDecodePipes) {
			SurfaceTexture sufraceTexture = decodePipe.getSurfaceTexture();
			if(sufraceTexture != null)
				sufraceTexture.updateTexImage();
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

		for (CameraFeed cameraFeed : mCameraFeeds) {
			// Update camera billboard
			cameraFeed.mModelMatrix = mCameraQuat.getMatrix();
	
			Matrix.invertM(cameraFeed.mModelMatrix, 0,
					cameraFeed.mModelMatrix, 0);
			Matrix.translateM(cameraFeed.mModelMatrix, 0, cameraFeed.mPosX, cameraFeed.mPosY, cameraFeed.mPosZ);
			// Matrix.rotateM(mCameraBillboard.mModelMatrix, 0, -90, 0, 0, 1);
			Matrix.rotateM(cameraFeed.mModelMatrix, 0, 180, 0, 0, 1);
	
			cameraFeed.draw();
		}
	}
}