package com.mcntech.ezavmixer;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.util.ArrayList;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.opengl.GLES11Ext;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.opengl.Matrix;
import android.util.Log;

public class VideoCompositor  implements GLSurfaceView.Renderer {
	public static final String TAG = "VideoCompositor";        

        
    public enum TransitionId {
    	WIPE,
    	MIX,
    	PIP
    }
    
    public interface TransitionControl {
    	void onSurfaceCreated();
    	void onDrawframe();
    }
    
    private Quaternion mCameraQuat;
    private float[] mViewMatrix = new float[16];
    private float[] mProjectionMatrix = new float[16];
    private float[] mMVPMatrix = new float[16];

    private int mProgramStitch;
    private int mVertexShaderStitch;
    private int mFragmentShaderStitch;
    private int[] mPositionHandlerStitch= new int[1];
    private int[] mTextureHandlerStitch = new int[2];
    private int[] mTexCoordHandlerStitch = new int[2];
    private int[] mAlphaHandlerStitch = new int[1];
    private int[] mMVPMatrixHandlerStitch = new int[1];
    
    private int   mHandleStitchX;
    
    private ArrayList<VideoPlane> mListPlanes = null;

    

    
    TransitionControl mTransitionController;
	public VideoCompositor(TransitionControl transitionController) {

		mTransitionController = transitionController;
		mListPlanes = new ArrayList<VideoPlane>();
	    mCameraQuat = new Quaternion();
	}

	public void setTextures(int planeId, int []texId, TransitionId transitionId, float progress) {

		for(VideoPlane plane : mListPlanes) {
			if(plane.mId== planeId) {
				switch(transitionId) {
					case WIPE:
						plane.mStitchX = progress;
						break;
					default:
						break;
				}
				plane.setActiveTextures(texId);
			}
		}
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
    private class VideoPlane {
		int     mId;
    	private float[] mModelMatrix;
		
		private boolean mIsVisible = true;
		private float mAlpha = 1.0f;
		private float mAutoAlphaX;
		private float mAutoAlphaY;
		
		private float mPosX;
		private float mPosY;
		private float mPosZ;

		private float mScaleX;
		private float mScaleY;
		private float mScaleZ;

		private int   mActiveTextures[];
		
	    private FloatBuffer mVertexBuffer;
	    private FloatBuffer mTexCoordBuffer;
	    
		private float mStitchX = 1.0f;
		private float mIncr = -0.01f;
		

	    private final float mVertexData[] =
	        {
				-1.6f,  -1.0f,
				-1.6f, 1.0f,
				1.6f, 1.0f,
				1.6f, -1.0f
	        };

	    // u,v
	    private final float mTexCoordData[] =
        {
            0.0f, 0.0f,
            0.0f, 1.0f,
            1.0f, 1.0f,
            1.0f, 0.0f
        };

        public VideoPlane(int id, float PosX, float PosY, float PosZ, float scaleX, float scaleY, float scaleZ) {
        	mId = id;
            mPosX=PosX;
            mPosY=PosY;
            mPosZ=PosZ;
            mScaleX = scaleX;
            mScaleY = scaleY;
            mScaleZ = scaleZ;
            mActiveTextures = new int[2];
            
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
        }

        public void setActiveTextures(int textures[])
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
            
            Matrix.multiplyMM(mMVPMatrix, 0, mViewMatrix, 0, mModelMatrix, 0);
            Matrix.multiplyMM(mMVPMatrix, 0, mProjectionMatrix, 0, mMVPMatrix, 0);

            //Matrix.multiplyMM(mMVPMatrix, 0, mProjectionMatrix, 0, mViewMatrix, 0);

            // Pass in the combined matrix.
            GLES20.glUniformMatrix4fv(mMVPMatrixHandlerStitch[0], 1, false, mMVPMatrix, 0);

            GLES20.glUniform1f(mAlphaHandlerStitch[0], mAlpha);
            GLES20.glUniform1f(mHandleStitchX, mStitchX);
    
            GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
            GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, mActiveTextures[0]);
            GLES20.glUniform1i(mTextureHandlerStitch[0], 0);

            GLES20.glActiveTexture(GLES20.GL_TEXTURE4);
            GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, mActiveTextures[1]);
            GLES20.glUniform1i(mTextureHandlerStitch[1], 4);
            
