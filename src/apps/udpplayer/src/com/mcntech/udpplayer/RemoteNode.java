package com.mcntech.udpplayer;

import java.math.BigInteger;
import java.net.InetAddress;
import java.net.UnknownHostException;

public class RemoteNode {
	final static int MAX_AUD_PIDS = 16;
	String mUrl;
	int mProgram;
	int mVidPID;
	AudioStream mAudPID[] = new AudioStream[MAX_AUD_PIDS];
	int mVidWidth;
	int mVidHeight;

	public String label = "";

	public RemoteNode(String url){
		mUrl = url;
	}
	
	public RemoteNode(RemoteNode dev){
	
	}

	@Override 
	public String toString(){
		return mUrl; 
	}

	public String getRtspStream(){
		String path = "v01";
		String stream = "rtsp://" + mUrl + ":" + mProgram;
		return stream; 
	}
	
	class AudioStream
	{
		int PID;
		int CodecType;
	}
}
