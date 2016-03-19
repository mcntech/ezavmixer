package com.mcntech.rtspplayer;

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
	
	public static boolean mSystemApp;
	public static int     mAudioDelay;	
	final static String KEY_ENABLE_AUDIO = "enable_audio";
	final static String KEY_ENABLE_VIDEO = "enable_video";
	final static String KEY_ENABLE_AUTO_START = "enable_auto_start";
	final static String KEY_ENABLE_LOGO = "enable_logo";
	final static String KEY_ENABLE_STATS = "enable_stats";	
	final static String KEY_USE_AUTIO_TRACK = "use_audio_track";
	final static String KEY_AUDIO_DELAY = "audio_delay";
	
	public static void loadSavedPreferences(Context context, boolean isSytemApp) {
		SharedPreferences sharedPreferences = PreferenceManager
				.getDefaultSharedPreferences(context);
		mEnableVideo = sharedPreferences.getBoolean(KEY_ENABLE_VIDEO, true);
		mEnableAudio = sharedPreferences.getBoolean(KEY_ENABLE_AUDIO, false);
		mEnableAutoStart = sharedPreferences.getBoolean(KEY_ENABLE_AUTO_START, true);
		mEnableLogo = sharedPreferences.getBoolean(KEY_ENABLE_LOGO, true);
		mEnableStats = sharedPreferences.getBoolean(KEY_ENABLE_STATS, false);		
		mUseAudioTrack = sharedPreferences.getBoolean(KEY_USE_AUTIO_TRACK, false);
		mAudioDelay = sharedPreferences.getInt(KEY_AUDIO_DELAY, 0);		
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