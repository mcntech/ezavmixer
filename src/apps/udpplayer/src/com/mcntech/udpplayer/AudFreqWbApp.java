package com.mcntech.udpplayer;

import android.content.Context;
import android.os.Build;
import android.webkit.JavascriptInterface;
import android.widget.Toast;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.Random;

public class AudFreqWbApp {

    public static final int FFT_SIZE = 48;
    public static final int MAX_CHAN = 12;

    public interface AudFreqCallback
    {
        int getNumChannels(int program, int stream);
        int getNumStreams(int program);
        int getData(int program, int stream, byte data[]);
    }
    Context mContext;
    AudFreqCallback mCallback;
    // Instantiate the interface and set the context
    AudFreqWbApp(Context c, AudFreqCallback audFreqCallback) {
        mContext = c;
        mCallback = audFreqCallback;
    }

    // Show a toast from the web page
    @JavascriptInterface
    public void showToast(String toast) {
        Toast.makeText(mContext, toast, Toast.LENGTH_SHORT).show();
    }

    @JavascriptInterface
    public int getAndroidVersion() {
        return android.os.Build.VERSION.SDK_INT;
    }

    @JavascriptInterface
    public void showAndroidVersion(String versionName) {
        Toast.makeText(mContext, versionName, Toast.LENGTH_SHORT).show();
    }

    @JavascriptInterface
    public String getInfo(int program_number) {
        JSONObject Info = new JSONObject();
        JSONArray arProgram = new JSONArray();
        JSONObject Program = new JSONObject();
        try {
            Info.put("action", "get_info");
            JSONObject Streams = new JSONObject();
            JSONArray arStream = new JSONArray();
            int numStreams = mCallback.getNumStreams(program_number);
            for (int i = 0; i < numStreams; i++) {
                JSONObject Stream = new JSONObject();


                JSONArray arChannel = new JSONArray();
                int numChannels = mCallback.getNumChannels(program_number, i);
                String[] channelLabels = {"left", "right", "center", "sub", "sleft", "sright"};
                for (int k = 0; k < numChannels; k++) {
                    JSONObject label = new JSONObject();
                    label.put("label", channelLabels[k]);
                    arChannel.put(label);
                }
                Stream.put("channels",arChannel);
                arStream.put(Stream);
            }

            Program.put("streams", arStream);
            Program.put("program_number", program_number);

            // We have only one program for now
            arProgram.put(Program);
            Info.put("programs", arProgram);
        } catch (JSONException e) {
            e.printStackTrace();
        }
        return Info.toString();
    }

    @JavascriptInterface
    public String getData(int program_number) {
        JSONObject Info = new JSONObject();
        JSONArray arProgram = new JSONArray();
        JSONObject Program = new JSONObject();
        byte []freqData = new byte[FFT_SIZE*MAX_CHAN];

        try {
            Info.put("action", "get_info");
            JSONObject Streams = new JSONObject();
            JSONArray arStream = new JSONArray();
            int numStreams = mCallback.getNumStreams(program_number);
            for (int i = 0; i < numStreams; i++) {
                JSONObject Stream = new JSONObject();

                JSONArray arChannel = new JSONArray();
                int numChannels = mCallback.getNumChannels(program_number, i);
                String[] channelLabels = {"left", "right"};

                mCallback.getData(program_number, i, freqData);
                for (int k = 0; k < numChannels; k++) {
                    JSONObject data = new JSONObject();
                    JSONArray arData = new JSONArray();

                    for(int n=0; n < FFT_SIZE; n++) {
                        arData.put(freqData[k * FFT_SIZE + n]);
                    }
                    data.put("data", arData);
                    arChannel.put(data);
                }
                Stream.put("channels",arChannel);
                arStream.put(Stream);
            }


            Program.put("streams", arStream);
            Program.put("program_number", program_number);

            // We have only one program for now
            arProgram.put(Program);
            Info.put("programs", arProgram);
        } catch (JSONException e) {
            e.printStackTrace();
        }
        return Info.toString();
    }
}