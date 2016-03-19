package com.mcntech.rtspplayer;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.ApplicationInfo;
import android.preference.PreferenceManager;

public class StartFcvdst extends BroadcastReceiver {
	@Override
	public void onReceive(Context context, Intent intent) {
        //if (Intent.ACTION_BOOT_COMPLETED.equals(intent.getAction())) 
		SharedPreferences sharedPreferences = PreferenceManager
				.getDefaultSharedPreferences(context);
		boolean mEnableAutoStart = sharedPreferences.getBoolean(Configure.KEY_ENABLE_AUTO_START, true);
		
		if (intent.getAction().equals(
	            android.net.ConnectivityManager.CONNECTIVITY_ACTION))	
        {
			if(mEnableAutoStart) {
	            Intent serviceIntent = new Intent(context, DecodeActivity.class);
	            serviceIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
	            context.startActivity(serviceIntent);
			}
        }
	}
}