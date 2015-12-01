package com.mcntech.ezscreencast;

import java.util.List;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.EditText;
import android.widget.TextView;

public class ConfigMpdSession extends Activity implements OnItemSelectedListener {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
    	int i;
    	Button mBtnSave;
    	super.onCreate(savedInstanceState);
        setContentView(R.layout.mpd_session_dlg);
        mBtnSave = (Button) findViewById(R.id.buttonSave);
        
        Bundle b = getIntent().getExtras();
        String value = b.getString("nickname");
        OnyxRemoteNode node = new DatabaseHandler(this).read(value);

        if(node != null) {
	        TextView ctrlTextView = (TextView)findViewById(R.id.textviewNickName);
	        ctrlTextView.setText(node.mNickname);
	        
	        mySetEditText(R.id.editAccessId,node.mAccessid);
	        mySetEditText(R.id.editSecKey,node.mSecuritykey);
	        mySetEditText(R.id.editHost,node.mHost);
	        mySetEditText(R.id.editPrefix,node.mFileprefix);
	        mySetEditText(R.id.editFolder,node.mFolder);
	        mySetEditText(R.id.editBucket,node.mBucket);
        }
        mBtnSave.setOnClickListener(new OnClickListener() {
	        public void onClick(View v) {
	        	OnyxRemoteNode node = new OnyxRemoteNode();

		        TextView ctrlTextView = (TextView)findViewById(R.id.textviewNickName);
		        node.mNickname = ctrlTextView.getText().toString();

	            node.mAccessid = myGetEditText(R.id.editAccessId);
	            node.mSecuritykey = myGetEditText(R.id.editSecKey);
	            node.mHost = myGetEditText(R.id.editHost);
	            node.mFileprefix = myGetEditText(R.id.editPrefix);
	            node.mFolder = myGetEditText(R.id.editFolder);
	            node.mBucket = myGetEditText(R.id.editBucket);	
	            
	            boolean result = new DatabaseHandler(getApplicationContext()).update(node);
	        	finish();
	        }
    	});
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
    void mySetEditText(int id, String value) {
        EditText ctrl = (EditText)findViewById(id);
        if(ctrl != null)
        	ctrl.setText(value);
    }
    String  myGetEditText(int id) {
    	String value = "";
        EditText ctrl = (EditText)findViewById(id);
        if(ctrl != null)
        	value = ctrl.getText().toString();
        return value;
    }    
}
