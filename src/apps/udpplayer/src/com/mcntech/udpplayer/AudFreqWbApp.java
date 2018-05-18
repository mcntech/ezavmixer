package com.mcntech.udpplayer;

import android.content.Context;
import android.os.Build;
import android.util.Log;
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
        int getData(int program, int stream, short data[], int spectWidth);
    }
    Context mContext;
    AudFreqCallback mCallback;
    int mProgram = 0;
    // Instantiate the interface and set the context
    AudFreqWbApp(Context c, AudFreqCallback audFreqCallback, int program) {
        mContext = c;
        mCallback = audFreqCallback;
        mProgram = program;
    }

    @JavascriptInterface
    public boolean isReady() {
        int numChannels = 0;
        int numStreams = mCallback.getNumStreams(mProgram);
        if(numStreams == 0)
            return false;

        for (int i = 0; i < numStreams; i++) {
            numChannels = mCallback.getNumChannels(mProgram, i);
            if(numChannels == 0)
                return false;
        }
        return true;
    }

    @JavascriptInterface
    public String getInfo() {
        JSONObject Info = new JSONObject();
        JSONArray arProgram = new JSONArray();
        JSONObject Program = new JSONObject();
        try {
            Info.put("action", "get_info");
            JSONObject Streams = new JSONObject();
            JSONArray arStream = new JSONArray();
            int numStreams = mCallback.getNumStreams(mProgram);
            for (int i = 0; i < numStreams; i++) {
                JSONObject Stream = new JSONObject();


                JSONArray arChannel = new JSONArray();
                int numChannels = mCallback.getNumChannels(mProgram, i);
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
            Program.put("program", mProgram);

            // We have only one program for now
            arProgram.put(Program);
            Info.put("programs", arProgram);
        } catch (JSONException e) {
            e.printStackTrace();
        }
        Log.d("AudFreqWbAp ",  Info.toString());
        return Info.toString();
    }

    @JavascriptInterface
    public String getData() {
        JSONObject Info = new JSONObject();
        JSONArray arProgram = new JSONArray();
        JSONObject Program = new JSONObject();
        short []freqData = new short[FFT_SIZE*6];

        try {
            Info.put("action", "get_data");
            JSONObject Streams = new JSONObject();
            JSONArray arStream = new JSONArray();
            int numStreams = mCallback.getNumStreams(mProgram);
            for (int i = 0; i < numStreams; i++) {
                JSONObject Stream = new JSONObject();

                JSONArray arChannel = new JSONArray();
                int numChannels = mCallback.getNumChannels(mProgram, i);
                String[] channelLabels = {"left", "right"};

                mCallback.getData(mProgram, i, freqData, FFT_SIZE);
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
            Program.put("program", mProgram);

            // We have only one program for now
            arProgram.put(Program);
            Info.put("programs", arProgram);
        } catch (JSONException e) {
            e.printStackTrace();
        }
        return Info.toString();
    }
}