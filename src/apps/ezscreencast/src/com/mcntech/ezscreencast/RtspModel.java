package com.mcntech.ezscreencast;

public class RtspModel {
	public void setNodeParams(OnyxRemoteNode node, int sessionId) {
		mNode = node;

		mPublishId = "publish" + sessionId;	
		//public static String mServerId = "server0";
		mSwcitchId = "switch" + sessionId;
		
		mInputId = "input" + sessionId;
		mInputType = "inproc";
		mInputUrl = "none";
		
		mMpdId = "mpd" + sessionId;
		mPeriodId = "period" + sessionId;
		mAdaptId = "adapt" + sessionId;
		mRepId = "rep" + sessionId;
	
	}
	public String mPublishId;	
	//public static String mServerId = "server0";
	public String mSwcitchId;
	
	public String mInputId;
	public String mInputType;
	public String mInputUrl;
	
	public String mMpdId;
	public String mPeriodId;
	public String mAdaptId;
	public String mRepId;
	OnyxRemoteNode mNode;
}
