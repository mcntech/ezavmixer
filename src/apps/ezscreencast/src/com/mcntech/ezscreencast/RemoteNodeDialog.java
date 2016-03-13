package com.mcntech.ezscreencast;

import android.app.Activity;
import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.Spinner;
import java.util.ArrayList;
import java.util.List;
import android.view.View.OnClickListener;
import com.mcntech.ezscreencast.R;

public class RemoteNodeDialog extends Activity {
	EditText mNodeUrl;
	public static ListView mRemoteNodeList = null;
	ArrayAdapter<OnyxRemoteNode> mListAdapter;
	@Override
    protected void onCreate(Bundle savedInstanceState) {
    	int i;
    	super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_nodelist);
        CodecModel.loadSavedPreferences(this, false);
                
        mNodeUrl = (EditText) findViewById(R.id.node_url);
        mRemoteNodeList = (ListView)findViewById(R.id.node_list);
        mListAdapter = new ArrayAdapter<OnyxRemoteNode>(this, 
				android.R.layout.simple_list_item_1, CodecModel.mOnyxRemoteNodeList); 
		mRemoteNodeList.setAdapter(mListAdapter ); 
		//mRemoteNodeList.setChoiceMode(ListView.CHOICE_MODE_MULTIPLE);
		mRemoteNodeList.setOnItemClickListener(new AdapterView.OnItemClickListener(){
            @Override
            public void onItemClick(AdapterView<?> parent, View item,
                    int position, long id)
            {
            	OnyxRemoteNode node = (OnyxRemoteNode)mListAdapter.getItem(position); 
            	CodecModel.mOnyxRemoteNodeList.remove(position);
            	CodecModel.saveRemoteNodeList(getApplicationContext());
            	mListAdapter.notifyDataSetChanged();
            }
        });
	}

	
    public void onAddRemoteNode(View v) {
		OnyxRemoteNode node = new OnyxRemoteNode(mNodeUrl.getText().toString());
		CodecModel.mOnyxRemoteNodeList.add(node);
		CodecModel.saveRemoteNodeList(getApplicationContext());
		mListAdapter.notifyDataSetChanged();
    }
    public void onClearAllNodes(View v) {
		OnyxRemoteNode node = new OnyxRemoteNode(mNodeUrl.toString());
		CodecModel.mOnyxRemoteNodeList.add(node);
    }

}