            GLES20.glDrawArrays(GLES20.GL_TRIANGLE_FAN, 0, 4);
        }
    }
 	@Override
	public void onSurfaceCreated(GL10 gl, EGLConfig config) {

        GLES20.glClearColor(0.5f, 0.5f, 0.5f, 0.5f);
        
        // Simple GLSL vertex/fragment, as GLES2 doesn't have the classical fixed pipeline
        final String camStitchvertexShader1 =
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
        
        final String camStitchFragmentShader1 = "#extension GL_OES_EGL_image_external : require\n"
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
                + "   if(v_TexCoordinate1.x < (1.0 - f_StitchX)) { texcoord = vec2( v_TexCoordinate1.x + f_StitchX, v_TexCoordinate1.y);gl_FragColor = texture2D(u_Texture1, texcoord);} else {texcoord = vec2(v_TexCoordinate2.x - (1.0 - f_StitchX), v_TexCoordinate2.y); gl_FragColor = texture2D(u_Texture2, texcoord);}\n"
                + "   gl_FragColor.a = gl_FragColor.a * f_Alpha;"
                + "}                              \n";

        // Simple GLSL vertex/fragment, as GLES2 doesn't have the classical fixed pipeline
        final String camStitchvertexShader2 =
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
                        + "   gl_Position = a_Position;   \n"
                        + "}                              \n";
        
        final String camStitchFragmentShader2 = "#extension GL_OES_EGL_image_external : require\n"
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
                + "   gl_FragColor = texture2D(u_Texture1, texcoord);\n"
                + "   gl_FragColor.a = gl_FragColor.a * f_Alpha;"
                + "}                              \n";
        
	        mVertexShaderStitch = compileShader(GLES20.GL_VERTEX_SHADER, camStitchvertexShader1);
	        mFragmentShaderStitch = compileShader(GLES20.GL_FRAGMENT_SHADER, camStitchFragmentShader1);
	   

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

    		mTransitionController.onSurfaceCreated();

 	}

	@Override
	public void onSurfaceChanged(GL10 gl, int width, int height) {
        // Set the OpenGL viewport to the same size as the surface.
        GLES20.glViewport(0, 0, width, height);

        // We use here a field of view of 40, which is mostly fine for a camera app representation
        //final float hfov = 90f;

        final float ratio = (float)width/height;//1280.0f / 720.0f;
        final float near = 3;//0.1f;
        final float far = 20;//1500.0f;
        final float left = ratio; //(float) Math.tan(hfov * Math.PI / 360.0f) * near;
        final float right = -left;
        final float bottom = -1;//ratio * right / 1.0f;
        final float top = 1;//ratio * left / 1.0f;

        Matrix.frustumM(mProjectionMatrix, 0, left, right, bottom, top, near, far);
	}
	
	public int createTexture() {
		int texture[] = new int[1];
		
		GLES20.glGenTextures(1, texture, 0);
		int textureId = texture[0];
		
		if (textureId == 0) {
			throw new RuntimeException("CAMERA TEXTURE ID == 0");
		}

		
		GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, textureId);

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

		return textureId;
	}
	
	public void AddVideoPlane(int id, float posX, float posY, float posZ, float scaleX, float scaleY, float scaleZ)
	{
	    VideoPlane videoPlane = new VideoPlane(id, posX, posY, posZ, scaleX, scaleY, scaleZ);
	    mListPlanes.add(videoPlane);
	}
	
	@Override
	public void onDrawFrame(GL10 gl) {
		
		mTransitionController.onDrawframe();
		
		GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT | GLES20.GL_DEPTH_BUFFER_BIT);

		GLES20.glBlendFunc(GLES20.GL_SRC_ALPHA, GLES20.GL_ONE_MINUS_SRC_ALPHA);
		GLES20.glEnable(GLES20.GL_BLEND);

/*		// Update camera view matrix
		float[] orientation =  mSensorFusion.getFusedOrientation();

		// Convert angles to degrees
		float rX = (float) (orientation[1] * 180.0f / Math.PI);
		float rY = (float) (orientation[0] * 180.0f / Math.PI);
		float rZ = (float) (orientation[2] * 180.0f / Math.PI);
*/
		float rX = 0; float rY = 0; float rZ = 0;
		
		// Update quaternion from euler angles out of orientation
		/*mCameraQuat.fromEuler(rX, 180.0f - rZ, rY);
		mCameraQuat = mCameraQuat.getConjugate();
		mCameraQuat.normalise();
		mViewMatrix = mCameraQuat.getMatrix();
		*/
		Matrix.setLookAtM(mViewMatrix, 0, 0, 0, -3, 0f, 0f, 0f, 0f, 1.0f, 0.0f);

		
		for (VideoPlane plane : mListPlanes) {
			if(plane.mActiveTextures[0] != 0) {
				plane.mModelMatrix = new float[16];
				Matrix.setIdentityM(plane.mModelMatrix, 0);//mCameraQuat.getMatrix();
				//Matrix.invertM(plane.mModelMatrix, 0, plane.mModelMatrix, 0);
				Matrix.scaleM(plane.mModelMatrix, 0, plane.mScaleX, plane.mScaleY, plane.mScaleZ);
				Matrix.translateM(plane.mModelMatrix, 0, plane.mPosX, plane.mPosY, plane.mPosZ);
				Matrix.rotateM(plane.mModelMatrix, 0, 180, 0, 0, 1);
				plane.draw();
			}
		}
	}
}