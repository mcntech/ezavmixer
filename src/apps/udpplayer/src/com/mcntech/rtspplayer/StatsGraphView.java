package com.mcntech.rtspplayer;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.util.Arrays;
import java.util.concurrent.Semaphore;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.opengl.GLU;
import android.util.AttributeSet;
import android.util.Log;

public class StatsGraphView extends GLSurfaceView implements GLSurfaceView.Renderer {
	Graph mGraph;
	Context mContext;
	SamplesBuffer[] samplesBuffer;
	float mRatio;
	
	public void Init(Context context, SamplesBuffer[] samplesBuffer){
	    mContext = context;
	    mGraph = new Graph(context,samplesBuffer);
	}
	

	public StatsGraphView(Context context, SamplesBuffer[] samplesBuffer) {
	    super(context);
	    Init(context, samplesBuffer);
	}
	public void EnableRenderer()
	{
	    setRenderer(this);
	}
/*
	public StatsGraphView(Context context, AttributeSet attribs) {
	    super(context, attribs);

	}
*/	
	@Override
	public void onPause(){
	    super.onPause();
	}
	
	@Override
	public void onResume(){
	    super.onResume();
	}

	void updateData(long ts)
	{
		mGraph.update(ts);				
	}
	
	// Rendering functions	
		@Override
		public void onDrawFrame(GL10 gl) {
			gl.glDisable(GL10.GL_DITHER);
			gl.glClear(GL10.GL_COLOR_BUFFER_BIT | GL10.GL_DEPTH_BUFFER_BIT);
			gl.glMatrixMode(GL10.GL_MODELVIEW);
			gl.glLoadIdentity();
			GLU.gluLookAt(gl, 0, 0, -3.01f, 0f, 0f, 0f, 0f, 1.0f, 0.0f);
			gl.glEnableClientState(GL10.GL_VERTEX_ARRAY);		
			mGraph.draw(gl);
		}
		
		@Override
		public void onSurfaceChanged(GL10 gl, int w, int h) {
			gl.glViewport(0, 0, w, h);
			mRatio = (float) w / h;
			gl.glMatrixMode(GL10.GL_PROJECTION);
			gl.glLoadIdentity();
			gl.glFrustumf(mRatio, -mRatio, -1, 1, 3, 7);
			
			recalculateScreen();
		}


		@Override
		public void onSurfaceCreated(GL10 gl, EGLConfig config) {
			gl.glDisable(GL10.GL_DITHER);
			gl.glHint(GL10.GL_PERSPECTIVE_CORRECTION_HINT,GL10.GL_FASTEST);
			gl.glClearColor(0, 0, 0, 0);
			gl.glShadeModel(GL10.GL_SMOOTH);
			gl.glEnable(GL10.GL_DEPTH_TEST);
			
		}


		private void recalculateScreen() {
			mGraph.recalculate(mRatio);
		}
	
	public class Grid {
		
		float mStartX;
		float mStartY;
		float mWidth;
		float mHeight;
		
		
		private int mDivisionsX;
		private int mDivisionsY;;
		
		float[] mCoords; //Regular line coordinates
		float[] mCoordsPrimary;//Primary line coordinates
		float[] mCoordsMiddle;//Axes coordinates
		
