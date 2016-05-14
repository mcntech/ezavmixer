package com.mcntech.sphereview;

import java.util.ArrayList;

public class VrRenderDb {
	public static final int ID_EYE_LEFT = 1;
	public static final int ID_EYE_RIGHT = 2;

    public static class VideoFeed
    {
    	String mUrl;
    	int    mIdEye;
    	// TODO: Add Camera location, direction and projection map
    	
    	VrDecodeToTexture decodePipe = null;
    	int               textureId = 0;
    	public VideoFeed(String url, int feedType){
    		mUrl = url;
    		mIdEye = feedType;
    	}
    }
    /*
     * video feed list
     */
    public static ArrayList<VideoFeed> mVideoFeeds = null;
    
    public static int getFeedCountForEye(int nIdEye){
    	int numFeeds = 0;
    	for(VideoFeed videoFeed : mVideoFeeds){
    		if ((videoFeed.mIdEye & nIdEye) != 0)
    			numFeeds++;
    	}
    	return numFeeds;
    }
}
