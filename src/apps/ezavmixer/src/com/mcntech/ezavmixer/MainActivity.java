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
import com.mcntech.ezavmixer.VideoCompositor.TransitionControl;
import com.mcntech.ezavmixer.VideoFeedList.VideoFeed;
import com.mcntech.ezavmixer.RemoteNode;
import com.mcntech.ezavmixer.R;
import com.mcntech.ezavmixer.OnyxPlayerApi;

public class MainActivity extends Activity implements TransitionControl {

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
	final String TAG = "ezavmixer";
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		Configure.loadSavedPreferences(this, false);
		mCompPlaneTexIds = new int[2];
		mThaumnail10TexId = new int[2];
		mThaumnail11TexId = new int[2];
		mThaumnail12TexId = new int[2];
		mThaumnail13TexId = new int[2];
		
		VideoFeedList.init();
		
		mNodeHandler = new RemoteNodeHandler(){
			@Override
			public void onDiscoverRemoteNode(String url) {

				Log.d(TAG, "MainActivity::onDiscoverRemoteNode");
				RemoteNode node = new RemoteNode(url);
				//mRemoteNodeList.add(node);
				RemoteNode remoteNode = new RemoteNode(url);
				VideoFeedList.addFeed(remoteNode.getRtspStream(RemoteNode.VID_RES_480P),  VideoFeedList.ID_EYE_LEFT | VideoFeedList.ID_EYE_RIGHT);
				runOnUiThread(new Runnable() {
					@Override
                    public void run() {
						//((BaseAdapter)((ListView)findViewById(R.id.listRemoteNodes)).getAdapter()).notifyDataSetChanged();
						// TODO: Update video
					}
				});
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

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// Handle action bar item clicks here. The action bar will
		// automatically handle clicks on the Home/Up button, so long
		// as you specify a parent activity in AndroidManifest.xml.
		int id = item.getItemId();
		if (id == R.id.action_settings) {
			return true;
		}
		return super.onOptionsItemSelected(item);
	}
	
	public void onStartStopMixer(View v)
	{
		// Initialize decode to texture
		for(int i=0; i < VideoFeedList.mVideoFeeds.size(); i++){
			VideoFeed videoFeed = VideoFeedList.mVideoFeeds.get(i);
			videoFeed.decodePipe = new DecodeToTexture(this, videoFeed.mUrl, 1280, 720);
		}
		
		mGLSurfaceView = new GLSurfaceView(this); 
		mGLSurfaceView.setEGLContextClientVersion(2);
		
		mCompositor = new VideoCompositor(this);
		mCompositor.AddVideoPlane(mCompPlaneId, 0.0f, 0f, -65.0f);

		mCompositor.AddVideoPlane(mThumbnail0, -65.0f, -200.0f, -270.0f);
	
		mCompositor.AddVideoPlane(mThumbnail1, -65.0f, 0f, -270.0f);
		mCompositor.AddVideoPlane(mThumbnail2, -65.0f, 200.0f, -270.0f);

		mGLSurfaceView.setRenderer(mCompositor);
		//setContentView(mGLSurfaceView);
		LayoutParams lp = new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT);
		
		LinearLayout layout = (LinearLayout) findViewById(R.id.l22);
		layout.addView(mGLSurfaceView, lp);
		
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
			mCompositor.setTextures(mThumbnail0, mThaumnail10TexId, transitionId, progress);
		
	
		if(mThaumnail11TexId[0] != 0)
			mCompositor.setTextures(mThumbnail1, mThaumnail11TexId, transitionId, progress);
		if(mThaumnail12TexId[0] != 0)
			mCompositor.setTextures(mThumbnail2, mThaumnail12TexId, transitionId, progress);
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
			mThaumnail10TexId[1] = VideoFeedList.mVideoFeeds.get(1).textureId;
		}

		if(VideoFeedList.mVideoFeeds.size() > 1) {
			mThaumnail11TexId[0] = VideoFeedList.mVideoFeeds.get(1).textureId;
		}
		if(VideoFeedList.mVideoFeeds.size() > 2) {
			mThaumnail12TexId[0] = VideoFeedList.mVideoFeeds.get(2).textureId;
		}

	}
}