		private FloatBuffer mFVertexBuffer;//Regular lines vertex buffer
		private FloatBuffer mFVertexBufferPrimary;//Primary lines vertex buffer
		private FloatBuffer mMiddleVertexBuffer;//Axes vertex buffer
		private FloatBuffer mFrameVertexBuffer;//Frame vertex buffer
		
		
		public void draw(GL10 gl) {
			gl.glColor4f(0.16f, 0.16f, 0.8f, 1f);//Dark Blue
			
			
			gl.glLineWidth(0.1f);//Thin lines width
			
			gl.glTranslatef(mStartX, mStartY, 0.01f);//Move below graph level
			
			gl.glEnableClientState (GL10.GL_VERTEX_ARRAY);				
			gl.glVertexPointer(2, GL10.GL_FLOAT, 0, mFVertexBuffer);//Select regular lines
			gl.glDrawArrays(GL10.GL_LINES, 0, (mDivisionsX+mDivisionsY)*2);//Draw regular lines
									
			int numberOfPrimaryPointsX = (mDivisionsX+5)/5;
			int numberOfPrimaryPointsY = (mDivisionsY+5)/5+1;
					
			gl.glLineWidth(2f);
			
			gl.glEnableClientState (GL10.GL_VERTEX_ARRAY);//Draw Primary 			
			gl.glVertexPointer(2, GL10.GL_FLOAT, 0, mFVertexBufferPrimary);
			gl.glDrawArrays(GL10.GL_LINES, 0, (numberOfPrimaryPointsX+numberOfPrimaryPointsY)*2);
			
			gl.glLineWidth(4f);//Draw axes
			gl.glVertexPointer(2, GL10.GL_FLOAT, 0, mMiddleVertexBuffer);
			gl.glDrawArrays(GL10.GL_LINE_STRIP, 0, 2);
					
			gl.glColor4f(0.5f, 0.5f, 0.5f, 1f);//Gray color
			
			gl.glLineWidth(3.f);//Draw the frame	
			gl.glVertexPointer(2, GL10.GL_FLOAT, 0, mFrameVertexBuffer);
			gl.glDrawArrays(GL10.GL_LINE_STRIP, 0, 5);
			gl.glDisableClientState (GL10.GL_VERTEX_ARRAY);
					
			gl.glTranslatef(-mStartX, -mStartY, -0.01f);
			
		
		}

