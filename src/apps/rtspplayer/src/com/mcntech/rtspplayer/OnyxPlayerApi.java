/*****************************************************************************
* Copyright (c) 2006-2012 BLACKFIRE RESEARCH CORP.                           *
* All Rights Reserved.                                                       *
* Blackfire Research Corporation, 1820 Lyon St, San Francisco, CA 94115.     *
* Tel 415 874 8579                                                           *
*                                                                            *
* This software and all related material is part of Blackfire Research Corps *
* Software Development Kit and is governed by the terms of Blackfire         *
* Research Corps Software License Agreement. By viewing or using this        *
* software you accept the terms of the Blackfire Research Corp Software      *
* License Agreement. Please see the LICENSE.TXT file for more information.   *
*                                                                            *
* This copyright and terms notice shall not be deleted or modified.          *
*                                                                            *
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY     *
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE        *
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A                 *
* PARTICULAR PURPOSE.                                                        *
******************************************************************************/
package com.mcntech.rtspplayer;
//import bfrx.WorkerThread;

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
	
	public interface SessionHandler {
		void onStartPlay();
		void onStopPlay();
		void onNetworkDisconnected();
		void onSessionError(final long sessionid,final String message);
	}
	
	private static long mHandle = 0;
	private static OnyxPlayerApi mSelf = null;
	static Context mAppContext = null;
	private static boolean RETRY = true;//false = initialize one time and do not retry
	//private static WorkerThread mWorkerThread;
	private static SessionHandler m_sessionHandler = null;
	final static Object mWaitOnDevice = new Object();
	private static long mplayTime = -1;
	private static long MINIMUMEVENTTIME = 1000;
	private static boolean mDiscontinuity = false;
	/* mActiveDevices contains the list of destination deviceIDs 
	which is updated both in MediaController and GroupListActivity class */
	static boolean mStopping = false;
	static Object mWaitOnStop = new Object();
	
	private OnyxPlayerApi() {
		//mWorkerThread = new WorkerThread();

	}
	/*
	public static void initialize(boolean retry, int role) {
		if(mHandle == 0) {
			int deviceIP = 0;//TODO: use local device ip to specify the local network adapter to run on
			System.out.println("java initializing mobilecast");
			mHandle = mSelf.init(deviceIP,retry, role);
		}
	}
	*/
	public static boolean initialize(Context appConext, boolean retry, int role, int audFramesPerPeriod, int audNumPeriods, int audDelayUs) {
		mSelf = new OnyxPlayerApi();
		if(mHandle == 0) {
			if(appConext == null)
				return false;
			mAppContext = appConext;
			//String configFile = mAppContext.getCacheDir().getAbsolutePath() + "/sshdDst.con";
			int deviceIP = 0;//TODO: use local device ip to specify the local network adapter to run on
			System.out.println("java initializing mobilecast");
			mHandle = mSelf.init(deviceIP, retry, role, audFramesPerPeriod, audNumPeriods, audDelayUs);
			//if(mHandle != 0)
			//	setLossless(mHandle,false);
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
	
	public static void setDeviceHandler(SessionHandler handler) {
		m_sessionHandler = handler;
	}
	
	
	public static void onNativeMessage(final Object title,final Object message) {		
		System.out.println("java onNativeMessage:" + title + " message:" + message);
	}

	public static void onStartPlay() {		
		if(m_sessionHandler != null)
			m_sessionHandler.onStartPlay();
	}

	public static void onStopPlay() {		
		if(m_sessionHandler != null)
			m_sessionHandler.onStopPlay();
	}

	public static void onDeviceError(final long deviceid, final Object message ) {		
		if(m_sessionHandler != null)
			m_sessionHandler.onSessionError(deviceid,(String)message);
	}
	
	private static void onNetworkDisconnected() {
		System.out.println("java onNetworkDisconnected");
		if(m_sessionHandler != null)
			m_sessionHandler.onNetworkDisconnected();
	}
	
	public static void onPrivateData(final long uniqueID, final Object title,final Object properties){
		System.out.println("java onPrivateData:" + title + " properties:" + properties);
	}
		
	public static String getVersion() {
		if(mHandle==0)
			return null;
		return getVersion(mHandle);
	}
	public static String getInfoStats() {
		if(mHandle==0)
			return null;
		return getOperatingInfo(mHandle);
	}

	static {
		System.loadLibrary("fcvdst");
		//mSelf = new DeviceController();
	}
	
	public static int getVideoFrame(ByteBuffer data, int size, int nTimeoutMs)
	{
		return getVideoFrame(mHandle, data, size, nTimeoutMs);
	}

	public static int getAudioFrame(ByteBuffer data, int size, int nTimeoutMs)
	{
		return getAudioFrame(mHandle, data, size, nTimeoutMs);
	}
	
	public static int getNumAvailVideoFrames()
	{
		return getNumAvailVideoFrames(mHandle);
	}
	
	public static long getFcClockUs()
	{
		return getFcClockUs(mHandle);
	}
	
	public static long getVideoPts()
	{
		return getVideoPts(mHandle);
	}

	public static long getAudioPts()
	{
		return getAudioPts(mHandle);
	}
	
	public static int getVidCodecType()
	{
		return getVidCodecType(mHandle);
	}		
	private native long init(int ip, boolean retry, int role, int audFramesPerPeriod, int audNumPeriods, int audDelayUs);
	private native boolean deinit(long handle);
	private native long restart(long handle,int ip);
	private native static boolean volumeUp(long handle);
	private native static boolean volumeDown(long handle);
	private native static boolean volumeChangeAll(long handle,int valuechange);
	private native static boolean volumeChangeDevice(long handle, long devid, int valuechange);
	public native static String getVersion(long handle);
	public native static String getOperatingInfo(long handle);
	public native static int getVideoFrame(long handle, ByteBuffer vidData, int numBytes, int nTimeoutMs);
	public native static int getAudioFrame(long handle, ByteBuffer vidData, int numBytes, int nTimeoutMs);	
	public native static int getNumAvailVideoFrames(long handle);
	public native static long getFcClockUs(long handle);
	public native static long getVideoPts(long handle);
	public native static long getAudioPts(long handle);	
	public native static int getVidCodecType(long handle);
}
