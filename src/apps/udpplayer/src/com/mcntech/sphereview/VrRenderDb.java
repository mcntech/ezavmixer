package com.mcntech.sphereview;

import java.util.ArrayList;

import android.content.Context;

import android.graphics.SurfaceTexture;
import android.os.Handler;

public class VrRenderDb  {
	public static final int ID_EYE_LEFT = 1;
	public static final int ID_EYE_RIGHT = 2;
    public interface DecPipeBase
    {
    	public Handler getHandler();
    	public SurfaceTexture getSurfaceTexture();
    }

    public static class VideoFeed
    {
		public String mUrl;
    	public int    mIdEye;
    	public int    mPosition;
    	// TODO: Add Camera location, direction and projection map
    	
    	//VrDecodeToTexture decodePipe = null;
    	public DecPipeBase decodePipe = null;
    	public int               textureId = 0;
    	public VideoFeed(String url, int feedType){
    		mUrl = url;
    		mIdEye = feedType;
    		mPosition = 0;
    	}    	
    }
    /*
     * video feed list
     */
    public static ArrayList<VideoFeed> mVideoFeeds = null;
    
    public static void init()
    {
	   VrRenderDb.mVideoFeeds =  new ArrayList<VrRenderDb.VideoFeed>();
    }

    public static ArrayList<VrRenderDb.VideoFeed> getVideoFeeds()
    {
    	return VrRenderDb.mVideoFeeds;
    }
    
    public static void addFeed(String url, int eyeId){
    	VideoFeed videoFeed = new VideoFeed(url, eyeId);
    	mVideoFeeds.add(videoFeed);
    }
    
    public static int getFeedCountForEye(int nIdEye){
    	int numFeeds = 0;
    	for(VideoFeed videoFeed : mVideoFeeds){
    		if ((videoFeed.mIdEye & nIdEye) != 0)
    			numFeeds++;
    	}
    	return numFeeds;
    }
    
    public static boolean moveUrl(String url, String posUrl){

    	VideoFeed feedSrc = null;
    	int        pos = -1;
    	for(VideoFeed videoFeed : mVideoFeeds){
    		if ((videoFeed.mUrl.equals(posUrl))) {
    			pos = mVideoFeeds.indexOf(videoFeed);
    		}
    		if ((videoFeed.mUrl.equals(url))) {
    			feedSrc = videoFeed;
    		}
    	}
    	if(feedSrc != null && pos != -1){
    		mVideoFeeds.remove(feedSrc);
    		mVideoFeeds.add(pos, feedSrc);
    		return true;
    	} else {
    		return false;
    	}
    }
}