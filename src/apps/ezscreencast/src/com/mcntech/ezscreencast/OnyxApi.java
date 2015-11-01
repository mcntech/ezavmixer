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

	public static final int PROTOCOL_MPD = 1;	
	public static final int PROTOCOL_RTSP = 2;
	
	public interface RemoteNodeHandler {
		void onConnectRemoteNode(String url);
		void onDisconnectRemoteNode(String url);
		void onStatusRemoteNode(String url, final String message);		
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
	
	public static void initialize(int protocol) {
		if(mHandle == 0) {
			mHandle = mSelf.init(protocol);
		}
	}

	public static void deinitialize() {
		try {
			mSelf.finalize();
		} catch (Throwable e) {
			e.printStackTrace();
		}
	}

	public static void startSession(MpdSession mpdSession, boolean enableAud, boolean enabeVid) {

		String jswitchId = mpdSession.mSwcitchId;
		String jinputId = mpdSession.mInputId;
		String jInputType = mpdSession.mInputType;
		String jUrl = mpdSession.mInputUrl;		
		CreateInputStream(mHandle, jinputId, jInputType, jUrl);
		CreateSwitch(mHandle, jswitchId);
		ConnectSwitchInput(mHandle, jswitchId, jinputId);
		
		String jserverId = mpdSession.mServerId;
		String jhost = mpdSession.mHost;
		String jaccessId = mpdSession.mAccessId;
		String jsecKey = mpdSession.mSecKey;
		String jbucket = mpdSession.mBucket;
		String jfolder = mpdSession.mFolder;
		String jfilePerfix = mpdSession.mFilePrefix;
		addS3PublishNode(mHandle, jserverId,
				jhost, jaccessId, jsecKey,
				jbucket, jfolder, jfilePerfix);
		String jmpdId = mpdSession.mMpdId;
		CreateMpd(mHandle, jmpdId);
		String jperiodId = mpdSession.mPeriodId;
		CreatePeriod(mHandle, jmpdId, jperiodId);
		String jadaptId = mpdSession.mAdaptId;		
		CreateAdaptationSet(mHandle, jmpdId, jperiodId, jadaptId);
		String jrepId = mpdSession.mRepId;		
		CreateRepresentation(mHandle,  jmpdId, jperiodId,jadaptId, jrepId);

		String jserverNode = mpdSession.mServerId;
		CreateMpdPublishStream(mHandle, jmpdId, jperiodId, jadaptId, jrepId, jswitchId, jserverNode);

		startSession(mHandle, enableAud, enabeVid);
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
	
    public static void addRtspPublishNode(String url){
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
	

	static {
		System.loadLibrary("OnyxApi");
		mSelf = new OnyxApi();
	}
	
	
	public static String getVersion() {
		if(mHandle==0)
			return null;
		return getVersion(mHandle);
	}
	
	public static int sendAudioData(String inputId, byte[] pcmBytes, int numBytes, long lPts, int nFlags){
		if(mHandle==0)
			return 0;
		return sendAudioData(mHandle, inputId, pcmBytes,numBytes, lPts, nFlags);
	}

	public static int sendVideoData(String inputId, byte[] vidBytes, int numBytes, long lPts, int nFlags){
		if(mHandle==0)
			return 0;
		return sendVideoData(mHandle,inputId,vidBytes,numBytes, lPts, nFlags);
	}
	

	public static long getClockUs(){
		if(mHandle==0)
			return 0;
		return getClockUs(mHandle);
	}

	private native long init(int protocol);
	private native boolean deinit(long handle);
	private native static void startSession(long handle, boolean enableAud, boolean enableVid);
	private native static void stopSession(long handle);

	private native static boolean addRtspPublishNode(long handle, String url, String appname);	
	private native static boolean removeRemoteNode(long handle, String url);	
	
	private native static boolean start(long handle);
	private native static boolean stop(long handle);
	private native static boolean pause(long handle);
	private native static boolean resume(long handle);	
	
	public native static String getVersion(long handle);
	
	private native static int sendAudioData(long handle,String inputId, byte[] pcmBytes, int numBytes, long lPts, int nFlags);
	private native static int sendVideoData(long handle,String inputId, byte[] vidBytes, int numBytes, long Pts, int Flags);	
	private native static long getClockUs(long handle);	
	private native static boolean addS3PublishNode(long handle, String jid,
			String jhost, String jaccessId, String jsecKey,
			String jbucket, String jfolder, String jfilePerfix);
	private native static boolean CreateMpd(long  handle, String jid);	
	private native static boolean CreatePeriod(long handle, String jmpdId, String jperiodId);
	private native static boolean CreateAdaptationSet(long handle, String jmpdId, String jperiodId, String jadaptId);
	private native static boolean CreateRepresentation(long handle,  String jmpdId, String jperiodId, String jadaptId, String jrepId);
	private native static boolean CreateMpdPublishStream(long handle,  String jmpdId, String jperiodId, String jadaptId, String jrepId, String jswitchId, String jserverNode);
	private native static boolean CreateInputStream(long handle, String jid, String jInputType, String jUrl);
	private native static boolean CreateSwitch(long handle, String jid);
	private native static boolean ConnectSwitchInput(long handle, String jSwitchId, String jInputId);
}
