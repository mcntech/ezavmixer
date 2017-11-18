package com.mcntech.udpplayer;

import android.content.Context;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.PorterDuff;
import android.opengl.GLES20;
import android.os.Trace;
import android.util.AttributeSet;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import com.android.grafika.gles.EglCore;
import com.android.grafika.gles.WindowSurface;
import com.mcntech.rtspplyer.R;;

public class Overlay  {
    private Bitmap bmp;
    private Bitmap mScaledBmp = null;
    private SurfaceHolder mHolder = null;
    private Thread mOverlayThread = null;
    private boolean mRunning = false;
    private static final int UPDATE_STEPS = 30;
    private boolean mAnimateLogo = true;
    
    public void SetLogoAnimation(boolean fAnimate) {
    	mAnimateLogo = fAnimate;
    }
    
    protected void DrawLogo(Canvas canvas, 
    		int nState, int frame, Paint paint) {
    	if(mHolder != null && mScaledBmp != null) {	
    		float angle = 0;
    		Matrix matrix = new Matrix();
    		if(mAnimateLogo) {
    			angle = (float)30.0 / UPDATE_STEPS * frame;
    	    	matrix.postRotate(angle, mScaledBmp.getWidth() / 2, mScaledBmp.getHeight() / 2);
    		}
	        canvas.drawColor(Color.TRANSPARENT, PorterDuff.Mode.CLEAR);
	        canvas.drawBitmap(mScaledBmp, matrix, paint);
    	}
    }
    public Bitmap getResizedBitmap(Bitmap bm, int newHeight, int newWidth)
    {
        int width = bm.getWidth();
        int height = bm.getHeight();
        float scaleWidth = ((float) newWidth) / width;
        float scaleHeight = ((float) newHeight) / height;
        // create a matrix for the manipulation
        Matrix matrix = new Matrix();
        // resize the bit map
        matrix.postScale(scaleWidth, scaleHeight);
        // recreate the new Bitmap
        Bitmap resizedBitmap = Bitmap.createBitmap(bm, 0, 0, width, height, matrix, false);
        return resizedBitmap;
    } 
    public Overlay(Context context, SurfaceView sv) {
    	SurfaceHolder holder = sv.getHolder();
        holder.addCallback(new SurfaceHolder.Callback() {
	         @Override
	         public void surfaceDestroyed(SurfaceHolder holder) {
	
	         }
           @Override
           public void surfaceCreated(SurfaceHolder holder) {
        	   mHolder =  holder;
                new Thread(new Runnable() {
                    public void run() {
                    	startOverlay();//onDraw();
                    }
                }).start();
           }
           @Override
           public void surfaceChanged(SurfaceHolder holder, int format,
                         int width, int height) {
           }
        });
        Resources  res = context.getResources();
        if(res != null)
        	bmp = BitmapFactory.decodeResource(res, R.drawable.logo);
  }
    
    /**
     * Clears the surface, then draws a filled circle with a shadow.
     * <p>
     * Similar to drawCircleSurface(), but the position changes based on the value of "i".
     */
    private void UpdateOverlay(int nState, int frame) {
        Paint paint = new Paint(Paint.ANTI_ALIAS_FLAG);
        paint.setColor(Color.WHITE);
        paint.setStyle(Paint.Style.FILL);

        Canvas canvas = mHolder.getSurface().lockCanvas(null);
        try {
            //canvas.drawColor(Color.TRANSPARENT, PorterDuff.Mode.CLEAR);
 
	        if(mScaledBmp == null) {
	           int width = canvas.getWidth();
	           int height = canvas.getHeight();
	    	   mScaledBmp = getResizedBitmap(bmp, height, width);
	        }
           DrawLogo(canvas, nState, frame, paint);
        } finally {
        	mHolder.getSurface().unlockCanvasAndPost(canvas);
        }
    } 
    
    private void startOverlay() {
	  final Surface surface = mHolder.getSurface();
	  mOverlayThread = new Thread() {
	        @Override
	        public void run() {
	            while (true) {
	                long startWhen = System.nanoTime();
	                for (int i = 0; i < UPDATE_STEPS; i++) {
	                    if (!mRunning) return;
	                    UpdateOverlay(0, i);
	                }
	                for (int i = UPDATE_STEPS; i > 0; i--) {
	                    if (!mRunning) return;
	                    UpdateOverlay(0, i);
	                }
	                long duration = System.nanoTime() - startWhen;
	                double framesPerSec = 1000000000.0 / (duration / (UPDATE_STEPS * 2.0));
	                Log.d("Overlay", "Update at " + framesPerSec + " fps");
	            }
	        }
	    };
	    mRunning = true;
	    mOverlayThread.setName("Overlay");
	    mOverlayThread.start();    
   }    
    /**
     * Signals the bounce-thread to stop, and waits for it to do so.
     */
    private void stopOverlay() {
        Log.d("Overlay", "Stopping Overlay thread");
        mRunning = false;      // tell thread to stop
        if(mOverlayThread != null){
	        try {
	        	mOverlayThread.join();
	        } catch (InterruptedException ignored) {}
        }
        mOverlayThread = null;
    }
    
    public void onPause() {
	   stopOverlay();
    }
}