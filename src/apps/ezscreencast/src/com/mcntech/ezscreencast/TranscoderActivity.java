package com.mcntech.ezscreencast;

import android.app.Activity;
import android.content.ContentResolver;
import android.content.Intent;
import android.media.MediaCodec;
import android.media.MediaCodec.BufferInfo;
import android.net.Uri;
import android.os.Bundle;
import android.os.ParcelFileDescriptor;
import android.os.SystemClock;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.ProgressBar;
import android.widget.Toast;

import net.ypresto.androidtranscoder.MediaTranscoder;
import net.ypresto.androidtranscoder.format.MediaFormatStrategyPresets;
import net.ypresto.androidtranscoder.engine.MediaTranscoderEngine;
import net.ypresto.androidtranscoder.engine.QueuedMuxer.SampleType;

import java.io.File;
import java.io.FileDescriptor;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.concurrent.Future;


public class TranscoderActivity extends Activity implements MediaTranscoderEngine.StreamIf {
    private static final String TAG = "TranscoderActivity";
    private static final int REQUEST_CODE_PICK = 1;
    private static final int PROGRESS_BAR_MAX = 1000;
    private Future<Void> mFuture;
    private long mStartPtsUs = 0;
    byte[] mSpsDdata;
    byte[] mPpsDdata;
    