		/**
		 * Regenerates the grid when graph display changes
		 * 
		 * @param startX
		 * @param startY
		 * @param graphWidth
		 * @param graphHeight
		 * @param divisionsX
		 * @param divisionsY
		 */
		public void setBounds(float startX, float startY, float graphWidth,
				float graphHeight, int divisionsX, int divisionsY) {
			mStartX = startX;
			mStartY = startY;
			mWidth = graphWidth;
			mHeight = graphHeight;
					
			startX=startY=0;
					
			mDivisionsX = divisionsX;
			mDivisionsY = divisionsY;
			
			
			//Regular lines
			mCoords = new float[(mDivisionsX*4+mDivisionsY+2)*4];
			
			for(int i=0; i<mDivisionsX;i++)
			{
				mCoords[i*4] = startX+(graphWidth*i)/mDivisionsX;
				mCoords[i*4+1] = startY;			
				mCoords[i*4+2] = startX+(graphWidth*i)/mDivisionsX;
				mCoords[i*4+3] = startY-graphHeight;
			}
			
			
			//X Axis
			mCoordsMiddle = new float[4];
			
			mCoordsMiddle[0] = startX;
			mCoordsMiddle[1] = startY - mHeight/2;
			mCoordsMiddle[2] = startX + mWidth;
			mCoordsMiddle[3] = startY - mHeight/2;
			
			
			//Primary lines
			int numberOfPrimaryPointsX = (mDivisionsX+5)/5;
			int numberOfPrimaryPointsY = (mDivisionsY+5)/5+1;
			mCoordsPrimary = new float[(numberOfPrimaryPointsX+numberOfPrimaryPointsY)*4];
					
			for(int i=0; i<numberOfPrimaryPointsX;i++)
			{
				mCoordsPrimary[i*4] = startX+(graphWidth*i*5)/mDivisionsX;
				mCoordsPrimary[i*4+1] = startY;			
				mCoordsPrimary[i*4+2] = startX+(graphWidth*i*5)/mDivisionsX;
				mCoordsPrimary[i*4+3] = startY-graphHeight;
			}

			
					
			float[] frameCoords =  new float[10];
			
			frameCoords[0]=startX;
			frameCoords[1]=startY;
			frameCoords[2]=startX+graphWidth;
			frameCoords[3]=startY;
			frameCoords[4]=startX+graphWidth;;
			frameCoords[5]=startY-graphHeight;
			frameCoords[6]=startX;
			frameCoords[7]=startY-graphHeight;
			frameCoords[8]=startX;
			frameCoords[9]=startY;
					
			for(int i=0; i<(mDivisionsY/2+1);i++)
			{
				mCoords[mDivisionsX*4+i*4] = startX;
				mCoords[mDivisionsX*4+i*4+1] = startY-graphHeight/2+(graphHeight*i)/(mDivisionsY);			
				mCoords[mDivisionsX*4+i*4+2] = startX+graphWidth;
				mCoords[mDivisionsX*4+i*4+3] = startY-graphHeight/2+(graphHeight*i)/(mDivisionsY);;			
			}
			
			int secondaryOffset = (mDivisionsY/2+1)*4;
			
			for(int i=0; i<(mDivisionsY/2+1);i++)
			{
				mCoords[secondaryOffset + mDivisionsX*4+i*4] = startX;
				mCoords[secondaryOffset + mDivisionsX*4+i*4+1] = startY-graphHeight/2-(graphHeight*(i+1))/mDivisionsY;			
				mCoords[secondaryOffset + mDivisionsX*4+i*4+2] = startX+graphWidth;
				mCoords[secondaryOffset + mDivisionsX*4+i*4+3] = startY-graphHeight/2-(graphHeight*(i+1))/mDivisionsY;;			
			}
			
			int primaryArrayOffset= numberOfPrimaryPointsX*4;
			
			for(int i=0; i<(numberOfPrimaryPointsY/2+1);i++)
			{
				mCoordsPrimary[primaryArrayOffset+i*4] = startX;
				mCoordsPrimary[primaryArrayOffset+i*4+1] = startY-graphHeight/2+(graphHeight*i*5)/mDivisionsY;			
				mCoordsPrimary[primaryArrayOffset+i*4+2] = startX+graphWidth;
				mCoordsPrimary[primaryArrayOffset+i*4+3] = startY-graphHeight/2+(graphHeight*i*5)/mDivisionsY;						
			}
			
			primaryArrayOffset=(numberOfPrimaryPointsX+numberOfPrimaryPointsY/2)*4;
			
			for(int i=0; i<(numberOfPrimaryPointsY/2);i++)
			{
				mCoordsPrimary[primaryArrayOffset+i*4] = startX;
				mCoordsPrimary[primaryArrayOffset+i*4+1] = startY-graphHeight/2-(graphHeight*i*5)/mDivisionsY;			
				mCoordsPrimary[primaryArrayOffset+i*4+2] = startX+graphWidth;
				mCoordsPrimary[primaryArrayOffset+i*4+3] = startY-graphHeight/2-(graphHeight*i*5)/mDivisionsY;;			
			}
			
			
			ByteBuffer vbb = ByteBuffer.allocateDirect((mDivisionsX+mDivisionsY+2)*2 * 2 * 4);
			vbb.order(ByteOrder.nativeOrder());
			mFVertexBuffer = vbb.asFloatBuffer();
			
			ByteBuffer vbbPrimary = ByteBuffer.allocateDirect((numberOfPrimaryPointsX+numberOfPrimaryPointsY)*2 * 2 * 4);
			vbbPrimary.order(ByteOrder.nativeOrder());
			mFVertexBufferPrimary = vbbPrimary.asFloatBuffer();
			
			
			ByteBuffer vbbFrame = ByteBuffer.allocateDirect(10 * 2 * 4);
			vbbFrame.order(ByteOrder.nativeOrder());
			mFrameVertexBuffer = vbbFrame.asFloatBuffer();
			
			ByteBuffer vbbMiddle = ByteBuffer.allocateDirect(4 * 2 * 4);
			vbbMiddle.order(ByteOrder.nativeOrder());
			mMiddleVertexBuffer = vbbMiddle.asFloatBuffer();
			
					
			for (int i = 0; i < (mDivisionsX+mDivisionsY)*2; i++) {
				for(int j = 0; j < 2; j++) {
					mFVertexBuffer.put(mCoords[i*2+j]);
				}
			}
			
			int loopValue = (numberOfPrimaryPointsX+numberOfPrimaryPointsY)*2;
			
			for (int i = 0; i < loopValue; i++) {
				mFVertexBufferPrimary.put(mCoordsPrimary[i*2]);
				mFVertexBufferPrimary.put(mCoordsPrimary[i*2+1]);
				}
		
			
			for(int i=0;i<10;i++)
			{
				mFrameVertexBuffer.put(frameCoords[i]);
			}
			
			for(int i=0;i<4;i++)
			{
				mMiddleVertexBuffer.put(mCoordsMiddle[i]);
			}
			
			mFVertexBuffer.position(0);
			mFrameVertexBuffer.position(0);
			mFVertexBufferPrimary.position(0);
			mMiddleVertexBuffer.position(0);
		}

	}
	
