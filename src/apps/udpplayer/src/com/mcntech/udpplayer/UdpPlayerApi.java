package com.mcntech.udpplayer;

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
public class UdpPlayerApi {	
	
	public interface RemoteNodeHandler {
		void onPsiChange(String url, String message);
		void onRemoteNodeError(final String url,final String message);
	}
	
	private static long mHandle = 0;
	private static UdpPlayerApi mSelf = null;
	private static RemoteNodeHandler m_nodeHandler = null;
	private static ArrayList<String> mActiveRemoteNodes = new ArrayList<String>();
	
	private UdpPlayerApi() {

	}
	public static boolean initialize() {
		mSelf = new UdpPlayerApi();
		if(mHandle == 0) {
			System.out.println("udp player");
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
	
		
	public static UdpPlayerApi getInstance() {
		return mSelf;
	}
	
	public static void setDeviceHandler(RemoteNodeHandler handler) {
		m_nodeHandler = handler;
	}
	
	public static void onNativeMessage(final Object title,final Object message) {		
		System.out.println("java onNativeMessage:" + title + " message:" + message);
	}

	public static void onPsiChange(String url, String psi)
	{		
		if(m_nodeHandler != null)
			m_nodeHandler.onPsiChange(url, psi);
	}

			
	public static String getVersion() {
		if(mHandle==0)
			return null;
		//return getVersion(mHandle);
		// TODO
		return "1.0";
	}

	static {
		System.loadLibrary("UdpPlayerApi");
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
	
	public static int getVideoFrame (String url, int strmId, ByteBuffer data, int size, int nTimeoutMs )
	{
		return getFrame(mHandle, url, strmId, data, size);
	}

	public static int getAudioFrame(String url, int strmId, ByteBuffer data, int size, int nTimeoutMs)
	{
		return getFrame(mHandle, url, strmId, data, size);
	}
	

	public static long getClockUs(String url, int strmId)
	{
		return getClockUs(mHandle, url, strmId);
	}
	
	public static long getVideoPts(String url, int strmId)
	{
		return getPts(mHandle, url, strmId);
	}

	public static long getAudioPts(String url, int strmId)
	{
		return getPts(mHandle, url, strmId);
	}
	
	public static int getAudCodecType(String url, int strmId)
	{
		return getCodecType(mHandle, url, strmId);
	}		
	public static int getVidCodecType(String url, int strmId)
	{
		return getCodecType(mHandle, url, strmId);
	}		

	public static int getNumAvailVideoFrames(String url, int strmId)
	{
		return getNumAvailFrames(mHandle, url, strmId);
	}		

	public static int getNumAvailAudioFrames(String url, int strmId)
	{
		return getNumAvailFrames(mHandle, url, strmId);
	}		

	private native long init();
	private native boolean deinit(long handle);

	public native static long addServer(long handle, String url);	
	public native static long removeServer(long handle, String url);
	
	public native static long startServer(long handle, String url);	
	public native static long stopServer(long handle, String url);	
	
	public native static int getFrame(long handle, String inputId, int strmId, ByteBuffer vidData, int numBytes);
	public native static long getClockUs(long handle, String inputId, int strmId);
	public native static long getPts(long handle, String inputId, int strmId);
	public native static int getCodecType(long handle, String inputId, int strmId);
	public native static int getNumAvailFrames(long handle, String inputId, int strmId);
}