    byte[] mAdtsData;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_transcoder);
        findViewById(R.id.select_video_button).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                startActivityForResult(new Intent(Intent.ACTION_GET_CONTENT).setType("video/*"), REQUEST_CODE_PICK);
            }
        });
        findViewById(R.id.cancel_button).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                mFuture.cancel(true);
            }
        });
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        switch (requestCode) {
            case REQUEST_CODE_PICK: {
                final File file;
                if (resultCode == RESULT_OK) {
                    try {
                        file = File.createTempFile("transcode_test", ".mp4", getExternalFilesDir(null));
                    } catch (IOException e) {
                        Log.e(TAG, "Failed to create temporary file.", e);
                        Toast.makeText(this, "Failed to create temporary file.", Toast.LENGTH_LONG).show();
                        return;
                    }
                    ContentResolver resolver = getContentResolver();
                    final ParcelFileDescriptor parcelFileDescriptor;
                    try {
                        parcelFileDescriptor = resolver.openFileDescriptor(data.getData(), "r");
                    } catch (FileNotFoundException e) {
                        Log.w("Could not open '" + data.getDataString() + "'", e);
                        Toast.makeText(TranscoderActivity.this, "File not found.", Toast.LENGTH_LONG).show();
                        return;
                    }
                    final FileDescriptor fileDescriptor = parcelFileDescriptor.getFileDescriptor();
                    final ProgressBar progressBar = (ProgressBar) findViewById(R.id.progress_bar);
                    progressBar.setMax(PROGRESS_BAR_MAX);
                    final long startTime = SystemClock.uptimeMillis();
                    MediaTranscoder.Listener listener = new MediaTranscoder.Listener() {
                        @Override
                        public void onTranscodeProgress(double progress) {
                            if (progress < 0) {
                                progressBar.setIndeterminate(true);
                            } else {
                                progressBar.setIndeterminate(false);
                                progressBar.setProgress((int) Math.round(progress * PROGRESS_BAR_MAX));
                            }
                        }

                        @Override
                        public void onTranscodeCompleted() {
                            Log.d(TAG, "transcoding took " + (SystemClock.uptimeMillis() - startTime) + "ms");
                            onTranscodeFinished(true, "transcoded file placed on " + file, parcelFileDescriptor);
                            startActivity(new Intent(Intent.ACTION_VIEW).setDataAndType(Uri.fromFile(file), "video/mp4"));
                        }

                        @Override
                        public void onTranscodeCanceled() {
                            onTranscodeFinished(false, "Transcoder canceled.", parcelFileDescriptor);
                        }

                        @Override
                        public void onTranscodeFailed(Exception exception) {
                            onTranscodeFinished(false, "Transcoder error occurred.", parcelFileDescriptor);
                        }
                    };
                    if(CodecModel.mSaveTranscodeFile) {
	                    Log.d(TAG, "transcoding into " + file);
	                    mFuture = MediaTranscoder.getInstance().transcodeVideo(fileDescriptor, file.getAbsolutePath(),
	                            MediaFormatStrategyPresets.createAndroidFollowInputStrategy(), listener, null);
                    } else {
	                    Log.d(TAG, "transcoding into " + file);
	                    mFuture = MediaTranscoder.getInstance().transcodeVideo(fileDescriptor, null,
	                            MediaFormatStrategyPresets.createAndroidFollowInputStrategy(), listener, this);                    	
                    }
                    switchButtonEnabled(true);
                }
                break;
            }
            default:
                super.onActivityResult(requestCode, resultCode, data);
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.transcoder, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();
        if (id == R.id.action_settings) {
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    private void onTranscodeFinished(boolean isSuccess, String toastMessage, ParcelFileDescriptor parcelFileDescriptor) {
        final ProgressBar progressBar = (ProgressBar) findViewById(R.id.progress_bar);
        progressBar.setIndeterminate(false);
        progressBar.setProgress(isSuccess ? PROGRESS_BAR_MAX : 0);
        switchButtonEnabled(false);
        Toast.makeText(TranscoderActivity.this, toastMessage, Toast.LENGTH_LONG).show();
        try {
            parcelFileDescriptor.close();
        } catch (IOException e) {
            Log.w("Error while closing", e);
        }
    }

    private void switchButtonEnabled(boolean isProgress) {
        findViewById(R.id.select_video_button).setEnabled(!isProgress);
        findViewById(R.id.cancel_button).setEnabled(isProgress);
    }

	@Override
	public void writeData(SampleType sampleType, ByteBuffer byteBuf,
			BufferInfo bufferInfo) {
		
		if(sampleType == SampleType.VIDEO) {
	    	byte[] vidBytes;
	    	long pts = 0;
	    	int prependLen = 0;
	    	int payloadLen = bufferInfo.size;
	        if(mStartPtsUs == 0)
	        	mStartPtsUs = bufferInfo.presentationTimeUs;
	        pts = (bufferInfo.presentationTimeUs - mStartPtsUs);
	
	        if((bufferInfo.flags & MediaCodec.BUFFER_FLAG_KEY_FRAME) != 0) {
	        	prependLen = mSpsDdata.length + mPpsDdata.length;
	        	vidBytes = new byte[prependLen + payloadLen];
	        	System.arraycopy(mSpsDdata, 0, vidBytes, 0, mSpsDdata.length);
	        	System.arraycopy(mPpsDdata, 0, vidBytes, mSpsDdata.length, mPpsDdata.length);
	        } else {
	        	vidBytes = new byte[payloadLen];	            	
	        }
	        // transfer bytes from this buffer into the given destination array
	        byteBuf.get(vidBytes, prependLen, payloadLen);
	        OnyxApi.sendVideoData("input0", vidBytes, prependLen + payloadLen, pts, bufferInfo.flags);
		} else if (sampleType == SampleType.AUDIO) {
        	byte[] audBytes;
        	long pts = 0;
        	int prependLen = 0;
        	int payloadLen = bufferInfo.size;

        	if(mStartPtsUs == 0)
            	mStartPtsUs = bufferInfo.presentationTimeUs;
        	
            pts = (bufferInfo.presentationTimeUs - mStartPtsUs);

            if((bufferInfo.flags & MediaCodec.BUFFER_FLAG_KEY_FRAME) != 0) {
            	prependLen = mAdtsData.length;
            	audBytes = new byte[prependLen + payloadLen];
            	System.arraycopy(mAdtsData, 0, audBytes, 0, mAdtsData.length);
            } else {
            	audBytes = new byte[payloadLen];	            	
            }
            // transfer bytes from this buffer into the given destination array
            byteBuf.get(audBytes, prependLen, payloadLen);
	        OnyxApi.sendAudioData("input0", audBytes, prependLen + payloadLen, pts, bufferInfo.flags);
		}
	}
}