	public class Graph {

		private static final int SAMPLES_IN_SCREEN = 1000;
		private static final int MAX_GRAPHS = 5;
		SamplesBuffer mSamplesBuffer[];
		Grid mGrid;
		Context mContext;
		GraphDataSource mDataSource[];
		ScreenBuffer mScreenBuffer[];

		class GraphColor
		{
			int R;
			int G;
			int B;
			
			GraphColor(int inR,int inG,int inB)
			{
				R = inR;
				G = inG;
				B = inB;
			}
		}

		GraphColor graphColor[]=new GraphColor[MAX_GRAPHS];
		
		void setColors()
		{
			graphColor[0]=new GraphColor(255, 0, 0);
			graphColor[1]=new GraphColor(0, 255, 0);
			graphColor[2]=new GraphColor(0, 0, 255);
			graphColor[3]=new GraphColor(255, 255, 0);
			graphColor[4]=new GraphColor(0, 255, 255);
		}
		

		public Graph(Context context, SamplesBuffer[] samplesBuffer) {
		
			//Set the colors array
			setColors();
			
			//Save the sample buffers
			mSamplesBuffer = samplesBuffer;

			mContext = context;

			//Create the grid
			mGrid = new Grid();

			//Create the data sources
			mDataSource = new GraphDataSource[samplesBuffer.length];
			
			for(int i=0;i<mDataSource.length;i++)
			{
				mDataSource[i] = new GraphDataSource(samplesBuffer[i]);
			}
		}

		public void update(long ts) {
			for(int i=0;i<5;i++)
			{
				mDataSource[i].updateScreenBuffer(ts, mScreenBuffer[i]);
			}
		}

		public void draw(GL10 gl) {
			mGrid.draw(gl);	

			gl.glEnableClientState (GL10.GL_VERTEX_ARRAY);
			gl.glLineWidth(2f);

			for(int i=0;i<mScreenBuffer.length;i++)
			{
				mScreenBuffer[i].setColor(gl);
				mScreenBuffer[i].drawScreenBuffer(gl);
			}

			gl.glDisableClientState (GL10.GL_VERTEX_ARRAY);
		}


		float mGraphWidth;
		float mGraphHeight;

		public void recalculate(float ratio) {
			mGraphWidth = 2.f*ratio;
			mGraphHeight = 2.0f;		

			mGrid.setBounds(-ratio, 1f,mGraphWidth, mGraphHeight,20,20);

			mScreenBuffer = new ScreenBuffer[mDataSource.length];
			
			for(int i=0;i<mScreenBuffer.length;i++)
			{
				mScreenBuffer[i] =  new PointScreenBuffer(SAMPLES_IN_SCREEN,-ratio, 1f, mGraphWidth, mGraphHeight);

				mScreenBuffer[i].setRGB(graphColor[i].R,graphColor[i].G,graphColor[i].B);
			}

		}						

	} // Graph
	public static class SamplesBuffer {

		private static final String TAG = "SamplesBuffer";

		short [] mSamples;	

		int mBufferStartIndex=0;	
		int mBufferSize;

		long mStartTimeStamp = Long.MAX_VALUE;

		boolean mCyclic = true;

		long mLatestSample = 0;
		
