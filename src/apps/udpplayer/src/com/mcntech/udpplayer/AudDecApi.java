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
        return new AudDecApi(type, true /* nameIsType */, false /* encoder */);
    }

    private AudDecApi(
            String name, boolean nameIsType, boolean encoder) {
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
        native_setup(name, nameIsType, encoder);
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

    public final void release() {
        //freeAllTrackedBuffers(); // free buffers first
        native_release();
    }

    public void configure(
            MediaFormat format) {
        // TODO
        //native_configure(keys, values, surface, crypto, flags);
        //native_configure(keys, values);
    }


    public ByteBuffer getInputBuffer(int index) {
        ByteBuffer newBuffer = getBuffer(true /* input */, index);
        return newBuffer;
    }

    public ByteBuffer getOutputBuffer(int index) {
        ByteBuffer newBuffer = getBuffer(false /* input */, index);
        return newBuffer;
    }

    static {
        System.loadLibrary("AudDecApi");
    }

    private native final void native_configure(
            String[] keys,Object[] values);
    private native final void native_start();
    private native final void native_stop();

    private native final void native_setup(
            String name, boolean nameIsType, boolean encoder);

    private native final void native_reset();
    private native final void native_release();
    private native final int native_sendInputData(ByteBuffer buf, int numBytes, long timestampUs, int flags);
    private native final int native_isInputFull();
    private native final int native_getOutputData(ByteBuffer buf, int numBytes);
    private native final int native_isOutputEmpty();

    private native final void releaseOutputBuffer(
            int index, boolean render, boolean updatePTS, long timeNs);
    private native final ByteBuffer getBuffer(boolean input, int index);
}
