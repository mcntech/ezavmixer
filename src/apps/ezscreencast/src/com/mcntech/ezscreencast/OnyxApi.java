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
	public static MpdModel mMpdSession = null;
	public static RtspModel mRtspSession = null;
	
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
	
	public static long initialize(int protocol) {
		if(mHandle == 0) {
			mHandle = mSelf.init(protocol);
		}
		return mHandle;
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

	native long init(int protocol);
	native boolean deinit(long handle);
	native static void stopSession(long handle);

	native static boolean addRtspPublishNode(long handle, String url, String appname);	
	native static boolean removeRemoteNode(long handle, String url);	
		
	native static String getVersion(long handle);
	
	native static int sendAudioData(long handle,String inputId, byte[] pcmBytes, int numBytes, long lPts, int nFlags);
	native static int sendVideoData(long handle,String inputId, byte[] vidBytes, int numBytes, long Pts, int Flags);	
	native static long getClockUs(long handle);	
	native static boolean addS3PublishNode(long handle, String jid,
			String jhost, String jaccessId, String jsecKey,
			String jbucket, String jfolder, String jfilePerfix);

	native static boolean CreateMpd(long  handle, String jid, boolean isLive, int durationMs);	
	native static boolean CreatePeriod(long handle, String jmpdId, String jperiodId);
	native static boolean CreateAdaptationSet(long handle, String jmpdId, String jperiodId, String jadaptId);
	native static boolean CreateRepresentation(long handle,  String jmpdId, String jperiodId, String jadaptId, String jrepId, String mimeType, String mCodecType);
	native static boolean CreateMpdPublishStream(long handle,  String jId, String jmpdId, String jperiodId, String jadaptId, String jrepId, String jswitchId, String jserverNode);
	native static boolean ConfigMpdPublishStream(long handle,  String jId, String jrepId, boolean fIsLive, int nBitrate, int nWidth, int nHeight, int nFramerate);
	native static boolean StartMpdPublishStream(long handle,  String jId);
	
	native static boolean CreateInputStream(long handle, String jid, String jInputType, String jUrl);
	native static boolean CreateSwitch(long handle, String jid);
	native static boolean ConnectSwitchInput(long handle, String jSwitchId, String jInputId);
	native static boolean StartSwitch(long handle, String jid);

	native static boolean CreateRtspPublishBridge(long handle,  String jId);
	native static boolean AddRtspPublishBridgeToMediaSwitch(long handle,  String jId, String jswitchId);
	native static boolean EnableRtspLocalServer(long handle,  String jId,  String jInterfaceName, String jStreamName, int nPort , boolean fEnableMux);	
	native static boolean StartRtspPublishBridge(long handle,  String jId);
	native static boolean StopRtspPublishBridge(long handle,  String jId);
	native static boolean StartRtspPublishNode(long handle,  String jId);
	native static boolean StopRtspPublishNode(long handle,  String jId);
	
	native static boolean UpdateMpdPublishStatus(long mHandle, String mPublishId);
	native static int onvifSrvrStart(long handle);
	native static int onvifSrvrStop(long handle);
}
