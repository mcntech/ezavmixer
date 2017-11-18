package com.mcntech.udpplayer;

import android.media.MediaCodecInfo;
import android.media.MediaCodecInfo.CodecCapabilities;
import android.media.MediaCodecInfo.CodecProfileLevel;
import android.media.MediaCodecList;

public class CodecInfo
{
	public static boolean isSupportedLevel(String mimeType, int level) {
		MediaCodecInfo info = null;
		CodecCapabilities capabilities = null;
		CodecProfileLevel[] profileLevels;
		for (int i = 0; i < MediaCodecList.getCodecCount() && info == null; i++) {
		    MediaCodecInfo curInfo = MediaCodecList.getCodecInfoAt(i);
		    if (curInfo.isEncoder())
		        continue;
		    String[] types = curInfo.getSupportedTypes();
		    for (int j = 0; j < types.length; j++) {
		        if (types[j].equals(mimeType)){
		            info = curInfo;
		            capabilities = curInfo.getCapabilitiesForType(mimeType);
		            profileLevels = capabilities.profileLevels;
		            for (int k=0; k < profileLevels.length; k++){
		            	if(level == profileLevels[k].level){
		            		return true;
		            	}
		            }
		        }		
		    }
		}
		return false;
	}
	
	public static boolean isMimeTypeAvailable(String mimeType) {
		MediaCodecInfo info = null;
		CodecCapabilities capabilities = null;
		for (int i = 0; i < MediaCodecList.getCodecCount() && info == null; i++) {
		    MediaCodecInfo curInfo = MediaCodecList.getCodecInfoAt(i);
		    if (curInfo.isEncoder())
		        continue;
		    String[] types = curInfo.getSupportedTypes();
		    for (int j = 0; j < types.length; j++) {
		        if (types[j].equals(mimeType)){
		        	return true;
		        }		
		    }
		}
		return false;
	}
}