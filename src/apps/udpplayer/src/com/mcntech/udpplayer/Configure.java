package com.mcntech.udpplayer;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.preference.PreferenceManager;

public class Configure {
	public static boolean mEnableAudioSpect;
	public static boolean mEnableStats;
	
	public static boolean mSystemApp;
	public static String  mUrl;
	
	final static String KEY_ENABLE_AUDIO = "enable_audio";
	final static String KEY_ENABLE_STATS = "enable_stats";	

	final static String KEY_URL = "url";
	
	public static void loadSavedPreferences(Context context, boolean isSytemApp) {
		SharedPreferences sharedPreferences = PreferenceManager
				.getDefaultSharedPreferences(context);
		mEnableAudioSpect = sharedPreferences.getBoolean(KEY_ENABLE_AUDIO, false);
		mEnableStats = sharedPreferences.getBoolean(KEY_ENABLE_STATS, false);
		mUrl = sharedPreferences.getString(KEY_URL, "udp://239.255.6.9:1234");
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