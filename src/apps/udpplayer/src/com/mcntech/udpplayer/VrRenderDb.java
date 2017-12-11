package com.mcntech.udpplayer;

import java.util.ArrayList;

import android.content.Context;

import android.graphics.SurfaceTexture;
import android.os.Handler;

public class VrRenderDb  {
	public static final int ID_EYE_LEFT = 1;
	public static final int ID_EYE_RIGHT = 2;

    public interface DecPipeBase
    {
		final int                        PLAYER_CMD_RUN = 1;
		final int                        PLAYER_CMD_STOP = 2;
		final int                        PLAYER_CMD_INIT = 3;
		final int                        PLAYER_CMD_DEINIT = 4;
		final int                        PLAYER_CMD_REINIT = 5;
		final int                        PLAYER_CMD_CREATE_AUDDECPIPE = 6;
		public Handler getHandler();
    	public SurfaceTexture getSurfaceTexture();
    }

    public static class VideoFeed
    {
		RemoteNode    mRemoteNode;
    	public int    mIdEye;
    	public int    mPosition;
    	// TODO: Add Camera location, direction and projection map
    	
    	//VrDecodeToTexture decodePipe = null;
    	public DecPipeBase decodePipe = null;
    	public int               textureId = 0;
    	public VideoFeed(RemoteNode remoteNode, int feedType){
			mRemoteNode = remoteNode;
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

    public static void addFeed(RemoteNode remoteNode, int eyeId){
    	VideoFeed videoFeed = new VideoFeed(remoteNode, eyeId);
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
    		if ((videoFeed.mRemoteNode.mUrl.equals(posUrl))) {
    			pos = mVideoFeeds.indexOf(videoFeed);
    		}
    		if ((videoFeed.mRemoteNode.equals(url))) {
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
