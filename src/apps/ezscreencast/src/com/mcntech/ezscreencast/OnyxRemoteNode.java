package com.mcntech.ezscreencast;

import java.math.BigInteger;
import java.net.InetAddress;
import java.net.UnknownHostException;

import android.text.Editable;

public class OnyxRemoteNode {
	
	public String mNickname;
	public String mHost; 
	public String mAccessid;
	public String mSecuritykey; 
	public String mFileprefix;
	public String mFolder;
	public String mBucket;

    public static final String NODE_NICKNAME = "nickname";
    public static final String NODE_HOST = "host";
    public static final String NODE_ACCESSID = "accessid";
    public static final String NODE_SECURITYKEY = "securitykey";
    public static final String NODE_FILEPREFIX = "fileprefix";
    public static final String NODE_FOLDER = "folder";
    public static final String NODE_BUCKET = "bucket";

	public OnyxRemoteNode(String nickname, String host, 
			String accessid, String securitykey, 
			String fileprefix, String folder, String  bucket){
		mNickname = nickname;
		mHost = host;
		mAccessid = accessid;
		mSecuritykey = securitykey;
		mFileprefix = fileprefix;
		mFolder = folder;
		mBucket = bucket;
	}
	
	public OnyxRemoteNode(){
	}

	public OnyxRemoteNode(String nickname){
		mNickname = nickname;
		mHost = "s3.amazonaws.com";
		mAccessid = "REPLACETHISID";
		mSecuritykey = "REPLACETHISKEY";;
		mFileprefix = "stream";
		mFolder = "live";
		mBucket = "REPLACETHISBUCKET";
	}

	@Override 
	public String toString(){
		String ret; 
		ret = mNickname;	
		return ret; 
	}

}
