package com.mcntech.udpplayer;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.ApplicationInfo;
import android.preference.PreferenceManager;

public class Receiver extends BroadcastReceiver {
	@Override
	public void onReceive(Context context, Intent intent) {
        //if (Intent.ACTION_BOOT_COMPLETED.equals(intent.getAction())) 
		SharedPreferences sharedPreferences = PreferenceManager
				.getDefaultSharedPreferences(context);

		
		if (intent.getAction().equals(
	            android.net.ConnectivityManager.CONNECTIVITY_ACTION))	
        {
			//if(mEnableAutoStart) {
	            //Intent serviceIntent = new Intent(context, SinglePlayerActivity.class);
	            //serviceIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
	            //context.startActivity(serviceIntent);
			//}
        }
	}
}