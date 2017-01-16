package com.mcntech.ezavmixer;

import android.app.ActionBar.LayoutParams;
import android.app.Activity;
import android.graphics.SurfaceTexture;
import android.opengl.GLES11Ext;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.BaseAdapter;
import android.widget.LinearLayout;
import android.widget.ListView;

import com.mcntech.ezavmixer.OnyxPlayerApi.RemoteNodeHandler;
import com.mcntech.ezavmixer.VideoMixerActivity.VideoFeedControl;
import com.mcntech.ezavmixer.VideoFeedList.VideoFeed;
import com.mcntech.ezavmixer.RemoteNode;
import com.mcntech.ezavmixer.R;
import com.mcntech.ezavmixer.OnyxPlayerApi;

public class VideoFeedController  implements VideoFeedControl {

	RemoteNodeHandler mNodeHandler;
	VideoCompositor mCompositor;

	
	int mCompPlaneId = 1;
	int mThumbnail0 = 2;
	int mThumbnail1 = 3;
	int mThumbnail2 = 4;
	int mThaumnail10TexId[];
	int mThaumnail11TexId[];
	int mThaumnail12TexId[];
	int mThaumnail13TexId[];
	
	int mCompPlaneTexIds[];

	private GLSurfaceView         mGLSurfaceView;
	GLSurfaceView mThaumbnailView;
	static final String TAG = "ezavmixer";
	
	Activity mActivity;
	protected void VideoFeedController(Activity activity) {
		Configure.loadSavedPreferences(activity, false);
		mCompPlaneTexIds = new int[2];
		mThaumnail10TexId = new int[2];
		mThaumnail11TexId = new int[2];
		mThaumnail12TexId = new int[2];
		mThaumnail13TexId = new int[2];
		mActivity = activity;
		VideoFeedList.init();
		
		mNodeHandler = new RemoteNodeHandler(){
			@Override
			public void onDiscoverRemoteNode(String url) {

				Log.d(TAG, "MainActivity::onDiscoverRemoteNode");
				RemoteNode node = new RemoteNode(url);
				//mRemoteNodeList.add(node);
				RemoteNode remoteNode = new RemoteNode(url);
				VideoFeedList.addFeed(remoteNode.getRtspStream(RemoteNode.VID_RES_480P),  VideoFeedList.ID_EYE_LEFT | VideoFeedList.ID_EYE_RIGHT);
			}
			@Override
			public void onConnectRemoteNode(String url) {
/*
				Log.d(LOG_TAG, "transition:onStartPlay");
	 			mHandler.sendEmptyMessage(PLAYER_CMD_RUN);
	 			if(Configure.mEnableVideo)
	 				waitForVideoPlay();
*/	 			
			}

			@Override
			public void onDisconnectRemoteNode(String url) {
/*
				Log.d(LOG_TAG, "transition:onStopPlay:Begin");
	 			mHandler.sendEmptyMessage(PLAYER_CMD_STOP);
	 			if(Configure.mEnableVideo)
	 				waitForVideoStop();
	 			Log.d(LOG_TAG, "transition:onStopPlay:End");
*/				
			}

			@Override
			public void onStatusRemoteNode(String url, String message) {
				// TODO Auto-generated method stub
				
			}

			@Override
			public void onRemoteNodeError(String url, String message) {
				// TODO Auto-generated method stub
				
			}

			@Override
			public void onNetworkDisconnected() {
				// TODO Auto-generated method stub
				
			}


	 	}; 
	 	
				
	 	OnyxPlayerApi.setDeviceHandler(mNodeHandler);	
		new Thread(new Runnable() {
			@Override
			public void run() {
				OnyxPlayerApi.initialize();
				OnyxPlayerApi.onvifDiscvrStart(0);
			}
		}).start();
	}

	
	public void onStartStopMixer(View v)
	{
		// Initialize decode to texture
		for(int i=0; i < VideoFeedList.mVideoFeeds.size(); i++){
			VideoFeed videoFeed = VideoFeedList.mVideoFeeds.get(i);
			videoFeed.decodePipe = new DecodeToTexture(mActivity, videoFeed.mUrl, 1280, 720);
		}

		mCompositor = new VideoCompositor(this);
		mCompositor.AddVideoPlane(mCompPlaneId, -1.0f, 0f, 0.0f, 1.0f, 1.0f, 1.0f);
		mCompositor.AddVideoPlane(mThumbnail0, 5.0f, 2.0f, 0.0f, 0.25f, 0.25f, 1.0f);
		mCompositor.AddVideoPlane(mThumbnail1, 5.0f, 0.0f, 0.0f, 0.25f, 0.25f, 1.0f);
		
		mGLSurfaceView.setRenderer(mCompositor);
		LayoutParams lp = new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT);
		
	}

	public void onSwap(View v)
	{
		if(mCompPlaneTexIds != null) {
			int tmp = mCompPlaneTexIds[0];
			mCompPlaneTexIds[0] = mCompPlaneTexIds[1];
			mCompPlaneTexIds[1] = tmp;
		}
	}
	
	@Override
	public void onDrawframe() {
		// TODO Auto-generated method stub

		for (VideoFeed videoFeed : VideoFeedList.mVideoFeeds) {

			if(videoFeed.decodePipe != null) {
				SurfaceTexture sufraceTexture = videoFeed.decodePipe.getSurfaceTexture();
				if(sufraceTexture != null){
					sufraceTexture.updateTexImage();
				}
			}
		}
		
		VideoCompositor.TransitionId transitionId = VideoCompositor.TransitionId.WIPE;
		float progress = 0.0f;
		mCompositor.setTextures(mCompPlaneId, mCompPlaneTexIds, transitionId, progress);
		
		if(mThaumnail10TexId[0] != 0)
			mCompositor.setTextures(mThumbnail0, mThaumnail10TexId,  transitionId, progress);
		if(mThaumnail11TexId[0] != 0)
			mCompositor.setTextures(mThumbnail1, mThaumnail11TexId,  transitionId, progress);
		
	}
	
	@Override
	public void onSurfaceCreated() {
		for(VideoFeed videoFeed : VideoFeedList.mVideoFeeds) {

			videoFeed.textureId = mCompositor.createTexture();
	
			SurfaceTexture surfaceTex = new SurfaceTexture(videoFeed.textureId);
			surfaceTex.setDefaultBufferSize(1280, 720);
		
			Handler handler = videoFeed.decodePipe.getHandler();
			handler.sendMessage(handler.obtainMessage(
					DecodeToTexture.PLAYER_CMD_INIT, surfaceTex));
		}
		if(VideoFeedList.mVideoFeeds.size() > 0) {
			mCompPlaneTexIds[0] = VideoFeedList.mVideoFeeds.get(0).textureId;
		}

		if(VideoFeedList.mVideoFeeds.size() > 1) {
			mCompPlaneTexIds[1] = VideoFeedList.mVideoFeeds.get(1).textureId;
		}
		
		if(VideoFeedList.mVideoFeeds.size() > 0) {
			mThaumnail10TexId[0] = VideoFeedList.mVideoFeeds.get(0).textureId;
		}
		if(VideoFeedList.mVideoFeeds.size() > 1) {
			mThaumnail11TexId[0] = VideoFeedList.mVideoFeeds.get(1).textureId;
		}
	}
}
