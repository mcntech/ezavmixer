package com.mcntech.ezscreencast;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.AdapterView.OnItemSelectedListener;

public class ShareMpdLink extends Activity implements OnItemSelectedListener{

    @Override
    protected void onCreate(Bundle savedInstanceState) {
    	int i;
    	//Button mBtnShare;
    	super.onCreate(savedInstanceState);
        setContentView(R.layout.share_mpd_link_dlg);
        //mBtnShare = (Button) findViewById(R.id.buttonShare);
        
        Bundle b = getIntent().getExtras();
        String value = b.getString("nickname");
        OnyxRemoteNode node = new DatabaseHandler(this).read(value);
        String shareLink = "https://" + node.mHost + "/" + node.mBucket + "/" + node.mFolder + "/" + node.mFileprefix + ".mpd";
    }
    
	@Override
	public void onItemSelected(AdapterView<?> parent, View view, int position,
			long id) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void onNothingSelected(AdapterView<?> parent) {
		// TODO Auto-generated method stub
		
	}

}
