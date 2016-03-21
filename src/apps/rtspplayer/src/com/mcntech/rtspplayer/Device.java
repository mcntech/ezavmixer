package com.mcntech.rtspplayer;

import java.math.BigInteger;
import java.net.InetAddress;
import java.net.UnknownHostException;

public class Device {

	public String label = "";

	public Device(){

	}
	public Device(Device dev){
	
	}

	@Override 
	public String toString(){
		String ret; 
		ret =  label ;
		return ret; 
	}

}
