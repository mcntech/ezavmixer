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

	public static final int PROTOCOL_MPD = 2;	
	public static final int PROTOCOL_RTSP = 1;
	public static String mError = "";
	public static MpdSession mMpdSession = null;
	
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
	
	public static void UpdateStatus() {
		if(mMpdSession != null) {
			UpdateMpdPublishStatus(mHandle, mMpdSession.mPublishId);
		}
	}
	
	public static boolean startSession(MpdSession mpdSession, boolean enableAud, boolean enabeVid) {

		boolean result = true;
		mMpdSession = mpdSession;
		String jPublishId = mpdSession.mPublishId;
		String jswitchId = mpdSession.mSwcitchId;
		String jinputId = mpdSession.mInputId;
		String jInputType = mpdSession.mInputType;
		String jUrl = mpdSession.mInputUrl;		
		CreateInputStream(mHandle, jinputId, jInputType, jUrl);
		CreateSwitch(mHandle, jswitchId);
		ConnectSwitchInput(mHandle, jswitchId, jinputId);
		
		OnyxRemoteNode node = mpdSession.mNode;
		String jserverId = node.mNickname;
		String jhost = node.mHost;
		String jaccessId = node.mAccessid;
		String jsecKey = node.mSecuritykey;
		String jbucket = node.mBucket;
		String jfolder = node.mFolder;
		String jfilePerfix = node.mFileprefix;
	
		boolean fIsLive = ConfigDatabase.mIsLiveStream;
		int nBitrate = ConfigDatabase.mVideoBitrate;
		int nWidth = ConfigDatabase.getVideoWidth();
		int nHeight = ConfigDatabase.getVideoHeight();
		int nFramerate = ConfigDatabase.getVideoFramerate();
		int segmentDurationMs = ConfigDatabase.mSegmentDuration * 1000;

		addS3PublishNode(mHandle, jserverId,
				jhost, jaccessId, jsecKey,
				jbucket, jfolder, jfilePerfix);
		
		String jmpdId = mpdSession.mMpdId;
		result = CreateMpd(mHandle, jmpdId, fIsLive, segmentDurationMs);
		if(!result) {
			mError = "Failed to create MPD";
			return result;
		}

		String jperiodId = mpdSession.mPeriodId;
		result = CreatePeriod(mHandle, jmpdId, jperiodId);
		if(!result) {
			mError = "Failed to create PERIOD";
			return result;
		}

		String jadaptId = mpdSession.mAdaptId;		
		result  = CreateAdaptationSet(mHandle, jmpdId, jperiodId, jadaptId);
		if(!result) {
			mError = "Failed to create ADAPTATION";
			return result;
		}
		
		String jrepId = mpdSession.mRepId;		
		result  = CreateRepresentation(mHandle,  jmpdId, jperiodId,jadaptId, jrepId);
		if(!result) {
			mError = "Failed to create Representation";
			return result;
		}

		//String jserverNode = MpdSession.mServerId;
		result = CreateMpdPublishStream(mHandle, jPublishId, jmpdId, jperiodId, jadaptId, jrepId, jswitchId, jserverId);

		if(!result) {
			mError = "Failed to create PublishStream";
			return result;
		}

		result = StartSwitch(mHandle, jswitchId);
		result= StartMpdPublishStream(mHandle, jPublishId);
		if(!result) {
			mError = "Failed to StartPublishStream";
			return result;
		}
		
		mError = "Succcess";
		return result;
	}

	public static void stopSession(){
		
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
			
	public static void onMpdPublishStatus(final String jPublishId, int nState, int nStrmInTime, int nStrmOutTime, int nLostBufferTime){
		if(m_nodeHandler != null){
			String statusMsg = " State=" + nState + " StrmInTime" + nStrmInTime + " StrmOutTime" + nStrmOutTime + " LostBufferTime=" + nLostBufferTime;
			m_nodeHandler.onStatusRemoteNode(jPublishId, statusMsg);
		}
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
	private native static void stopSession(long handle);

	private native static boolean addRtspPublishNode(long handle, String url, String appname);	
	private native static boolean removeRemoteNode(long handle, String url);	
		
	public native static String getVersion(long handle);
	
	private native static int sendAudioData(long handle,String inputId, byte[] pcmBytes, int numBytes, long lPts, int nFlags);
	private native static int sendVideoData(long handle,String inputId, byte[] vidBytes, int numBytes, long Pts, int Flags);	
	private native static long getClockUs(long handle);	
	private native static boolean addS3PublishNode(long handle, String jid,
			String jhost, String jaccessId, String jsecKey,
			String jbucket, String jfolder, String jfilePerfix);
	private native static boolean CreateMpd(long  handle, String jid, boolean isLive, int durationMs);	
	private native static boolean CreatePeriod(long handle, String jmpdId, String jperiodId);
	private native static boolean CreateAdaptationSet(long handle, String jmpdId, String jperiodId, String jadaptId);
	private native static boolean CreateRepresentation(long handle,  String jmpdId, String jperiodId, String jadaptId, String jrepId);
	private native static boolean CreateMpdPublishStream(long handle,  String jId, String jmpdId, String jperiodId, String jadaptId, String jrepId, String jswitchId, String jserverNode);
	private native static boolean ConfigMpdPublishStream(long handle,  String jId, String jrepId, boolean fIsLive, int nBitrate, int nWidth, int nHeight, int nFramerate);
	private native static boolean StartMpdPublishStream(long handle,  String jId);
	private native static boolean CreateInputStream(long handle, String jid, String jInputType, String jUrl);
	private native static boolean CreateSwitch(long handle, String jid);
	private native static boolean ConnectSwitchInput(long handle, String jSwitchId, String jInputId);
	private native static boolean StartSwitch(long handle, String jid);
	private native static boolean UpdateMpdPublishStatus(long mHandle, String mPublishId);
}
