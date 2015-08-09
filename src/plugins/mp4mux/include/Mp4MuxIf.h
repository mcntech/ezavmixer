#ifndef __MP4_MUX_IF_H__
#define __MP4_MUX_IF_H__

#ifdef WIN32
#define int64_t __int64
#else
#define int64_t long long
#endif

#define MODE_MOV			1
#define AV_NOPTS_VALUE		-1
#define MOV_INDEX_CLUSTER_SIZE 16384

#define MAX_SPS_SIZE		256
#define MAX_PPS_SIZE		256

#define TRACK_INDEX_INVALID	-1


class CPacket {
public:
    long long		m_llPts;                            
    long long   	m_llDts;                            
    unsigned char	*m_pData;
    int				m_lSize;
    int				m_StreamIndex;
    int				m_Flags;
    int				m_lDuration;                         
}; 

class CMoofParam
{
public:
	int             m_nSize;
	int             m_nStartPtsMs;
};

class CMp4MuxIf
{
public:
	enum {
		TRACK_INDEX_VIDEO =	0,
		TRACK_INDEX_AUDIO = 1
	};
	enum {
		CODEC_TYPE_AUDIO = 1,
		CODEC_TYPE_VIDEO =	2
	};

	virtual int InitVideoTrack(unsigned char *pSpsData, long lSpsSize, unsigned char *pPpsData, long lPpsSize) { return 0;}
	virtual int DeinitVideoTrack() {return 0;}
	virtual int InitAudioTrack(unsigned char *pConfigData, long lSize) {return 0;}
	virtual int DeinitAudioTrack(){return 0;}
	virtual int GenerateInitSegment(char *pBuff, int nMaxLen){return 0;}

	virtual void *OpenMoof(char *pBuff, int nMaxLen, int nTracks){return 0;}
	virtual int CloseMoof(void *, CMoofParam *pParam){return 0;}

	virtual int WritePacket(CPacket *pPkt) {pPkt; return 0;}
};

/**
 * Creates MPEG_DASH emsg box
 */
int CreateEmsg(char *pEmsgData, int nEmsgMaxLen, 
		const char *scheme_id_uri, const char *value,
		unsigned long presentation_time_delta, unsigned long event_duration,
		unsigned long id, 
		unsigned char *message_data, int message_len);


CMp4MuxIf *CreateMp4Mux();
void DeleteMp4Mux(CMp4MuxIf *);

CMp4MuxIf *CreateMp4Segmenter(int fEnableAud, int fEnableVid);
void DeleteMp4Segmenter(CMp4MuxIf *);

#endif //__MP4_MUX_IF_H__