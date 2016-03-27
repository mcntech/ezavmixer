package com.mcntech.rtspplayer;

import java.math.BigInteger;
import java.net.InetAddress;
import java.net.UnknownHostException;

public class RemoteNode {
	String mUrl;
	final static int VID_RES_4K = 1;
	final static int VID_RES_1080P = 2;
	final static int VID_RES_720P = 3;
	final static int VID_RES_480P = 4;
	final static int VID_RES_240P = 5;

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
	// Testing with Onvif.
	// Needs modification
	public String getRtspStream(int res){
		String path = "v01";
		switch(res){
			case VID_RES_720P:
				path = "v01";
				break;
			case VID_RES_480P:
				path = "v02";
				break;
			case VID_RES_240P:
				path = "v03";
				break;
				default:
					path = "v02";
				
		}
		String stream = "rtsp://" + mUrl + "/" + path;
		return stream; 
	}

}
