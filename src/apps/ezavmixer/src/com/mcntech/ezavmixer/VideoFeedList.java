package com.mcntech.ezavmixer;



/*
 * Maintains a list of video feeds VideoFeed
 * VideoFeed consists of url, texture id and additional parameters
 * 
 */
import java.util.ArrayList;

import android.graphics.SurfaceTexture;
import android.os.Handler;

public class VideoFeedList  {
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
	   VideoFeedList.mVideoFeeds =  new ArrayList<VideoFeedList.VideoFeed>();
    }

    public static ArrayList<VideoFeedList.VideoFeed> getVideoFeeds()
    {
    	return VideoFeedList.mVideoFeeds;
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
