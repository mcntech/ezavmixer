package com.mcntech.rtspplayer;

import java.io.IOException;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Vector;
import java.util.concurrent.atomic.AtomicBoolean;

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.media.MediaPlayer;
import android.util.Log;
import android.widget.TabHost;
@SuppressWarnings("unused") //suppress unused variable warnings
public class OnyxPlayerApi {	
	
	public interface RemoteNodeHandler {
		void onDiscoverRemoteNode(String url);
		void onConnectRemoteNode(String url);
		void onDisconnectRemoteNode(String url);
		void onStatusRemoteNode(String url, final String message);		
		void onRemoteNodeError(final String url,final String message);
		void onNetworkDisconnected();		
	}
	
	private static long mHandle = 0;
	private static OnyxPlayerApi mSelf = null;
	private static RemoteNodeHandler m_nodeHandler = null;
	private static ArrayList<String> mActiveRemoteNodes = new ArrayList<String>();
	
	private OnyxPlayerApi() {

	}
	public static boolean initialize() {
		mSelf = new OnyxPlayerApi();
		if(mHandle == 0) {
			System.out.println("rtsp player");
			mHandle = mSelf.init();
		}
		if(mHandle != 0)
			return true;
		else
			return false;
	}

	public static void deinitialize() {
		try {
			mSelf.finalize();
		} catch (Throwable e) {
			e.printStackTrace();
		}
	}
	
	protected void finalize() throws Throwable {
		if(mHandle==0)
			return;
		deinit(mHandle);
		mHandle = 0;
	}
	
		
	public static OnyxPlayerApi getInstance() {
		return mSelf;
	}
	
	public static void setDeviceHandler(RemoteNodeHandler handler) {
		m_nodeHandler = handler;
	}
	
	public static void onNativeMessage(final Object title,final Object message) {		
		System.out.println("java onNativeMessage:" + title + " message:" + message);
	}

	public static void onDiscoverRemoteNode(final String url ) 
	{		
		if(m_nodeHandler != null) {
			Log.d("OnyxPlayerApi", "onDiscoverRemoteNode:" + url);
			m_nodeHandler.onDiscoverRemoteNode(url);
			if(!mActiveRemoteNodes.contains(url)) {
				mActiveRemoteNodes.add(url);
			}
		}
	}

	public static void onConnectRemoteNode(final String url ) 
	{		
		if(m_nodeHandler != null)
			m_nodeHandler.onConnectRemoteNode(url);
	}

	public static void onDisconnectRemoteNode(final String url ) 
	{		
		if(m_nodeHandler != null)
			m_nodeHandler.onDisconnectRemoteNode(url);
	}
	public static void onStatusRemoteNode(final String url, String Msg) 
	{		
		if(m_nodeHandler != null)
			m_nodeHandler.onStatusRemoteNode(url, Msg);
	}
		
	public static void onRemoteNodeError(final String url, final Object message ) 
	{		
		if(m_nodeHandler != null)
			m_nodeHandler.onRemoteNodeError(url,(String)message);
	}
	public static void onStatusRemoteNode(final String url, final Object message ) 
	{		
		if(m_nodeHandler != null)
			m_nodeHandler.onStatusRemoteNode(url,(String)message);
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
	
			
	public static String getVersion() {
		if(mHandle==0)
			return null;
		return getVersion(mHandle);
	}

	static {
		System.loadLibrary("OnyxPlayerApi");
	}

	public static long addServer(String url)
	{
		return addServer(mHandle, url);
	}

	public static long removeServer(String url)
	{
		return removeServer(mHandle, url);
	}

	public static long startServer(String url)
	{
		return startServer(mHandle, url);
	}

	public static long stopServer(String url)
	{
		return stopServer(mHandle, url);
	}

	
	public static int getVideoFrame(String url, ByteBuffer data, int size, int nTimeoutMs)
	{
		return getVideoFrame(mHandle, url, data, size);
	}

	public static int getAudioFrame(String url, ByteBuffer data, int size, int nTimeoutMs)
	{
		return getAudioFrame(mHandle, url, data, size);
	}
	

	public static long getClockUs(String url)
	{
		return getClockUs(mHandle, url);
	}
	
	public static long getVideoPts(String url)
	{
		return getVideoPts(mHandle, url);
	}

	public static long getAudioPts(String url)
	{
		return getAudioPts(mHandle, url);
	}
	
	public static int getAudCodecType(String url)
	{
		return getAudCodecType(mHandle, url);
	}		
	public static int getVidCodecType(String url)
	{
		return getVidCodecType(mHandle, url);
	}		

	public static int getNumAvailVideoFrames(String url)
	{
		return getNumAvailVideoFrames(mHandle, url);
	}		

	public static int getNumAvailAudioFrames(String url)
	{
		return getNumAvailAudioFrames(mHandle, url);
	}		

	private native long init();
	private native boolean deinit(long handle);

	public native static String getVersion(long handle);

	public native static long addServer(long handle, String url);	
	public native static long removeServer(long handle, String url);
	
	public native static long startServer(long handle, String url);	
	public native static long stopServer(long handle, String url);	
	
	public native static int getVideoFrame(long handle, String inputId, ByteBuffer vidData, int numBytes);
	public native static int getAudioFrame(long handle, String inputId, ByteBuffer vidData, int numBytes);	
	public native static long getClockUs(long handle, String inputId);
	public native static long getVideoPts(long handle, String inputId);
	public native static long getAudioPts(long handle, String inputId);	
	public native static int getVidCodecType(long handle, String inputId);
	public native static int getAudCodecType(long handle, String inputId);	
	public native static int getNumAvailVideoFrames(long handle, String inputId);
	public native static int getNumAvailAudioFrames(long handle, String inputId);
	
	public native static int onvifDiscvrStart(long handle);
	public native static int onvifDiscvrStop(long handle);	
}
