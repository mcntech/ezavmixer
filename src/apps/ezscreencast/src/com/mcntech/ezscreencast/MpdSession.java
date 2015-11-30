package com.mcntech.ezscreencast;

public class MpdSession {
	public void setNodeParams(OnyxRemoteNode node) {
		mNode = node;
	}
	public static String mPublishId = "publish0";	
	public static String mServerId = "server0";
	public static String mSwcitchId = "switch0";
	
	public static String mInputId = "input0";
	public static String mInputType = "inproc";
	public static String mInputUrl = "none";
	
	public static String mMpdId = "mpd0";
	public static String mPeriodId = "period0";
	public static String mAdaptId = "adapt0";
	public static String mRepId = "rep0";
	OnyxRemoteNode mNode;
}
