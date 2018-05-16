package com.mcntech.udpplayer;

import android.media.MediaFormat;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.HashMap;
import java.util.Map;

public class AudDecApi {

    public final static class BufferInfo {
        public void set(
                int newOffset, int newSize, long newTimeUs, int newFlags) {
            offset = newOffset;
            size = newSize;
            presentationTimeUs = newTimeUs;
            flags = newFlags;
        }
        public int offset;
        public int size;
        public long presentationTimeUs;
        public int flags;
    };

    public static final int BUFFER_FLAG_SYNC_FRAME = 1;
    public static final int BUFFER_FLAG_KEY_FRAME = 1;
    public static final int BUFFER_FLAG_CODEC_CONFIG = 2;
    public static final int BUFFER_FLAG_END_OF_STREAM = 4;
    //private EventHandler mEventHandler;
    //private Callback mCallback;
    private static final int EVENT_CALLBACK = 1;
    private static final int EVENT_SET_CALLBACK = 2;
    private static final int CB_INPUT_AVAILABLE = 1;
    private static final int CB_OUTPUT_AVAILABLE = 2;
    private static final int CB_ERROR = 3;
    private static final int CB_OUTPUT_FORMAT_CHANGE = 4;

    public static final int INFO_TRY_AGAIN_LATER        = -1;
    public static final int INFO_OUTPUT_FORMAT_CHANGED  = -2;
    public static final int INFO_OUTPUT_BUFFERS_CHANGED = -3;

    public static AudDecApi createDecoderByType(String type)
            throws IOException {
        return new AudDecApi(type);
    }

    private AudDecApi(String name) {
        /*
        Looper looper;
        if ((looper = Looper.myLooper()) != null) {
            mEventHandler = new EventHandler(this, looper);
        } else if ((looper = Looper.getMainLooper()) != null) {
            mEventHandler = new EventHandler(this, looper);
        } else {
            mEventHandler = null;
        }
        mBufferLock = new Object();
        */
        native_setup(name);
    }

    public final void start() {
        native_start();
    }

    public final void stop() {
        native_stop();
    }

    public final int isInputFull()
    {
        return native_isInputFull();
    }
    public final int sendInputData(ByteBuffer buf, int numBytes, long timestampUs, int flags) {
        int res = native_sendInputData(buf, numBytes, timestampUs, flags);
        return res;
    }

    public final int isOutputEmpty()
    {
        return native_isOutputEmpty();
    }

    public final int getOutputData(
            ByteBuffer buf, int nBytes) {
        int res = native_getOutputData(buf, nBytes);
        return res;
    }

    public final int getFreqData(
            ByteBuffer buf, int nBytes) {
        int res = native_getFreqData(buf, nBytes);
        return res;
    }

    public final void release() {
        //freeAllTrackedBuffers(); // free buffers first
        native_release();
    }

    static {
        System.loadLibrary("AudDecApi");
        native_init();
    }

    private native final void native_start();
    private native final void native_stop();

    private static native final void native_init();
    private native final void native_setup(String name);

    private native final void native_reset();
    private native final void native_release();
    private native final int native_sendInputData(ByteBuffer buf, int numBytes, long timestampUs, int flags);
    private native final int native_isInputFull();
    private native final int native_getOutputData(ByteBuffer buf, int numBytes);
    private native final int native_getFreqData(ByteBuffer buf, int numBytes);
    private native final int native_isOutputEmpty();

    private long mNativeContext;
}
