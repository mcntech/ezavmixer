#ifndef FCDST_FRAME_BUFFER_H
#define FCDST_FRAME_BUFFER_H

#include <vector>
#include <pthread.h>

#define AVCAP_CODEC_TYPE_PCM       1
#define AVCAP_CODEC_TYPE_H264      64
#define MAX_AV_BUFFER_FRAMES       30
#define UNPACKED_BUFFER_SIZE 10000

typedef struct _AVCAP_FRAME_T
{
	unsigned char  *data;		/**< audio or video data pointer */
	int            size;		/**< size of audio or video data */
	int            codec;		/**< Codec Type. AVCAP_CODEC_TYPE_PCM or AVCAP_CODEC_TYPE_H264 */
	int            width;		/**< Video with. Not implemented! */
	int            height;		/**< Video Height. Not implemented! */
	int            sample_rate;	/**< audio sample rate. Not implemented! */
	int            sample_size;	/**< audio sample size in bytes. Not implemented! */
	int            channels;	/**< Number of audio channels. Not implemented! */
	int64_t        pts;			/**< Audio or video frame PTS in 90kHz units*/
} AVCAP_FRAME_T;

///////////////////////////////////////////////////////////////////////////////
// CBufferSrcManager

class CBufferSrcManagerV2
{
public:
	pthread_cond_t m_spaceAvailCond;
	pthread_cond_t m_dataAvailCond;
	pthread_mutex_t m_dataMutex;

private:
	unsigned char * sourceBuffer;
	int			sourceBufferSize;
	int			WriteOffset,ReadOffset;
	int			m_audioBytesAvailable, m_bytesRead;
	int			bytesWritten;

public:
    bool    mRunning;
    CBufferSrcManagerV2(void);
	~CBufferSrcManagerV2(void);
	ESTATUS init(int size);
	ESTATUS initAudioQueue(int size){return init(size);}
	void Start();
	void Stop();

	ESTATUS Render(unsigned char * samples,int size); //return an error code
	int GetNextPacket(void * outputbuffer,int readsize);
	virtual void resetBufferOffset(void);

	inline int
	usedSpace(void)
	{
		return (WriteOffset + sourceBufferSize - ReadOffset) % sourceBufferSize;
	}

	inline int
	freeSpace(void)
	{
		int freebytes = 0;
		freebytes = sourceBufferSize - usedSpace() - 1;
		return freebytes;
	}

};

class CAvBufferSrcManager: public CBufferSrcManagerV2 //CBufferSrcManager
{
	public:
	CAvBufferSrcManager()
	{
		m_nWr = m_nRd = 0;
		m_nMax = MAX_AV_BUFFER_FRAMES;
		m_crntAvFrame = NULL;
	}

	int WriteFrame(unsigned char * samples,int size, long lPts, int nFlags, int nCodec);
	int GetFrame(AVCAP_FRAME_T *avFrame);
	int WriteFrameBegin(unsigned char * samples,int size, int64_t llPts, int nFlags, int nCodec);
	int WriteFrameMiddle(unsigned char * samples,int size, int nCodec);
	int WriteFrameEnd(unsigned char * samples,int size, int nCodec);
	int IsFrameStarted() {return m_crntAvFrame != NULL;}
	void resetBufferOffset() {CBufferSrcManagerV2::resetBufferOffset(); m_nWr = m_nRd = 0;m_crntAvFrame=NULL;}
	int GetNumFramesAvail() { return (m_nWr + m_nMax - m_nRd) % m_nMax;}
private:
	AVCAP_FRAME_T m_avcapFrameList[MAX_AV_BUFFER_FRAMES]; // TODO implement list
	int m_nWr;
	int m_nRd;
	int m_nMax;
	AVCAP_FRAME_T *m_crntAvFrame;
};


#endif // FCDST_FRAME_BUFFER_H
