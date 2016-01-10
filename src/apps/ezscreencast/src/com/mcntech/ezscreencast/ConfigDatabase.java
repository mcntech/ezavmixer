package com.mcntech.ezscreencast;

import java.util.ArrayList;
import java.util.List;

import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.pm.ApplicationInfo;
import android.preference.PreferenceManager;
import android.widget.AdapterView.OnItemSelectedListener;
import com.google.gson.Gson;
import com.google.gson.GsonBuilder;

public class ConfigDatabase {

	public static int mSegmentDuration;
	public static int mVideoBitrate;
	public static String mVideoResolution;
	public static String mAudioSource;
	public static boolean mEnableVideo;
	public static boolean mEnableAudio;
	public static boolean mSystemApp;
	public static boolean mIsLiveStream;
	public static String mMuxType;
	public static String mVidCodecType;
	public static String mAudCodecType;
	public static boolean mSaveTranscodeFile;
	
	final static String KEY_SEGMENT_DURATION = "segment_duration";
	final static String KEY_IS_LIVE_STREAM = "is_live_stream";
	final static String KEY_MUX_TYPE = "mux_type";	
	final static String KEY_VID_CODEC_TYPE = "vid_codec_type";	
	final static String KEY_AUD_CODEC_TYPE = "aud_codec_type";		
	final static String KEY_PUBLISH_URL_1 = "rtsp:192.168.1.20:554/test";

	final static String KEY_AUDIO_SOURCE = "audio_source";
	final static String KEY_ENABLE_AUDIO = "enable_audio";
	final static String KEY_ENABLE_VIDEO = "enable_video";
	final static String KEY_VIDEO_BITRATE = "video_bitrate";
	final static String KEY_VIDEO_RESOLUTION = "video_resolution";
	final static String AUDSRC_SYSTEM_AUDIO = "System Audio";
	final static String AUDSRC_MIC = "Mic";
	final static String KEY_SAVE_TRANSCODE_FILE = "save_transcode_file";
	
	final static String VID_RES_480P = "640x352@60fps";
	final static String VID_RES_720P = "1280x720@60fps";
	final static String VID_RES_1080P = "1920x1080@60fps";
	final static String VID_RES_4K = "3840x2160@30fps";
	
	final static String MUX_TYPE_TS = "video/mp2t";
	final static String MUX_TYPE_MP4 = "video/mp4";

	final static String VID_CODEC_TYPE_H264 = "avc1.42E01E";
	final static String VID_CODEC_TYPE_HEVC = "vid_codec_hevc";
	
	final static String AUD_CODEC_TYPE_AAC = "aud_codec_aac";
	
	public static ArrayList<OnyxRemoteNode> mOnyxRemoteNodeList = null;
	
	public static void loadSavedPreferences(Context context, boolean isSytemApp) {
		SharedPreferences sharedPreferences = PreferenceManager
				.getDefaultSharedPreferences(context);
		mSegmentDuration = sharedPreferences.getInt(KEY_SEGMENT_DURATION, 4);
		mVideoBitrate = sharedPreferences.getInt(KEY_VIDEO_BITRATE, 6000000);
		mAudioSource = sharedPreferences.getString(KEY_AUDIO_SOURCE, AUDSRC_SYSTEM_AUDIO);
		mEnableVideo = sharedPreferences.getBoolean(KEY_ENABLE_VIDEO, true);
		mIsLiveStream = sharedPreferences.getBoolean(KEY_IS_LIVE_STREAM, true);
		mMuxType = sharedPreferences.getString(KEY_MUX_TYPE, MUX_TYPE_MP4);
		mVidCodecType = sharedPreferences.getString(KEY_VID_CODEC_TYPE, VID_CODEC_TYPE_H264);
		mAudCodecType = sharedPreferences.getString(KEY_AUD_CODEC_TYPE, AUD_CODEC_TYPE_AAC);
		mSaveTranscodeFile = sharedPreferences.getBoolean(KEY_SAVE_TRANSCODE_FILE, true);
		if(isSytemApp) {
			mEnableAudio = sharedPreferences.getBoolean(KEY_ENABLE_AUDIO, false);
		} else {
			mEnableAudio = false;
		}
		mVideoResolution = sharedPreferences.getString(KEY_VIDEO_RESOLUTION, VID_RES_720P);
		mSystemApp = isSytemApp;
		
		SharedPreferences prefs = context.getSharedPreferences("settings", Context.MODE_PRIVATE);
		String value = prefs.getString("RemoteNodeList", null);
		GsonBuilder gsonb = new GsonBuilder();
		Gson gson = gsonb.create();
		OnyxRemoteNode[] list = gson.fromJson(value, OnyxRemoteNode[].class);
		mOnyxRemoteNodeList = new ArrayList<OnyxRemoteNode>();
		if(list != null) {
			for (OnyxRemoteNode item:list){
				mOnyxRemoteNodeList.add(item);
			}
		}
	}

	public static int getVideoWidth() {
		if(mVideoResolution.equals(VID_RES_480P))
			return 640;
		else if(mVideoResolution.equals(VID_RES_720P))
			return 1280;
		else if(mVideoResolution.equals(VID_RES_1080P))
			return 1920;
		else if(mVideoResolution.equals(VID_RES_4K))
			return 3840;
		else
			return 1280;
	}

	public static int getVideoHeight() {
		if(mVideoResolution.equals(VID_RES_480P))
			return 352;
		else if(mVideoResolution.equals(VID_RES_720P))
			return 720;
		else if(mVideoResolution.equals(VID_RES_1080P))
			return 1080;
		else if(mVideoResolution.equals(VID_RES_4K))
			return 2160;
		else
			return 720;
	}

	public static int getVideoFramerate() {
		if(mVideoResolution.equals(VID_RES_480P))
			return 60;
		else if(mVideoResolution.equals(VID_RES_720P))
			return 60;
		else if(mVideoResolution.equals(VID_RES_1080P))
			return 60;
		else if(mVideoResolution.equals(VID_RES_4K))
			return 30;
		else
			return 60;
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
	
	public static void saveRemoteNodeList(Context ctx) {
		
		SharedPreferences sharedPreferences = PreferenceManager
				.getDefaultSharedPreferences(ctx);
	
		GsonBuilder gsonb = new GsonBuilder();
		Gson gson = gsonb.create();
		List<String> list = new ArrayList<String>();

		if(mOnyxRemoteNodeList != null) {
			for (OnyxRemoteNode item:mOnyxRemoteNodeList){
				list.add(item.mNickname);
			}
		}
		String value = gson.toJson(list);
		Editor editor = sharedPreferences.edit();
		editor.putString("RemoteNodeList", value);
		editor.commit();
	}
	
}