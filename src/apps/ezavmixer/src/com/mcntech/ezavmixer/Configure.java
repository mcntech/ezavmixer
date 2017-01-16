package com.mcntech.ezavmixer;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.preference.PreferenceManager;

public class Configure {
	public static boolean mEnableVideo;
	public static boolean mEnableAudio;
	public static boolean mUseAudioTrack;	
	public static boolean mEnableAutoStart;	
	public static boolean mEnableLogo;
	public static boolean mEnableStats;
	public static boolean mEnableOnScreenChannel;
	
	public static boolean mSystemApp;
	public static int     mAudioDelay;	
	public static String  mRtspUrl1;
	
	final static String KEY_ENABLE_AUDIO = "enable_audio";
	final static String KEY_ENABLE_VIDEO = "enable_video";
	final static String KEY_ENABLE_AUTO_START = "enable_auto_start";
	final static String KEY_ENABLE_ON_SCREEN_CHANNEL = "enable_on_screen_channel";	
	final static String KEY_ENABLE_LOGO = "enable_logo";
	final static String KEY_ENABLE_STATS = "enable_stats";	
	final static String KEY_USE_AUTIO_TRACK = "use_audio_track";
	final static String KEY_AUDIO_DELAY = "audio_delay";
	final static String KEY_RTSP_URL_1 = "rtsp_url_1";
	
	public static void loadSavedPreferences(Context context, boolean isSytemApp) {
		SharedPreferences sharedPreferences = PreferenceManager
				.getDefaultSharedPreferences(context);
		mEnableVideo = sharedPreferences.getBoolean(KEY_ENABLE_VIDEO, true);
		mEnableAudio = sharedPreferences.getBoolean(KEY_ENABLE_AUDIO, false);
		mEnableAutoStart = sharedPreferences.getBoolean(KEY_ENABLE_AUTO_START, false);
		mEnableLogo = sharedPreferences.getBoolean(KEY_ENABLE_LOGO, true);
		mEnableOnScreenChannel = sharedPreferences.getBoolean(KEY_ENABLE_ON_SCREEN_CHANNEL, true);
		mEnableStats = sharedPreferences.getBoolean(KEY_ENABLE_STATS, false);		
		mUseAudioTrack = sharedPreferences.getBoolean(KEY_USE_AUTIO_TRACK, false);
		mAudioDelay = sharedPreferences.getInt(KEY_AUDIO_DELAY, 0);
		mRtspUrl1 = sharedPreferences.getString(KEY_RTSP_URL_1, "rtsp://192.168.0.101:8554/v01");		
		mSystemApp = isSytemApp;
	}

	public static void savePreferences(Context ctx, String key, boolean value) {
		SharedPreferences sharedPreferences = PreferenceManager
				.getDefaultSharedPreferences(ctx);
		Editor editor = sharedPreferences.edit();
		editor.putBoolean(key, value);
		editor.commit();
	}

	public static void savePreferences(Context ctx, String key, String value) {
		SharedPreferences sharedPreferences = PreferenceManager
				.getDefaultSharedPreferences(ctx);
		Editor editor = sharedPreferences.edit();
		editor.putString(key, value);
		editor.commit();
	}
	
	public static void savePreferences(Context ctx,String key, int value) {
		SharedPreferences sharedPreferences = PreferenceManager
				.getDefaultSharedPreferences(ctx);
		Editor editor = sharedPreferences.edit();
		editor.putInt(key, value);
		editor.commit();
	}	
}