		Semaphore mSemaphore;
		
		
		//For testing purposes - checks buffer behavior in several scenarios 
/*		static void simpleTest()
		{
			SamplesBuffer sb = new SamplesBuffer(10,true);
			
			sb.setTimestamp(0);
			
			for(int i=0;i<10;i++)
			{
				sb.addSample((short)i, (long)i);
			}
			
			sb.addSample((short)10, (long)10);
						
			sb.addSample((short)11, (long)11);
			
			sb.printArray();
			
			sb.addSample((short)18, (long)18);				
			
			sb.printArray();
			
			sb.addSample((short)28, (long)28);
			sb.addSample((short)29, (long)29);
			
			for(int i=30;i<60;i++)
			{
				sb.addSample((short)i, (long)i);
			}
			
			sb.printArray();
			
			short atIndex4 = sb.getSampleAtTime(52);
			
			System.out.println("Value at time 52:"+atIndex4);
			
			sb.addSample((short)49, 49);
			sb.addSample((short)48, 48);
			sb.addSample((short)500, 50);
			sb.printArray();
			
			sb.addSample((short)28, 28);
			
			sb.printArray();
			
			for(int i=30;i<40;i++)
			{
				sb.addSample((short)i, (long)i);
			}
			
			sb.printArray();
			
			sb.addSample((short)27, 27);
			
			sb.printArray();
		}
*/
		//Debug function - used in testing
		void printArray()
		{
			System.out.println("Buffer Index Start:"+mBufferStartIndex+" Start TS:"+mStartTimeStamp);
			
			for(int i=0;i<mBufferSize;i++)
			{
				System.out.println("Sample("+i+"):"+mSamples[i]);
			}
		}
		
		public SamplesBuffer(int bufferSize, boolean cyclic) {

			Log.i("SamplesBuffer","Constructor call");
			mSamples = new short[bufferSize];	
			
			int count =0;	
			
			mBufferSize = bufferSize;
			mCyclic = cyclic;
			
			mSemaphore = new Semaphore(1, true);
			
			invalidateWholeBuffer();
		}

		void setTimestamp(long timeStamp)
		{
			mStartTimeStamp = timeStamp;
		}

		int getIndexOffsetFromTimestamp(long timeStamp)
		{
			return (int)((timeStamp-mStartTimeStamp+1)/2);
		}

