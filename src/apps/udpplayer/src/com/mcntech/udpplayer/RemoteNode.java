package com.mcntech.udpplayer;

import java.math.BigInteger;
import java.net.InetAddress;
import java.net.UnknownHostException;

public class RemoteNode {
	String mUrl;
	int mProgram;

	public String label = "";

	public RemoteNode(String url, String codec, int pgm){
		mUrl = url;
		mProgram =  pgm;
	}

	@Override 
	public String toString(){
		return mUrl+":" + Integer.toString(mProgram);
	}

}
