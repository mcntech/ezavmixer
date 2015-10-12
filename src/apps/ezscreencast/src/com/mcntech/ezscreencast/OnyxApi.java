package com.mcntech.ezscreencast;

import java.io.IOException;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Vector;
import java.util.concurrent.atomic.AtomicBoolean;

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.media.MediaPlayer;
import android.widget.TabHost;
@SuppressWarnings("unused") //suppress unused variable warnings
public class OnyxApi {	

	public interface RemoteNodeHandler {
		void onConnectRemoteNode(OnyxRemoteNode node,boolean updated);
		void onRemoveRemoteNode(String url);
		void onRemoteNodeError(final String url,final String message);
		void onNetworkDisconnected();
	}


	private static long mHandle = 0;
	private static OnyxApi mSelf = null;
	private static boolean RETRY = true;
	private static int mMediaPosition = -1;

	private static RemoteNodeHandler m_nodeHandler = null;
	final static Object mWaitOnRemoteNode = new Object();
	private static ArrayList<String> mActiveRemoteNodes = new ArrayList<String>();
	static Object mWaitOnStop = new Object();
	
	private OnyxApi() {
	}
	
	public static void initialize(boolean retry, int nDelay) {
		if(mHandle == 0) {
			int nodeIP = 0;
			mHandle = mSelf.init(nodeIP);
		}
	}

	public static void deinitialize() {
		try {
			mSelf.finalize();
		} catch (Throwable e) {
			e.printStackTrace();
		}
	}

	public static void startSession(boolean enableAud, boolean enabeVid) {
		mSelf.startSession(mHandle, enableAud, enabeVid);
	}

	public static void stopSession() {
		mSelf.stopSession(mHandle);
	}
	
	protected void finalize() throws Throwable {
		if(mHandle==0)
			return;
		deinit(mHandle);
	}
	
	@SuppressWarnings("unchecked")
	public static OnyxApi getInstance() {
		return mSelf;
	}
	
	public static boolean stop() {
		if(mHandle==0)
			return false;
		stop(mHandle);
		synchronized (mWaitOnStop) {
			mWaitOnStop.notify();
		}
		return true;
	}
		
	public static void setRemoteNodeHandler(RemoteNodeHandler handler) {
		m_nodeHandler = handler;
	}
	
    public static void addRemoteNode(String url){
		mActiveRemoteNodes.add(url);
	}

	public static void removeRemoteNodeID(String url){
			mActiveRemoteNodes.remove(url);
	}
	
	public static void clearRemoteNodeList(){
		mActiveRemoteNodes.clear();
	}
			

	public static void onNativeMessage(final Object title,final Object message) {		
		System.out.println("java onNativeMessage:" + title + " message:" + message);
	}
	
	public static void onRemoteNodeError(final String url, final Object message ) 
	{		
		if(m_nodeHandler != null)
			m_nodeHandler.onRemoteNodeError(url,(String)message);
	}
		
	public static void onRemoteNodePlayStarted(final long nodeid ) {		
		System.out.println("java onRemoteNodePlayStarted:" + " id:" + nodeid);
		return;
	}
		
	private static void onNetworkDisconnected() {
		System.out.println("java onNetworkDisconnected");
		if(m_nodeHandler != null)
			m_nodeHandler.onNetworkDisconnected();
	}
	

	static {
		System.loadLibrary("OnyxApi");
		mSelf = new OnyxApi();
	}
	
	public static boolean setRemoteNodeSettings(OnyxRemoteNode newSettings){
		if(mHandle==0)
			return false;
		return setRemoteNodeSettings(mHandle,newSettings);
	}
	
	public static String getVersion() {
		if(mHandle==0)
			return null;
		return getVersion(mHandle);
	}
	public static boolean isRemoteNodeActive(String url) {
		if(mHandle==0)
			return false;
		return isRemoteNodeActive(mHandle,url);
	}
	
	public static int sendAudioData(byte[] pcmBytes, int numBytes, long lPts, int nFlags){
		if(mHandle==0)
			return 0;
		return sendAudioData(mHandle,pcmBytes,numBytes, lPts, nFlags);
	}

	public static int sendVideoData(byte[] vidBytes, int numBytes, long lPts, int nFlags){
		if(mHandle==0)
			return 0;
		return sendVideoData(mHandle,vidBytes,numBytes, lPts, nFlags);
	}
	

	public static long getClockUs(){
		if(mHandle==0)
			return 0;
		return getClockUs(mHandle);
	}

	private native long init(int ip);
	private native boolean deinit(long handle);
	private native static void startSession(long handle, boolean enableAud, boolean enableVid);
	private native static void stopSession(long handle);

	private native static boolean addRemoteNode(long handle, String url);	
	private native static boolean removeRemoteNode(long handle, String url);	
	private native static boolean isRemoteNodeActive(long handle, String url);
	private native static boolean setRemoteNodeSettings(long handle, OnyxRemoteNode newSettings);
	
	private native static boolean start(long handle);
	private native static boolean stop(long handle);
	private native static boolean pause(long handle);
	private native static boolean resume(long handle);	
	public native static String getVersion(long handle);
	private native static int sendAudioData(long handle,byte[] pcmBytes, int numBytes, long lPts, int nFlags);
	private native static int sendVideoData(long handle,byte[] vidBytes, int numBytes, long Pts, int Flags);	
	private native static long getClockUs(long handle);	
}