		/**
		 * @param timeStamp
		 * @return
		 */
		short getSampleAtTime(long timeStamp)
		{		
			try {
				mSemaphore.acquire();
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			
			int index = getIndexOffsetFromTimestamp(timeStamp);
			
			short returnValue = Short.MAX_VALUE;

			//Just update sample - no need to move the buffer!
			if((timeStamp>=mStartTimeStamp) && (timeStamp<=(mStartTimeStamp+mBufferSize*2)))
			{
				//Position is relative, must add to it buffer start index
				int position = wrap(mBufferStartIndex+index);

				returnValue =  mSamples[position];
			}

			mSemaphore.release();
			
			return returnValue;
		}

		protected int wrap(int i) {
			if(i>=mBufferSize)
			{
				return i-mBufferSize;
			}
			else
			{
				if(i<0)
				{
					i = mBufferSize+i;
				}
			}
			
			return i;
		}
		
		public long getLatestTimeStamp()
		{
			return mLatestSample;
		}
		
		public int addSample(short sampleValue, long timeStamp, boolean isHeartBeat)
		{
			return addSample(sampleValue, timeStamp);
		}

		int lastIndex = 0;
		
		public int addSample(short sampleValue, long timeStamp)
		{			
			try {
				mSemaphore.acquire();
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			
			
			if(mStartTimeStamp==Long.MAX_VALUE)
			{
				Log.i("ReadEcgFromFile","Setting ts:"+timeStamp);
				mStartTimeStamp = timeStamp;
			}
			
			if(timeStamp>mLatestSample)
			{
				//Log.i(TAG,"Setting latest timestamp:"+timeStamp);
				mLatestSample = timeStamp;
			}
			
			int index = getIndexOffsetFromTimestamp(timeStamp);
			
			if(lastIndex!=index-1)
			{
			//	Log.i("C0", "JumpedIndex! index:"+index+ " Last index:"+lastIndex);
			}
			
			lastIndex = index;
			
			int position =0;

			//If index falls inside current buffer -  Simply write it in the appropriate position
			//Just update sample - no need to move the buffer!
			if((index<mBufferSize) && (index>0))
			{
				//Position is relative, must add to it buffer start index
				position = wrap(mBufferStartIndex+index);

				mSamples[position] = sampleValue;
			}		
			//If index is in the future
			else if(index>=mBufferSize)
			{
				int delta = index+1-mBufferSize;

				//Now invalidate everything from write position to start position(If needed)

				//If have to move less than buffer size forward...			
				if(delta<mBufferSize)
				{	
					//Save the old last position of the buffer
					int oldStart = mBufferStartIndex;

					//Move the start pointer so that it points to write position-buffer size
					mBufferStartIndex=wrap(mBufferStartIndex+delta);
					
					mStartTimeStamp += delta*2;

					//Find the new position after buffer is moved. Sample is now last in buffer
					position = wrap(mBufferStartIndex+(mBufferSize-1));								

					//Fill area between old position and new position with uninitialized values.
					while(oldStart!=mBufferStartIndex)
					{
						mSamples[oldStart]=Short.MAX_VALUE;

						oldStart=wrap(oldStart+1);
					}

				}
				//Moved too much - have to erase the whole buffer
				else
				{
					if(delta>=mBufferSize)
					{
						invalidateWholeBuffer();
						mBufferStartIndex = 0;
						position  = 0;
					//	Log.i("ReadEcgFromFile","Setting ts:"+timeStamp);
						mStartTimeStamp = timeStamp;
					}
				}

			}
			else if(index<0) //If index is in the past
			{

				//Buffer samples still relevant
				if(index>=(-mBufferSize))
				{

					int oldStartPosition = mBufferStartIndex;

					//Move the start position to the write position.
					mBufferStartIndex = wrap(mBufferStartIndex+index);
					

					//Now invalidate everything from new start position to old start position
					while(mBufferStartIndex!=oldStartPosition)
					{
						mSamples[wrap(oldStartPosition+mBufferSize-1)]=Short.MAX_VALUE;
						oldStartPosition = wrap(oldStartPosition-1);
					}

					position = mBufferStartIndex;
					
					mStartTimeStamp += index*2;
					
					//Log.i("ReadEcgFromFile","Setting ts:"+mStartTimeStamp);
				}
				//Moved too much - erase all buffer
				else
				{
					invalidateWholeBuffer();
					mBufferStartIndex = 0;
					position  = 0;
					
					
					mStartTimeStamp = timeStamp;
					//Log.i("ReadEcgFromFile","Setting ts:"+mStartTimeStamp);
				}
			}
			mSamples[position] = sampleValue;
			
			mSemaphore.release();
			
			
			return position;
		}


		private void invalidateWholeBuffer() {
			//Log.i("OFFL","Invalidate buffer");
			Arrays.fill(mSamples, Short.MAX_VALUE);		
		}

		public long getStartTs() {
			return mStartTimeStamp;
		}


		public long lastSampleTime() {
			
			if(mLatestSample>(mStartTimeStamp+mBufferSize*2))
			{
				mLatestSample = mStartTimeStamp+mBufferSize*2;
			}
			
			return mLatestSample;
		}

		public void reset() {
		//	Log.i("OFFL","Reset");
			try {
				mSemaphore.acquire();
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
					
			mBufferStartIndex = 0;
			mStartTimeStamp = Long.MAX_VALUE;
			invalidateWholeBuffer();
			
			mSemaphore.release();
		}

		public int getBufferSize() {
			return mBufferSize;
		}
		
		long getTimeAtIndex(int index)
		{
			int delta = index - mBufferStartIndex;
			
			if(delta<0)
			{
				delta = mBufferSize+delta;
			}
			
			return mStartTimeStamp + (long)delta*2; 
			
		}

		public long getStartTsWithOffset() {		
			return 0;
		}
		
	}
	
	
	
	public static class PointScreenBuffer extends ScreenBuffer {

		PointScreenBuffer(int samplesInScreen, float startX, float startY,
				float width, float height) {
			super(samplesInScreen, startX, startY, width, height);
			// TODO Auto-generated constructor stub
		}

		@Override
		void putSample(float sample)
		{
			mFVertexBuffer.put(mBufferWritePointer*2+1,sample);
		}
		
		@Override
		int getAllocation(int samplesInScreen)
		{
			return mSamplesInScreen*2 * 2 * 4;
		}
		
		@Override
		void fillVertexArrayX()
		{
			//Fill the data buffer with 0 value samples.
			for(int i=0;i<mSamplesInScreen;i++)
			{
				float value = (float)(mWidth*(float)i)/(float)mSamplesInScreen;
				mFVertexBuffer.put(value);
				
				mFVertexBuffer.put(0);			
			}
		}
		
		
		@Override
		void drawScreenBuffer(GL10 gl) {
			{		
				int readPointer = mBufferReadPointer;

				float xOffset = mStartX-mFVertexBuffer.get(readPointer*2);						


				gl.glPointSize(3f);

				//Draw Right Side
				gl.glTranslatef(xOffset, mStartY-mHeight/2.f, 0);

				mFVertexBuffer.position(0);				
				
				mFVertexBuffer.mark();

				gl.glVertexPointer(2, GL10.GL_FLOAT, 0, mFVertexBuffer);				

				gl.glDrawArrays(GL10.GL_POINTS, readPointer, getNumberOfSamplesLeft(readPointer));

				gl.glTranslatef(-xOffset, -mStartY+mHeight/2.0f, 0);
				
				mFVertexBuffer.reset();

				//If there is a left side - draw left side
				if(getNumberOfSamplesRight(readPointer)!=0)
				{
					xOffset = mStartX+mWidth-mFVertexBuffer.get(readPointer*2);

					gl.glTranslatef(xOffset, mStartY-mHeight/2.f, 0);

					gl.glDrawArrays(GL10.GL_POINTS, 0, getNumberOfSamplesRight(readPointer));

					gl.glTranslatef(-xOffset, -mStartY+mHeight/2.f, 0);

				}

			}
		}

	}
	abstract public static class ScreenBuffer {

		private static final String TAG = "ScreenBuffer";
		int mBufferWritePointer;
		int mBufferReadPointer;
		int mSamplesInScreen;
		float mStartX;
		float mStartY;
		float mHeight;
		float mWidth;
		float mRComponent = 0;
		float mGComponent = 1;
		float mBComponent = 0;
		protected FloatBuffer mFVertexBuffer;
		long mLastTimeStamp;
					
		boolean mBufferFull = false;
		
		boolean mCyclic = true;
		
				
		void setCyclic(boolean cyclic)
		{
			mCyclic = cyclic;
		}
		

		abstract void fillVertexArrayX();
		abstract int getAllocation(int samplesInScreen);
		
		ScreenBuffer(int samplesInScreen, float startX, float startY, float width, float height) 
		{
			mSamplesInScreen = samplesInScreen;
			mStartX = startX;
			mStartY = startY;
			mHeight = height;
			mWidth  = width;
			
			
			ByteBuffer vbb = ByteBuffer.allocateDirect(getAllocation(mSamplesInScreen));
			vbb.order(ByteOrder.nativeOrder());
			mFVertexBuffer = vbb.asFloatBuffer();
			
			
			fillVertexArrayX();					
		}
		
			
		private void advanceReadPointer()
		{
			mBufferReadPointer++;
			if( mBufferReadPointer == getBufferSize())
			{
				mBufferReadPointer = 0;
			}
		}
		
		private int getBufferSize() {
			return (mSamplesInScreen);
		}

		private void advanceWritePointer()
		{
			mBufferWritePointer++;
			if( mBufferWritePointer == getBufferSize() )
			{
				mBufferWritePointer = 0;
			}
			
			//If no place left in buffer we just ran over a sample - in this case: Move the read pointer.
			if( mBufferWritePointer == mBufferReadPointer )
			{
				mBufferFull = true;
			}
		}
		
		synchronized boolean addSample(float sample)
		{		
			float putValue = sample;
			
			if(putValue>(mHeight/2))
			{
				putValue = mHeight/2;
			}
			
			if(putValue<(-mHeight/2))
			{
				putValue = -mHeight/2;
			}
			
			//Log.i(TAG,"height:"+mHeight+" sample:"+sample);
			
			if(mBufferFull)
			{
				if(mCyclic==true)
				{
					advanceReadPointer();
				}
				else 
				{
					return false;
				}
			}
			putSample(putValue);
									
			advanceWritePointer();
			
			return true;
		}
		
		abstract void putSample(float sample);
			
		int getNumberOfSamplesLeft(int readPointer)
		{
			return mSamplesInScreen - readPointer;
		}
		
		int getNumberOfSamplesRight(int readPointer)
		{
			return readPointer;
		}
		

		abstract void drawScreenBuffer(GL10 gl);

		
		int getScreenBufferSize()
		{
			return mSamplesInScreen;
		}

		public void copyScreenBuffer(ScreenBuffer screenBuffer) {
			for(int i=0;i<mSamplesInScreen;i++)
			{
				float sample = mFVertexBuffer.get(((mBufferReadPointer+i)%mSamplesInScreen)*2+1);
				screenBuffer.addSample(sample);
			}
			
		}

		public void reset() {
			mBufferReadPointer = 0;
			mBufferWritePointer = 0;
			mBufferFull = false;
			
			mLastTimeStamp = Long.MAX_VALUE;
			
		}

		public long getSamplesInScreen() {
			return mSamplesInScreen;
		}

		public int getNumberOfValidSamples() {
			// TODO Auto-generated method stub
			return 0;
		}

		void setRGB(float r, float g, float b)
		{
			mRComponent = r;
			mGComponent = g;
			mBComponent = b;
		}

		public void setColor(GL10 gl) {
			gl.glColor4f(mRComponent, mGComponent, mBComponent, 1);
		}
		
		public void setLastTimeStamp(long ts)
		{
			mLastTimeStamp = ts;
		}
		
		public long getLastTimeStamp() {
			// TODO Auto-generated method stub
			return mLastTimeStamp;
		}

	}
	
	
	
	public class GraphDataSource {

		private static final long SCREEN_BUFFER_TIME_RESOLUTION = 2;//500 samples per second

		private static final float NORMALIZATION_FACTOR = 90.0f;

		float mScreenPart = 1.0f;
		
		float mMaxSample = Float.MIN_VALUE;
		float mMinSample = Float.MAX_VALUE;
		
		/**
		 * Load samples to screen buffer from a samples buffer in offline mode.
		 * @param fromTimeStamp
		 * @param upToTimeStamp
		 * @param screenBuffer
		 */
		void loadSamplesToScreenBuffer(long fromTimeStamp, long upToTimeStamp, ScreenBuffer screenBuffer)
		{		
			screenBuffer.reset();

			updateScreenBuffer(upToTimeStamp, screenBuffer);		
		}

		/**
		 * Update a screen buffer with data
		 * @param upToTimeStamp
		 * @param screenBuffer
		 * @param online
		 * @return
		 */
		boolean updateScreenBuffer(long upToTimeStamp, ScreenBuffer screenBuffer)
		{		
			int count = 0;		
			
			//Safety measure
			if(screenBuffer == null)
			{
				return false;
			}
			
			
			//Get start time of buffer
			long startTime = screenBuffer.getLastTimeStamp();

			if(startTime==Long.MAX_VALUE)
			{
				startTime=0;
			}
		
			
			float lastNormalizedSample=0;
			float normalizedSample;
			
			//Add sample by sample based on time
			while(startTime<=upToTimeStamp)
			{								
				short sample = mSamplesBuffer.getSampleAtTime(startTime);			
						
				if(sample!=Short.MAX_VALUE)
				{											
					normalizedSample = ((float)sample)/NORMALIZATION_FACTOR;
					
					//Remember last sample - buffer may not contain an entry in the next read position
					lastNormalizedSample = normalizedSample;
				}
				else
				{				
					normalizedSample = lastNormalizedSample;
				}

				//Add the sample to screen buffer
				screenBuffer.addSample(normalizedSample);
				
				//Move to next sample
				startTime += SCREEN_BUFFER_TIME_RESOLUTION;
				
				screenBuffer.setLastTimeStamp(startTime);
			}		
			
									
			return true;
		}


		SamplesBuffer mSamplesBuffer;

		GraphDataSource(SamplesBuffer samplesBuffer)
		{
			mSamplesBuffer = samplesBuffer;
		}
		
		public long getEndTs()
		{		
			return mSamplesBuffer.lastSampleTime();
		}

		public long getStartTs() {
			return mSamplesBuffer.getStartTs();				
		}

	}
}