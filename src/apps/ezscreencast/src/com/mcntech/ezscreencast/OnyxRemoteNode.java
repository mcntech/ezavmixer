package com.mcntech.ezscreencast;

import java.math.BigInteger;
import java.net.InetAddress;
import java.net.UnknownHostException;

public class OnyxRemoteNode {
	public int protocol = 0;
	public int codecc = 0;
	public String url = ""; 
	public OnyxRemoteNode(){

	}
	public OnyxRemoteNode(OnyxRemoteNode dev){
	}

	@Override 
	public String toString(){
		String ret; 
		ret = url;	
		return ret; 
	}

}
