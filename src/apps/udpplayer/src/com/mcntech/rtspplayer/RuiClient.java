
package com.mcntech.rtspplayer;

/**
 * Created by ram on 2/5/16.
 */
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.PrintStream;
import java.net.InetAddress;
import java.net.Socket;
import java.net.UnknownHostException;

import org.json.JSONException;
import org.json.JSONObject;

import android.app.Activity;
import android.content.Context;
import android.net.nsd.NsdManager;
import android.net.nsd.NsdServiceInfo;
import android.os.AsyncTask;
import android.util.Log;
import android.widget.TextView;

public class RuiClient extends AsyncTask<Void, Void, Void> {

	final static String TAG = "RuiClient";
    public static String mDstAddress = null;
    public static int mDstPort;
    public static Socket mSocket = null;
    public static OutputStream mOutputStream = null;
    public static PrintStream mPrintStream = null;
    
    String response = "";
    String msgTobeSent;
    TextView textResponse;

    private static String SERVICE_NAME = "FcTouchClient";
    private static String SERVICE_TYPE = "_fcinput._tcp.";
    static NsdManager mNsdManager = null;
    static NsdManager.ResolveListener mResolveListener;
    static NsdManager.DiscoveryListener mDiscoveryListener;
     
    RuiClient(String action, int ptId, int x, int y) throws JSONException {
    	JSONObject ptAction = new JSONObject();
    	if(action.equalsIgnoreCase("down"))
    		ptAction.put("ACTION", "ACTION_DOWN");
    	else if (action.equalsIgnoreCase("up"))
    		ptAction.put("ACTION", "ACTION_UP");
        else if (action.equalsIgnoreCase("move"))  
        	ptAction.put("ACTION", "ACTION_MOVE");
    	ptAction.put("ACTION_POINTER_INDEX", ptId);	    
    	ptAction.put("AXIS_X", x);
    	ptAction.put("AXIS_Y", y);
    	msgTobeSent = ptAction.toString();
    }
    
    public static int init(Activity activity) {
        // NSD Stuff
    	
    	mDiscoveryListener = new NsdManager.DiscoveryListener() {

            // Called as soon as service discovery begins.
            @Override
            public void onDiscoveryStarted(String regType) {
                Log.d(TAG, "Service discovery started");
            }

            @Override
            public void onServiceFound(NsdServiceInfo service) {
                // A service was found! Do something with it.
                Log.d(TAG, "Service discovery success : " + service);
                Log.d(TAG, "Host = "+ service.getServiceName());
                Log.d(TAG, "port = " + String.valueOf(service.getPort()));

                if (!service.getServiceType().equals(SERVICE_TYPE)) {
                    // Service type is the string containing the protocol and
                    // transport layer for this service.
                    Log.d(TAG, "Unknown Service Type: " + service.getServiceType());
                } else if (service.getServiceName().equals(SERVICE_NAME)) {
                    // The name of the service tells the user what they'd be
                    // connecting to. It could be "Bob's Chat App".
                    Log.d(TAG, "Same machine: " + SERVICE_NAME);
                } else {
                    Log.d(TAG, "Diff Machine : " + service.getServiceName());
                    // connect to the service and obtain serviceInfo
                    mNsdManager.resolveService(service, mResolveListener);
                }
            }

            @Override
            public void onServiceLost(NsdServiceInfo service) {
                // When the network service is no longer available.
                // Internal bookkeeping code goes here.
                Log.e(TAG, "service lost" + service);
            }

            @Override
            public void onDiscoveryStopped(String serviceType) {
                Log.i(TAG, "Discovery stopped: " + serviceType);
            }

            @Override
            public void onStartDiscoveryFailed(String serviceType, int errorCode) {
                Log.e(TAG, "Discovery failed: Error code:" + errorCode);
                mNsdManager.stopServiceDiscovery(this);
            }

            @Override
            public void onStopDiscoveryFailed(String serviceType, int errorCode) {
                Log.e(TAG, "Discovery failed: Error code:" + errorCode);
                mNsdManager.stopServiceDiscovery(this);
            }
        };

        mResolveListener = new NsdManager.ResolveListener() {

            @Override
            public void onResolveFailed(NsdServiceInfo serviceInfo, int errorCode) {
                // Called when the resolve fails. Use the error code to debug.
                Log.e(TAG, "Resolve failed " + errorCode);
                Log.e(TAG, "serivce = " + serviceInfo);
            }

            @Override
            public void onServiceResolved(NsdServiceInfo serviceInfo) {
                Log.d(TAG, "Resolve Succeeded. " + serviceInfo);

                if (serviceInfo.getServiceName().equals(SERVICE_NAME)) {
                    Log.d(TAG, "Same IP.");
                    return;
                }

                // Obtain port and IP
                int hostPort = serviceInfo.getPort();
                InetAddress hostAddress = serviceInfo.getHost();
                RuiClient.setFcTransmitterAddress(hostAddress.getHostAddress(), hostPort);
            }
        };
        mNsdManager = (NsdManager) activity.getSystemService(Context.NSD_SERVICE);
        mNsdManager.discoverServices(SERVICE_TYPE,
        		NsdManager.PROTOCOL_DNS_SD, mDiscoveryListener);

    	return 0;
    }
    
    public static void setFcTransmitterAddress(String Address, int Port)
    {
        mDstAddress = Address;
        mDstPort = Port;
        if (mSocket != null) {
            try {
            	mSocket.close();
            	mSocket = null;
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    	try {
			mSocket = new Socket(mDstAddress, mDstPort);
			mSocket.setTcpNoDelay(true);
        	mOutputStream = mSocket.getOutputStream();
            mPrintStream = new PrintStream(mOutputStream);
		} catch (UnknownHostException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
    }

    @Override
    protected Void doInBackground(Void... arg0) {

      Log.d(TAG, "Sending touch event: " + msgTobeSent);
      if(mSocket != null) {  
            mPrintStream.print(msgTobeSent);
            mPrintStream.flush( );
            Log.d(TAG, "Sending touch event: Complete");
		} else {
            Log.d(TAG, "Sending touch event: Socket not open");
		}

        return null;
    }

    @Override
    protected void onPostExecute(Void result) {
        //textResponse.setText(response);
        super.onPostExecute(result);
    }

}