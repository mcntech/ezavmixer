package com.mcntech.rtspplayer;


import android.app.Activity;
import android.graphics.SurfaceTexture;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.TextureView;


import com.mcntech.rtspplyer.R;
import com.mcntech.sphereview.CameraManager;

import com.mcntech.sphereview.VrEyeRender;

public class VrPlayerActivity extends Activity implements CameraManager/*, SurfaceTexture.OnFrameAvailableListener*/  {
	
	public final String LOG_TAG = "rtsp";	
	VrEyeRender                   mVrRender = null;
	SurfaceTexture                mVrTexture;
	private GLSurfaceView         mGLSurfaceView;
	
	public class DecodeCtx {
		TextureView               mVideoSv = null;
		String                    mUrl;                		
	    DecodePipe                mDecodePipe;
	    boolean                   mVisible;
	}
    
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		mGLSurfaceView = new GLSurfaceView(this); 
		mGLSurfaceView.setEGLContextClientVersion(2);
		
		VrEyeRender mVrRender = new VrEyeRender(this, 1280, 720);
		mGLSurfaceView.setRenderer(mVrRender);
		setContentView(mGLSurfaceView);
	}
	
    void makeStreamVisible(String url) {
    	// Get Current Visibe
    	
    	// Start Playing url
    	
    	// install callback for notifying first frame
    	
    		// on callback make Current invisible and make url visible, stop current
    }
    
	protected void onDestroy() {
		super.onDestroy();
	}
	
   @Override
    protected void onPause() {

   }
  
   @Override
   public void onBackPressed() {
	   System.exit(2);
  }

@Override
public void setRenderToTexture(SurfaceTexture texture) {
	mVrTexture = texture;
/*	mVrTexture.setOnFrameAvailableListener(this);*/
	
}

/*@Override
public void onFrameAvailable(SurfaceTexture surfaceTexture) {
	mGLSurfaceView.requestRender();
	
}*/
   
}