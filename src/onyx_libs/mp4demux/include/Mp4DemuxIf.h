#ifndef __Mp4_FILE_WRITER_IF__
#define __Mp4_FILE_WRITER_IF__

#define int64_t __int64


#define PKT_FLAG_KEY		1
#define PKT_FLAG_EOS		0x00010000

#define MODE_MOV			1
#define AV_NOPTS_VALUE		-1
#define MOV_INDEX_CLUSTER_SIZE 16384

#define MAX_SPS_SIZE		256
#define MAX_PPS_SIZE		256
#define MAX_AAC_CFG_LEN		8

#define TRACK_INDEX_INVALID	-1

#define TRACK_INDEX_VIDEO	0
#define TRACK_INDEX_AUDIO	1

namespace Mp4Demux {
	enum {
		CODEC_ID_UNKNOWN,
		CODEC_ID_H264,
		CODEC_ID_AAC
	};
	enum {
		CODEC_TYPE_UNKNOWN,
		CODEC_TYPE_AUDIO,
		CODEC_TYPE_VIDEO
	};

class CPacket {
public:
    int64_t			m_llPts;                            
    int64_t			m_llDts;                            
    unsigned char	*m_pData;
    int				m_lSize;
    int				m_CodecType;
    int				m_CodecId;
    int				m_Flags;
    int				m_lDuration;                         
}; 

class CTrackInf 
{
public:
	int			     m_CodecId;
	int			     m_CodecType;

    unsigned int     m_lTimescale;
    long long        m_llTtrackDuration;
};

class CAvcCfgRecord {
public:
	unsigned char bVersion;
	unsigned char Profile;
	unsigned char ProfileCompat;
	unsigned char Level; 
	unsigned char PictureLengthSize; 
	unsigned short nonStoredDegredationPriorityLownonReferenceDegredationPriorityLow;
	unsigned short nonReferenceStoredDegredationPriorityHigh;
	//bit(6) reserved = ‘111111’b;
	//unsigned int(2) lengthSizeMinusOne; 
	unsigned char lengthSizeMinusOne;
	unsigned char numOfSequenceParameterSets;
	//for (i=0; i< numOfSequenceParameterSets;  i++) {
	//	unsigned int(16) sequenceParameterSetLength ;
	//	bit(8*sequenceParameterSetLength) sequenceParameterSetNALUnit;
	//}
	unsigned int	lSpsSize;
	unsigned char	Sps[MAX_SPS_SIZE];
	unsigned char numOfPictureParameterSets;

	//for (i=0; i< numOfPictureParameterSets;  i++) {
	//		unsigned int(16) pictureParameterSetLength;
	//		bit(8*pictureParameterSetLength) pictureParameterSetNALUnit;
	//	}
	unsigned int	lPpsSize;
	unsigned char	Pps[MAX_PPS_SIZE];
};

class CAacCfgRecord
{
public:
	CAacCfgRecord() {cfgLen = 0;}
	int cfgLen;
	char cfgRec[MAX_AAC_CFG_LEN];
};

class CMp4DemuxIf
{
public:
	CMp4DemuxIf();
	virtual ~CMp4DemuxIf();
	static CMp4DemuxIf *CreateInstance();
	virtual int OpenFileReader(const char *pszFileName){return S_OK;}
	virtual int CloseFileReader() {return S_OK;}
	virtual int GetAvSample(int TrackId, CPacket *pPkt) {return S_OK;}

	virtual int AdvanceSample(int TrackId) {return S_OK;}
	virtual void Seek(int TrackId, long long llTime) {}

	virtual CAvcCfgRecord *GetAvcCfg(int TrackId) = 0;
	virtual int GetVidWidth(int TrackId) = 0;
	virtual int GetVidHeight(int TrackId) = 0;

	virtual CAacCfgRecord *GetAacCfg(int TrackId) = 0;
	virtual int GetAudSampleRate(int TrackId) = 0;
	virtual int GetAudNumChannels(int TrackId) = 0;

	virtual int GetNextPresentationTrackId() = 0;
	virtual int GetNmumTracks() = 0;
	virtual CTrackInf *GetTrackInf(int TrackId) = 0;		

};
}
#endif //__Mp4_FILE_WRITER_IF__