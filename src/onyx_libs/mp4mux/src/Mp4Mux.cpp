#ifdef WIN32
#include <windows.h>
#include <tchar.h>
#else
#include <unistd.h>
#include <stdarg.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include "Mp4MuxIf.h"
//#include "AVParse.h"
//#include "Dbg.h"
#include "h264parser.h"
#include <vector>
#include <list>
#include <map>

#define CODEC_ID_H264		1
#define CODEC_ID_AAC		2
#define PKT_FLAG_KEY		1

#define MAX_TRACKS			2
#define MAX_VOS_DATA_SIZE	256
#define INT32_MAX			0x7FFFFFFF
#define UINT32_MAX			0xFFFFFFFF
#define ADTS_HEADER_LEN     (7)

#define AV_NOPTS_VALUE		-1
#define MOV_INDEX_CLUSTER_SIZE 16384

#define IS_START_CODE(x)  (x[0] == 0x00 && x[1] == 0x00 && x[2] == 0x00 && x[3] == 0x01)

//#define USE_RESERVED_AREA
#define MOOV_RESERVED_BYTES		(1024 * 1024)
#define DASH_COMPLIANCE

class CWriterBase
{
public:
	virtual void WriteBuffer(const unsigned char *pData, int lSize) = 0;
	virtual int64_t GetPos() = 0;
	virtual void SeekPos(int64_t llPos) = 0;

	virtual void WriteBE32(unsigned int ulVal)
	{
		// TODO: Optimize with swap
		unsigned long dwBytesWritten;
		unsigned char strInt[4];

		strInt[0] = (unsigned char)(ulVal >> 24);
		strInt[1] = (unsigned char)(ulVal >> 16);
		strInt[2] = (unsigned char)(ulVal >> 8);
		strInt[3] = ulVal;
		WriteBuffer(strInt, 4);
	}

	void WriteTag(const char *pszTag)
	{
		WriteBuffer((const unsigned char *)pszTag, 4);
	}

	void WriteString(const char *pszVal)
	{
		unsigned char ucLen = strlen((const char *)pszVal);
		WriteBuffer(&ucLen, 1);			/* string counter */
		WriteBuffer((const unsigned char *)pszVal, ucLen);	/* handler description */
	}

	void WriteStringWithNull(const char *pszVal)
	{
		unsigned char cNull = 0;
		WriteBuffer((const unsigned char *)pszVal, strlen(pszVal));	/* handler description */
		WriteBuffer(&cNull, 1);	
	}

	int UpdateSize(long lPos)
	{
		unsigned long lCurPos = GetPos();
		SeekPos(lPos);
		WriteBE32((unsigned long)(lCurPos - lPos)); /* Modify Size */
		/* Move back to current pos */
		SeekPos(lCurPos);
		return (int)(lCurPos - lPos);
	}
};
#ifdef WIN32
// TODO : Port for linux to support direct write
class CFileWriter : public CWriterBase
{
public:
	virtual void WriteBuffer(const unsigned char *pData, int lSize)
	{
		unsigned long dwBytesWritten;
		WriteFile(m_hFile, pData, lSize, &dwBytesWritten, NULL);
	}

	virtual int64_t GetPos() 
	{
		int64_t llPos;
		LONG lDistLow = 0;
		LONG lDistHigh = 0;
#if 0
		lDistLow = GetFileSize(m_hFile, &lDistHigh);
#else
		lDistLow = SetFilePointer( m_hFile, lDistLow, &lDistHigh, FILE_CURRENT ); 
#endif
		llPos = lDistHigh;
		llPos |= llPos << 32 | lDistLow;
		return llPos;
	}

	virtual void SeekPos(int64_t llPos) 
	{
		long lDistLow = (unsigned long)(llPos & 0xffffffff);
		long lDistHigh = (unsigned long)(llPos >> 32);
		SetFilePointer( m_hFile, lDistLow, &lDistHigh, FILE_BEGIN ); 
	}
	
	HANDLE	m_hFile;
};
#endif
class CBuffWriter : public CWriterBase
{
public:
	void WriteBuffer(const unsigned char *pData, int nSize)
	{
		if(m_nWr + nSize < m_nMaxLen) {
			memcpy(m_pBuff + m_nWr, pData, nSize);
			m_nWr += nSize;
			if(m_nWr > m_nEnd)
				m_nEnd = m_nWr;
		}
	}
	
	virtual int64_t GetPos() 
	{
		return m_nWr;
	}

	virtual void SeekPos(int64_t llPos) 
	{
		if(llPos < m_nMaxLen)
			m_nWr = llPos;
	}


	CBuffWriter(char *pData, int nLen)
	{
		m_pBuff = pData;
		m_nMaxLen = nLen;
		m_nWr = m_nEnd = 0;
		m_fExternMem = 1;
	}

	~CBuffWriter()
	{
		if(!m_fExternMem && m_pBuff)
			free(m_pBuff);
	}
	char *m_pBuff;
	int64_t   m_nEnd;
	int64_t   m_nWr;
	int64_t   m_nMaxLen;
	int       m_fExternMem;
};

class CMp4AvcCfgRecord {
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
	unsigned char	lSpsSize;
	unsigned char	Sps[MAX_SPS_SIZE];
	unsigned char numOfPictureParameterSets;

	//for (i=0; i< numOfPictureParameterSets;  i++) {
	//		unsigned int(16) pictureParameterSetLength;
	//		bit(8*pictureParameterSetLength) pictureParameterSetNALUnit;
	//	}
	unsigned char	lPpsSize;
	unsigned char	Pps[MAX_PPS_SIZE];
};

typedef struct _stts_t  {
    int count;
    int duration;
} stts_t;


typedef struct _ChunkEntry {
    unsigned int flags;
	unsigned int size;
    int64_t     m_llPos;
    unsigned int samplesInChunk;
    char         key_frame;
    unsigned int entries;
    int64_t      m_Cts;
    int64_t      m_llDts;
} ChunkEntry;

#define DBG_LVL_STRM	5
#define DBG_LVL_TRACE	4
#define DBG_LVL_MSG		3
#define DBG_LVL_SETUP	2
#define DBG_LVL_WARN	1
#define DBG_LVL_ERR		0

#ifdef NO_DEBUG_LOG
#define DbgOut(DbgLevel, x)
#else
static int modDbgLevel = DBG_LVL_ERR;
static void dbg_printf (const char *format, ...)
{
	char buffer[1024 + 1];
	va_list arg;
	va_start (arg, format);
	vsprintf (buffer, format, arg);
#ifdef WIN32
	OutputDebugStringA(buffer);
#else
	fprintf(stderr, ":%s", buffer);
#endif
}

#define DbgOut(DbgLevel, x) do { if(DbgLevel <= modDbgLevel) { \
									fprintf(stderr,"%s:%s:%d:", __FILE__, __FUNCTION__, __LINE__); \
									dbg_printf x;                                          \
									fprintf(stderr,"\n");                                          \
								}                                                                  \
							} while(0);
#endif


static unsigned short getAdtsFrameLen(unsigned char *pAdts)
{
	unsigned short usLen = (((unsigned short)pAdts[3] << 11) & 0x1800) |
							(((unsigned short)pAdts[4] << 3) & 0x07F8) |
							(((unsigned short)pAdts[5] >> 5) & 0x0007);
	return usLen;
}

void getAacConfigFromAdts(char *pConfig, char *pAdts)
{
        unsigned char profileVal      = pAdts[2] >> 6;
        unsigned char sampleRateId    = (pAdts[2] >> 2 ) & 0x0f;
        unsigned char channelsVal     = ((pAdts[2] & 0x01) << 2) | ((pAdts[3] >> 6) & 0x03);
        pConfig[0]  = (profileVal + 1) << 3 | (sampleRateId >> 1);
        pConfig[1]  = ((sampleRateId & 0x01) << 7) | (channelsVal <<3);
}

int64_t CovertMsToScaledUnits(unsigned long ulTimescale, int64_t llTime)
{
	return (llTime * ((double)ulTimescale / 1000));
}

int64_t CovertScaledUnitsToMs(unsigned long ulTimescale, int64_t llTime)
{
	return (llTime * ((double)1000 / ulTimescale));
}

unsigned char *FindStartCode( unsigned char *p,  unsigned char *end)
{
    while( p < end - 3) {
        if( p[0] == 0 && p[1] == 0 && ((p[2] == 1) || (p[2] == 0 && p[3] == 1)) )
            return p;
		p++;
    }
    return end;
}

class CTrack {
public:
	CTrack()
	{
		m_CrntChunk = 0;
		m_ChunkList = NULL;
		m_fSpsFound = 0;
		m_fAudCfgFound = 0;
		m_vosLen = 0;
		m_AvgBitrate = 0;
		// TODO
		m_BufferSize = 1024*1024;
		m_AvgBitrate = 0;
		m_MinBitrate = 0;
		m_MaxBitrate = 0;
		m_AudioVbr = 1;
		m_SampleCount = 0;
	}
    int         m_Mode;
    int         m_CrntChunk;
    long        m_lTimescale;
    long        time;
    int64_t     m_llTtrackDuration;
    long        m_SampleCount;
    int         hasKeyframes;
    int         hasBframes;
    int         language;
    int         trackID;
    unsigned char  m_Tag[5]; ///< stsd fourcc
    //AVCodecContext *enc;
	long		m_lWidth;
	long		m_lHeight;
	int			m_CodecType;
	unsigned char m_szName[32];
    int         m_vosLen;
	
    unsigned char  m_vosData[MAX_VOS_DATA_SIZE];  
								// SPecific to Audio track

	CMp4AvcCfgRecord AvcCfgRecord;	// Specific to Video track
    ChunkEntry   *m_ChunkList;
    int         m_AudioVbr;


	// RAM
	int			m_fSpsFound;	// Specific to Video track
	int			m_fAudCfgFound;	// SPecific to Audio track
	int			m_CodecId;
	int			m_Samplerate;
	int			m_Channels;
	// Audio
	int        m_AvgBitrate;
	int        m_MinBitrate;
	int        m_MaxBitrate;
	int		   m_BufferSize;
};

class CSample
{
public:
	CSample(unsigned long ulFlags, unsigned long ulDuration = 0, unsigned long ulSize = 0, unsigned long ulCompTimeOffset = 0)
	{
		sample_duration = ulDuration;
		sample_size = ulSize;
		sample_flags = ulFlags;
		sample_composition_time_offset = ulCompTimeOffset;
	}
		
	unsigned int sample_duration;
	unsigned int sample_size;
	unsigned int sample_flags;
	unsigned int sample_composition_time_offset;
};

#define TRUN_DATA_OFFSET_PRESENT			0x000001
#define TRUN_FIRST_SAMPLE_FLAG_PRESENT		0x000004
#define TRUN_SAMPLE_DURATION_PRESENT		0x000100
#define TRUN_SAMPLE_SIZE_PRESENT			0x000200
#define TRUN_SAMPLE_FLAGS_PRESENT			0x000400
#define TRUN_SAMPLE_COMP_TIME_OFFSET_PRESENT	0x000800

class CTrackRun
{
public:
	CTrackRun(unsigned int ulFlags)
	{
		tr_flags = ulFlags;
	}
	~CTrackRun()
	{
		for (std::vector<CSample *>::iterator it = m_arSamples.begin(); it != m_arSamples.end(); it++) {
			delete *it;
		}
		m_arSamples.clear();
	}

	void AddSampleInf(unsigned long ulFlags, unsigned long ulDuration = 0, unsigned long ulSize = 0, unsigned long ulCompTimeOffset = 0)
	{
		CSample *pSample = new CSample(ulFlags, ulDuration, ulSize, ulCompTimeOffset);
		m_arSamples.push_back(pSample);
	}
	unsigned int tr_flags;
	unsigned int sample_count; // Same size of m_arSamples;
	signed int   data_offset;
	unsigned long ulAddr_data_offset; // Location of data offset
	unsigned int first_sample_flags;
	std::vector<CSample *> m_arSamples;
};

#define TFHD_BASE_DATA_OFFSET_PRESENT		0x000001
#define TFHD_SAMPLE_DESCRIPT_INDEX_PRESENT	0x000002
#define TFHD_DEF_SAMPLE_DURATION_PRESENT	0x000008
#define TFHD_DEF_SAMPLE_SIZE_PRESENT		0x000010
#define TFHD_DEF_SAMPLE_FLAGS_PRESENT		0x000020
#define TFHD_DURATION_EMPTY					0x010000

class CTrackFragment
{
public:
	CTrackFragment(CTrack *pTrack)
	{
		tf_flags = TFHD_DURATION_EMPTY;
		m_pTrack = pTrack;
		track_ID = pTrack->trackID;
		sample_description_index = 0;
		default_sample_duration = 0;
		default_sample_size = 0;
		default_sample_flags = 0;
		m_pTrun = new CTrackRun(TRUN_DATA_OFFSET_PRESENT | TRUN_SAMPLE_DURATION_PRESENT | TRUN_SAMPLE_SIZE_PRESENT	| TRUN_SAMPLE_COMP_TIME_OFFSET_PRESENT);
	}
	~CTrackFragment()
	{
		delete m_pTrun;
	}
public:
	unsigned int tf_flags;
	unsigned int track_ID;
	// all the following are optional fields
	unsigned long long base_data_offset;
	unsigned int sample_description_index;
	unsigned int default_sample_duration;
	unsigned int default_sample_size;
	unsigned int default_sample_flags;
	
	//unsigned long long decode_time;             //tfdt box
	unsigned long decode_time;             //tfdt box
	CTrackRun    *m_pTrun;
	CTrack       *m_pTrack;
};

class CMoof
{
public:
	CMoof()
	{
	}
	~CMoof()
	{
		for (std::map<int, CTrackFragment *>::iterator it = m_Trafs.begin(); it != m_Trafs.end(); it++) {
			CTrackFragment *pTraf = it->second;
			delete pTraf;
		}
		m_Trafs.clear();
	}
	void AddTraf(int iIndex, CTrack *pTrack, long long nDecodeTimeStamp)
	{
		CTrackFragment *pTraf = new CTrackFragment(pTrack);
		pTraf->decode_time = nDecodeTimeStamp;
		m_Trafs[iIndex] = pTraf;
	}
	CTrackFragment *GetTraf(int nIndex)
	{
		CTrackFragment *pTraf = NULL;
		std::map<int, CTrackFragment *>::iterator it;
		it = m_Trafs.find(nIndex);
		if(it != m_Trafs.end()) {
			pTraf = it->second;
		}

		return pTraf;
	}

public:
	unsigned long sequence_number;
	std::map <int, CTrackFragment *> m_Trafs;
};

class CSidxReference
{
public:
	enum SAP_TYPE_T
	{
		SAP_TYPE_0 = 0x1,
		SAP_TYPE_1 = 0x2,
		SAP_TYPE_2 = 0x3,
		SAP_TYPE_3 = 0x4,
		SAP_TYPE_4 = 0x5,
		SAP_TYPE_5 = 0x6,
	};
	
	CSidxReference(
		unsigned long reference_type, 
		unsigned long referenced_size, 
		unsigned long subsegment_duration, 
		unsigned long starts_with_SAP, 
		unsigned long SAP_type, 
		unsigned long SAP_delta_time)
	{
		m_reference_type = reference_type;
		m_referenced_size = referenced_size;
		m_subsegment_duration = subsegment_duration;
		m_starts_with_SAP = starts_with_SAP;
		m_SAP_type = SAP_type;
		m_SAP_delta_time = SAP_delta_time;
	}
	unsigned long   m_reference_type;            // 1 bit
	unsigned long   m_referenced_size;            // 31 bits
	unsigned long   m_subsegment_duration;       
	unsigned long   m_starts_with_SAP;           // 1 bit
	unsigned long   m_SAP_type;                  // 3 bits
	unsigned long   m_SAP_delta_time;            // 28 bits
	
	unsigned long   m_pos_referenced_size;
};

class CMp4Sidx
{
public:
	CMp4Sidx(unsigned long reference_ID, unsigned long timescale, unsigned long presentation_time, unsigned long first_offset)
	{
		m_pReference = NULL;
		m_reference_ID = reference_ID;
		m_timescale = timescale;
		m_earliest_presentation_time = presentation_time;
		m_first_offset = first_offset;
	}
	void SetReference(CSidxReference *pReference)
	{
		m_pReference = pReference;
	}
	void AddChildSidx(CMp4Sidx *pSidx)
	{
		m_SidxList.push_back(pSidx);
	}

	~CMp4Sidx()
	{
		for (std::vector<CMp4Sidx *>::iterator it = m_SidxList.begin(); it != m_SidxList.end(); it++) {
			delete *it;
		}
		m_SidxList.clear();
		if(m_pReference)
			delete m_pReference;
	}
	unsigned long m_reference_ID;
	unsigned long m_timescale;
	unsigned long m_earliest_presentation_time;
	unsigned long m_first_offset;
	unsigned short reserved;
	//unsigned short reference_count;
	std::vector<CMp4Sidx *> m_SidxList;
	CSidxReference *m_pReference;
	unsigned long   m_pos_first_offset;
};

class CMp4MuxBase
{
public:
	virtual void WriteBuffer(const unsigned char *pData, int lSize)
	{
		m_pWriter->WriteBuffer(pData, lSize);
	}

	virtual int64_t GetPos() 
	{
		return m_pWriter->GetPos();
	}

	virtual void SeekPos(int64_t llPos) 
	{
		m_pWriter->SeekPos(llPos);
	}

	void WriteByte(unsigned char b)
	{
		WriteBuffer(&b, 1);
	}

	void WriteBE16(unsigned int ulVal)
	{
		WriteByte((unsigned char)(ulVal >> 8));
		WriteByte((unsigned char)ulVal);
	}

	void WriteBE24(unsigned int ulVal)
	{
		WriteByte((unsigned char)(ulVal >> 16));
		WriteByte((unsigned char)(ulVal >> 8));
		WriteByte((unsigned char)ulVal);
	}


	void WriteBE32(unsigned int ulVal)
	{
		WriteByte((unsigned char)(ulVal >> 24));
		WriteByte((unsigned char)(ulVal >> 16));
		WriteByte((unsigned char)(ulVal >> 8));
		WriteByte((unsigned char)ulVal);
	}

	void WriteBE64(int64_t val)
	{
		WriteBE32((unsigned long)(val >> 32));
		WriteBE32((unsigned long)(val & 0xffffffff));
	}

	void WriteTag(const char *pszTag)
	{
		while (*pszTag) {
			WriteByte(*pszTag++);
		}
	}

	int UpdateSize(int64_t llPos)
	{
		int64_t llCurPos = GetPos();
		SeekPos(llPos);
		WriteBE32((unsigned long)(llCurPos - llPos)); /* Modify Size */
		/* Move back to current pos */
		SeekPos(llCurPos);

		return (int)(llCurPos - llPos);
	}

	int UpdateSize(int64_t llPos, unsigned long ulValue)
	{
		int64_t llCurPos = GetPos();
		SeekPos(llPos);
		WriteBE32(ulValue); /* Modify Size */
		/* Move back to current pos */
		SeekPos(llCurPos);
		return 0;
	}

public:
	CWriterBase *m_pWriter;
};

class CMp4Mux : public CMp4MuxBase, public CMp4MuxIf
{
public:


///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////// Audio ///////////////////////////////////////////////
/*
 * Calculates number of bytes required by a descriptor 
 * tag + length specifier(7bit digits ?) + length
 */
unsigned int getDescrAndTagLength(unsigned int len)
{
	return (4/*Length*/ + 1/*tag*/ + len);
}

void WriteDescrTagAndLen(unsigned char tag, unsigned int len)
{
	WriteByte(tag);
	for(int i = 3; i > 0; i--) {
		WriteByte((len >> (7*i)) | 0x80);
	}
	WriteByte(len & 0x7F);
}

/*
Containing profile atom 
Track (sound), movie

part-ID 
0x20202020 (universal feature)

feature-code 
'mp4a'

feature-value 
Unsigned int(32) where least significant 5 bits hold the AudioObjectType as found in the AudioSpecificInfo (as defined in specification ISO/IEC 14496-3, subclause 1.6) found in the esds of the MPEG-4 audio codec (QuickTime type 'mp4a') sample description

Feature Values
The least significant 5 bits hold the value. The most significant 27 bits of the feature value should be set to 0.

The list of audio object type constants is defined in specification ISO/IEC 14496-3, subclause 1.5.1.1.

Examples: AAC LC is indicated by the value 2, CELP is indicated by the value 8.

Writer Responsibilities
A writer of the MPEG-4 Audio Codec feature should record the 5 bits corresponding to the AudioObjectType found in the ES_descriptor's audio DecoderSpecificConfig. The most significant 27 bits of the value should be set to 0.

  * 8+ bytes vers. 2 ES Descriptor box
      = long unsigned offset + long ASCII text string 'esds'
   - if encoded to ISO/IEC 14496-10 AVC standards then optionally use:
      = long unsigned offset + long ASCII text string 'm4ds'

    -> 4 bytes version/flags = 8-bit hex version + 24-bit hex flags
        (current = 0)

    -> 1 byte ES descriptor type tag = 8-bit hex value 0x03
    -> 3 bytes extended descriptor type tag string = 3 * 8-bit hex value
      - types are Start = 0x80 ; End = 0xFE
      - NOTE: the extended start tags may be left out
    -> 1 byte descriptor type length = 8-bit unsigned length

      -> 2 bytes ES ID = 16-bit unsigned value
      -> 1 byte stream priority = 8-bit unsigned value
        - Defaults to 16 and ranges from 0 through to 31

        -> 1 byte decoder config descriptor type tag = 8-bit hex value 0x04
        -> 3 bytes extended descriptor type tag string = 3 * 8-bit hex value
          - types are Start = 0x80 ; End = 0xFE
          - NOTE: the extended start tags may be left out
        -> 1 byte descriptor type length = 8-bit unsigned length

          -> 1 byte object type ID = 8-bit unsigned value
            - type IDs are system v1 = 1 ; system v2 = 2
            - type IDs are MPEG-4 video = 32 ; MPEG-4 AVC SPS = 33
            - type IDs are MPEG-4 AVC PPS = 34 ; MPEG-4 audio = 64
            - type IDs are MPEG-2 simple video = 96
            - type IDs are MPEG-2 main video = 97
            - type IDs are MPEG-2 SNR video = 98
            - type IDs are MPEG-2 spatial video = 99
            - type IDs are MPEG-2 high video = 100
            - type IDs are MPEG-2 4:2:2 video = 101
            - type IDs are MPEG-4 ADTS main = 102
            - type IDs are MPEG-4 ADTS Low Complexity = 103
            - type IDs are MPEG-4 ADTS Scalable Sampling Rate = 104
            - type IDs are MPEG-2 ADTS = 105 ; MPEG-1 video = 106
            - type IDs are MPEG-1 ADTS = 107 ; JPEG video = 108
            - type IDs are private audio = 192 ; private video = 208
            - type IDs are 16-bit PCM LE audio = 224 ; vorbis audio = 225
            - type IDs are dolby v3 (AC3) audio = 226 ; alaw audio = 227
            - type IDs are mulaw audio = 228 ; G723 ADPCM audio = 229
            - type IDs are 16-bit PCM Big Endian audio = 230
            - type IDs are Y'CbCr 4:2:0 (YV12) video = 240 ; H264 video = 241
            - type IDs are H263 video = 242 ; H261 video = 243
          -> 6 bits stream type = 3/4 byte hex value
            - type IDs are object descript. = 1 ; clock ref. = 2
            - type IDs are scene descript. = 4 ; visual = 4
            - type IDs are audio = 5 ; MPEG-7 = 6 ; IPMP = 7
            - type IDs are OCI = 8 ; MPEG Java = 9
            - type IDs are user private = 32
          -> 1 bit upstream flag = 1/8 byte hex value
          -> 1 bit reserved flag = 1/8 byte hex value set to 1
          -> 3 bytes buffer size = 24-bit unsigned value
          -> 4 bytes maximum bit rate = 32-bit unsigned value
          -> 4 bytes average bit rate = 32-bit unsigned value

            -> 1 byte decoder specific descriptor type tag
                = 8-bit hex value 0x05
            -> 3 bytes extended descriptor type tag string
                = 3 * 8-bit hex value
              - types are Start = 0x80 ; End = 0xFE
              - NOTE: the extended start tags may be left out
            -> 1 byte descriptor type length
                = 8-bit unsigned length

              -> ES header start codes = hex dump

        -> 1 byte SL config descriptor type tag = 8-bit hex value 0x06
        -> 3 bytes extended descriptor type tag string = 3 * 8-bit hex value
          - types are Start = 0x80 ; End = 0xFE
          - NOTE: the extended start tags may be left out
        -> 1 byte descriptor type length = 8-bit unsigned length

          -> 1 byte SL value = 8-bit hex value set to 0x02
*/

int WriteAacEsdsBox(CTrack *pTrack) // Basic
{
	int64_t llPos = GetPos();
	int decoderSpecificInfoLen = pTrack->m_vosLen ? getDescrAndTagLength(pTrack->m_vosLen) : 0;

	WriteBE32( 0);				// reserved for size

	/*  * 8+ bytes vers. 2 ES Descriptor box
      = long unsigned offset + long ASCII text string 'esds'*/
	WriteTag("esds");

	/* -> 4 bytes version/flags = 8-bit hex version + 24-bit hex flags  (current = 0)*/
	WriteBE32(0);				// Version(8) & Flags(24)

	int esdDescLen = 
		3
		+ (5 + 13 + (5 + decoderSpecificInfoLen))
		+ 5 + 1;
	// ES descriptor
	WriteDescrTagAndLen(0x03, esdDescLen);

	/* two bytes ES ID = 16-bit unsigned value */
	/* 2*/WriteBE16(pTrack->trackID);
	/* 3*/WriteByte(0x00);			// flags (= no flags)

	// DecoderConfig descriptor
	int decconfigDescLen = 13 + 5 + decoderSpecificInfoLen;
	/* 5*/WriteDescrTagAndLen(0x04, decconfigDescLen);

		/* 1*/WriteByte(0x40/*pTrack->m_CodecId*/);
		/* 2*/WriteByte(0x15); // flags (= Audiostream)
		/* 3*/WriteByte(pTrack->m_BufferSize >> (3+16));    // Buffersize DB (24 bits)
		/* 5*/WriteBE16((pTrack->m_BufferSize >> 3) & 0xFFFF); // Buffersize DB

		/* 9*/WriteBE32(pTrack->m_MaxBitrate);
		
		/* 13*/
		if(pTrack->m_MaxBitrate != pTrack->m_MinBitrate || pTrack->m_MinBitrate == 0)
			WriteBE32(0); // vbr
		else
			WriteBE32(pTrack->m_AvgBitrate);

		// DecoderSpecific info descriptor
		if (pTrack->m_vosLen) {
			WriteDescrTagAndLen(0x05, pTrack->m_vosLen);
			WriteBuffer (pTrack->m_vosData, pTrack->m_vosLen);
		}
	/*  -> 1 byte SL config descriptor type tag = 8-bit hex value 0x06    // SL descriptor*/
	WriteDescrTagAndLen(0x06, 1);
	/*      -> 1 byte SL value = 8-bit hex value set to 0x02 */
	WriteByte(0x02);

	/* Fill the size */
	return UpdateSize(llPos);
}

int WriteWaveBox(CTrack *pTrack)
{
	int64_t llPos = GetPos();

    WriteBE32(0);			/* size */
    WriteTag("wave");

    WriteBE32(12);			/* size */
    WriteTag("frma");
	//WriteBE32((const char *)pTrack->m_Tag);
	WriteTag((const char *)pTrack->m_Tag);
    if (pTrack->m_CodecId == CODEC_ID_AAC) {
        /* useless atom needed by mplayer, ipod, not needed by quicktime */
        WriteBE32(12); /* size */
        WriteTag("mp4a");
        WriteBE32(0);
        WriteAacEsdsBox(pTrack);
    }
#if 0
	else if (track->enc->codec_id == CODEC_ID_PCM_S24LE ||
               track->enc->codec_id == CODEC_ID_PCM_S32LE) {
        mov_write_enda_tag(pb);
    } else if (track->enc->codec_id == CODEC_ID_AC3) {
        mov_write_ac3_tag(pb, track);
    } else if (track->enc->codec_id == CODEC_ID_ALAC) {
        mov_write_extradata_tag(pb, track);
    }
#endif
	WriteBE32(8);				/* size */
	WriteBE32(0);				/* null tag */
	return UpdateSize(llPos);
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int WriteNalUnits(
	  unsigned char			*pData, 
	  int					*size)
{
    unsigned char *p = pData;
    unsigned char *end = p + *size;
    unsigned char *pNalStart, *pNalEnd;
	int lNalSize = 0;
	long lBytesWritten = 0;
    pNalStart = FindStartCode(p, end);
	CTrack	 *pTrack = GetTrack(TRACK_INDEX_VIDEO);

	if(pTrack == NULL)
		return -1;

    while (pNalStart < end) {
		/* Skip Start code */
		while(!*(pNalStart++));
        
		pNalEnd = FindStartCode(pNalStart, end);
		lNalSize = pNalEnd - pNalStart;
		assert(lNalSize > 0);
		/* Update SPS and PPS */
		if((pNalStart[0] & 0x1F) == 7 /*SPS*/) {
			UpdateAvccSps(&pTrack->AvcCfgRecord, pNalStart, lNalSize);
			pTrack->m_fSpsFound = 1;
		} else if((pNalStart[0] & 0x1F) == 8 /*PPS*/) {
			UpdateAvccPps(&pTrack->AvcCfgRecord, pNalStart, lNalSize);
		}
		if((pNalStart[0] & 0x1F) >= 20) {
			assert(0);
		}
		if(pTrack->m_fSpsFound) {
			WriteBE32( lNalSize);
			WriteBuffer(pNalStart, lNalSize);
			lBytesWritten += (4 + lNalSize);
		} else {
			// Discard data
		}
        pNalStart = pNalEnd;
    }
    return lBytesWritten;
}


unsigned char *FindAacFrameStart(
	 unsigned char *p, 
	 unsigned char *end)
{
    while( p < end - 3) {
        if( p[0] == 0XFF && ((p[1] & 0xFE) == 0xF8) && p[1] != 0xFF)
            return p;
		p++;
    }

    return end;
}

int WriteAacFrames(
	CTrack	        *pTrk,
	CPacket         *pPkt
	)
{
    unsigned char *pFrameStart =  pPkt->m_pData;
	unsigned char *pDataEnd = pFrameStart + pPkt->m_lSize;
	int nFrameSize = 0;
	long lBytesWritten = 0;

	int64_t llPts = CovertMsToScaledUnits(pTrk->m_lTimescale, pPkt->m_llPts);
	int64_t llDts = llPts;

    while ((pFrameStart + 7) < pDataEnd) {
		nFrameSize = getAdtsFrameLen(pFrameStart);
		int nAdtsHdrSize = 7 + (( pFrameStart[1] & 0x01 ) ? 0 : 2 );
		unsigned char *pData = pFrameStart + nAdtsHdrSize;
		int64_t llChunkPos = GetPos();
		int nSize = nFrameSize - nAdtsHdrSize;
		assert(nSize);
		WriteBuffer(pData, nSize);
		AddChunkInfo(pTrk, llChunkPos, nSize, 1, llPts, llDts, 1);
		llPts += 1024;	// 1024 samples
		llDts = llPts;
		lBytesWritten += nSize;
		pFrameStart += nFrameSize;

    }
    return lBytesWritten;
}


int64_t WriteFreeBox(long lReservedSize)
{
	char FillBytes[1024] = {0};
	int64_t llPos = GetPos();
	WriteBE32(lReservedSize);		/* for size */
	WriteTag("free");	/* Header */
	lReservedSize -= 8;
	while(lReservedSize){
		int FillSize = lReservedSize > 1024 ? 1024 : lReservedSize;
		WriteBuffer((const unsigned char *)FillBytes, FillSize);
		lReservedSize -= FillSize;
	}
	return llPos;
}

int WriteFtypBox()
{
	int64_t llPos = GetPos();
	WriteBE32(0);		/* for size */
	WriteTag("ftyp");	/* Header */
#ifdef DASH_COMPLIANCE
	WriteTag("dash");	/* MajorBrand */
	WriteBE32(0);		/* MinorVersion */
	WriteTag("iso6");	/* Compatible brand */
	WriteTag("avc1");
#else
	WriteTag("isom");	/* MajorBrand */
	WriteBE32(0);		/* MinorVersion */
	WriteTag("isom");	/* Compatible brand */
	WriteTag("avc1");
#endif
    return UpdateSize(llPos);
}

int WriteMdatBox()
{
    m_llMdatPos = GetPos();
    WriteBE32(0);		/* for size */
    WriteTag("mdat");
    return 0;
}


int WriteMovieHeader()
{
	WriteFtypBox();
#ifdef USE_RESERVED_AREA
	m_llMoovPos = WriteFreeBox(MOOV_RESERVED_BYTES);
#endif
	WriteMdatBox();
	return 0;
}

/* Supports only 32 bit size */
int WriteMovieTrailer()
{
	DbgOut(DBG_LVL_TRACE,("%s Enter", __FUNCTION__));
	
	int64_t llPos = GetPos();
	
	SeekPos(m_llMdatPos);
	WriteBE32(m_lMdatSize + 8);

	DbgOut(DBG_LVL_MSG,("%s m_llMdatPos=%llx m_lMdatSize=%x: Enter", __FUNCTION__,m_llMdatPos, m_lMdatSize));

	/* Update movie duration */
	for (std::map<int, CTrack *>::iterator it  = m_Tracks.begin(); it  != m_Tracks.end();it++) {
		CTrack *pTrack = it->second;
		if(pTrack->m_CrntChunk) {
			int64_t llTtrackDuration = (int64_t)(pTrack->m_llTtrackDuration * ((double)m_lMovieTimescale / pTrack->m_lTimescale));
			if(llTtrackDuration > m_llMovieDuration)
				m_llMovieDuration = llTtrackDuration;
		}
	}
#ifdef USE_RESERVED_AREA
	SeekPos(m_llMoovPos);
	long lMoovBytes = WriteMoovBox();
	WriteFreeBox(MOOV_RESERVED_BYTES - lMoovBytes);
#else
	SeekPos(llPos);
	WriteMoovBox();
#endif
	return 0;
}


int UpdateAvccSps(
		CMp4AvcCfgRecord *pAvcCfgRecord, 
		unsigned char *pSpsData, long lSpsSize)
{
	pAvcCfgRecord->bVersion = 1;
	pAvcCfgRecord->Profile = pSpsData[1]; /* profile */
	// TODO
	pAvcCfgRecord->ProfileCompat = 0; /* profile compat */
	pAvcCfgRecord->Level = pSpsData[3];;

	pAvcCfgRecord->lSpsSize = (unsigned char)lSpsSize;
	memcpy( pAvcCfgRecord->Sps, pSpsData, lSpsSize);
	
	CTrack	 *pTrack = GetTrack(TRACK_INDEX_VIDEO);
#if 0 // TODO
	h264_decode_seq_parameter_set((char *)pSpsData, lSpsSize,&pTrack->m_lWidth, &pTrack->m_lHeight);
#endif
	return 0;
}

int UpdateAvccPps(
		CMp4AvcCfgRecord *pAvcCfgRecord, 
		unsigned char *pPpsData, long lPpsSize)
{
	pAvcCfgRecord->lPpsSize = (unsigned char)lPpsSize;
	memcpy( pAvcCfgRecord->Pps, pPpsData, lPpsSize);

	return 0;
}

int WriteAvcc(CMp4AvcCfgRecord *pAvcCfgRecord)
{
	WriteByte( pAvcCfgRecord->bVersion); /* version */
	WriteByte(pAvcCfgRecord->Profile); /* profile */

	WriteByte(pAvcCfgRecord->ProfileCompat); /* profile compat */
	WriteByte(pAvcCfgRecord->Level); /* level */
	WriteByte( 0xff); /* 6 bits reserved (111111) + 2 bits nal size length - 1 (11) */
	WriteByte( 0xe1); /* 3 bits reserved (111) + 5 bits number of sps (00001) */

	WriteBE16(pAvcCfgRecord->lSpsSize);
	WriteBuffer( pAvcCfgRecord->Sps, pAvcCfgRecord->lSpsSize);
	WriteByte( 1); /* number of pps */
	WriteBE16(pAvcCfgRecord->lPpsSize);
	WriteBuffer(pAvcCfgRecord->Pps, pAvcCfgRecord->lPpsSize);

	return 0;
}

int WriteAvccBox(CTrack *track)
{
    int64_t llPos = GetPos();

    WriteBE32(0);
    WriteTag("avcC");
	WriteAvcc(&track->AvcCfgRecord);
    return UpdateSize(llPos);
}
/*
** moov: Movie Box
** The metadata for a presentation.
** 
** Parent : File
*/

int WriteMoovBox()
{
	int64_t llPos = GetPos();
	WriteBE32(0);		/* for size */
	WriteTag("moov");	/* Header */
	DbgOut(DBG_LVL_MSG,("%s : moov pos=%llx ", __FUNCTION__,llPos));
	WriteMvhdBox();

	for (std::map<int, CTrack *>::iterator it  = m_Tracks.begin(); it  != m_Tracks.end();it++) {
		CTrack *pTrack = it->second;
        //if(pTrack->m_CrntChunk > 0) 
		{
#ifdef DASH_COMPLIANCE
			// For Moof only
			WriteMvexBox(pTrack);
#endif
            WriteTrakBox(pTrack);
        }
    }

    return UpdateSize(llPos);
}

/*
** mvhd : Movie Header Box
** This box defines overall information which is media-independent, and relevant to the entire presentation
** considered as a whole.
**
** Parent : moov
*/
int WriteMvhdBox()
{
	unsigned char bVersion = 0;
	WriteBE32(108);				// mvhd size for version 0
    WriteTag("mvhd");
    WriteByte(0);				// bVersion = 0 
    WriteBE24( 0); /* flags */

	WriteBE32(m_lTime);		/*The creation time of the box,
							  expressed as seconds elapsed since
							  midnight, January 1, 1904 (UTC) */
    WriteBE32(m_lTime);		/* modification time */

	WriteBE32(m_lMovieTimescale); /* timescale */
	if(bVersion)
		WriteBE64(m_llMovieDuration); /* duration of longest track */
	else 
		WriteBE32((unsigned long)m_llMovieDuration); /* duration of longest track */

    WriteBE32(0x00010000);	/* reserved (preferred rate) 1.0 = normal */
    WriteBE16( 0x0100);	/* reserved (preferred volume) 1.0 = normal */
    WriteBE16( 0);		/* reserved */
    WriteBE32( 0);			/* reserved */
    WriteBE32( 0);			/* reserved */

    /* Matrix structure */
    WriteBE32(0x00010000);	/* reserved */
    WriteBE32( 0x0);		/* reserved */
    WriteBE32( 0x0);		/* reserved */
    WriteBE32( 0x0);		/* reserved */
    WriteBE32( 0x00010000); /* reserved */
    WriteBE32( 0x0);		/* reserved */
    WriteBE32( 0x0);		/* reserved */
    WriteBE32( 0x0);		/* reserved */
    WriteBE32( 0x40000000); /* reserved */

    WriteBE32( 0);			/* reserved (preview time) */
    WriteBE32( 0);			/* reserved (preview duration) */
    WriteBE32( 0);			/* reserved (poster time) */
    WriteBE32( 0);			/* reserved (selection time) */
    WriteBE32( 0);			/* reserved (selection duration) */
    WriteBE32( 0);			/* reserved (current time) */
    WriteBE32( m_MaxTrackID); /* Next track id */
    return 0x6c;
}

/*
** trak : Track Box
** This is a container box for a single track of a presentation.
** The presentation carries its own temporal and spatial
** information. Each track will contain its associated Media Box.
**
** Parent : moov
*/

int WriteTrakBox(CTrack *track)
{
	DbgOut(DBG_LVL_TRACE,("%s track=%d : Enter", __FUNCTION__,track->trackID));
    int64_t llPos = GetPos();
    WriteBE32(0);			/* size */
    WriteTag("trak");
    WriteTkhdBox(track);
    WriteMdiaBox(track);
    int ret = UpdateSize(llPos);
	DbgOut(DBG_LVL_TRACE,("%s track=%d : Leave", __FUNCTION__,track->trackID));
	return ret;
}

/*
** tkhd : Track Header Box
** This box specifies the characteristics of a single track
**
** Parent : trak
*/

int WriteTkhdBox(CTrack *track)
{
    unsigned char bVersion = track->m_llTtrackDuration < INT32_MAX ? 0 : 1;

    (bVersion == 1) ? WriteBE32(104) : WriteBE32( 92); /* size */
    WriteTag("tkhd");
    WriteByte(bVersion);
#ifdef DASH_COMPLIANCE
    WriteBE24(0x3); /* flags (track enabled) */
#else
    WriteBE24(0xf); /* flags (track enabled) */
#endif
    if (bVersion == 1) {
        WriteBE64(track->time);
        WriteBE64(track->time);
    } else {
        WriteBE32(track->time); /* creation time */
        WriteBE32(track->time); /* modification time */
    }
    WriteBE32(track->trackID); /* track-id */
    WriteBE32( 0); /* reserved */

	int64_t llTtrackDuration = (int64_t)(track->m_llTtrackDuration * ((double)m_lMovieTimescale / track->m_lTimescale));
    (bVersion == 1) ? WriteBE64(llTtrackDuration) : WriteBE32((unsigned long)llTtrackDuration);

    WriteBE32( 0); /* reserved */
    WriteBE32( 0); /* reserved */
    WriteBE32( 0x0); /* reserved (Layer & Alternate group) */
    /* Volume, only for audio */
    if(track->m_CodecType == CODEC_TYPE_AUDIO)
        WriteBE16( 0x0100);
    else
        WriteBE16( 0);
    WriteBE16( 0); /* reserved */

    /* Matrix structure */
    WriteBE32( 0x00010000); /* reserved */
    WriteBE32( 0x0); /* reserved */
    WriteBE32( 0x0); /* reserved */
    WriteBE32( 0x0); /* reserved */
    WriteBE32( 0x00010000); /* reserved */
    WriteBE32( 0x0); /* reserved */
    WriteBE32( 0x0); /* reserved */
    WriteBE32( 0x0); /* reserved */
    WriteBE32( 0x40000000); /* reserved */

    /* Track width and height, for visual only */
    if(track->m_CodecType == CODEC_TYPE_VIDEO) {
        //double sample_aspect_ratio = av_q2d(st->sample_aspect_ratio);
        //if(!sample_aspect_ratio) sample_aspect_ratio = 1;
		double sample_aspect_ratio = 1;
        WriteBE32((int)(sample_aspect_ratio * track->m_lWidth*0x10000));
        WriteBE32(track->m_lHeight*0x10000);
    }  else {
        WriteBE32(0);
        WriteBE32(0);
    }
    return 0x5c;
}

/*
** mdia : Media Box
** The media declaration container contains all the objects that declare information about the media data within a
** track.
**
** Parent : trak
*/

int WriteMdiaBox(CTrack *track)
{
    int64_t llPos = GetPos();
    WriteBE32(0); /* size */
    WriteTag( "mdia");
    WriteMdhdBox( track);
    WriteHdlrBox( track);
    WriteMinfBox( track);
    return UpdateSize(llPos);
}

/*
** mdhd : Media Header Box
** The media header declares overall information that is media-independent, and relevant to characteristics of
** the media in a track
**
** Parent : mdia
*/

int WriteMdhdBox(CTrack *track)
{
    unsigned char bVersion = track->m_llTtrackDuration < INT32_MAX ? 0 : 1;

    (bVersion == 1) ? WriteBE32( 44) : WriteBE32(32); /* size */
    WriteTag("mdhd");
    WriteByte(bVersion);
    WriteBE24( 0); /* flags */
    if (bVersion == 1) {
        WriteBE64(track->time);
        WriteBE64(track->time);
    } else {
        WriteBE32(track->time); /* creation time */
        WriteBE32(track->time); /* modification time */
    }
    WriteBE32(track->m_lTimescale); /* time scale (sample rate for audio) */
    (bVersion == 1) ? WriteBE64(track->m_llTtrackDuration) : WriteBE32((int)(track->m_llTtrackDuration)); /* duration */
    WriteBE16(track->language); /* language */
    WriteBE16(0); /* reserved (quality) */

    return 32;
}
/*
** hdlr : Handler Reference Box
** This box within a Media Box declares the process by which the media-data in the track is presented, and thus,
** the nature of the media in a track. For example, a video track would be handled by a video handler.
** Parent : mdia
*/

int WriteHdlrBox(CTrack *track)
{
    const char *descr, *hdlr, *hdlrType;
    int64_t llPos = GetPos();

    hdlr = (track->m_Mode == MODE_MOV) ? "mhlr" : "\0\0\0\0";
    if (track->m_CodecType == CODEC_TYPE_VIDEO) {
        hdlrType = "vide";
        descr = "VideoHandler";
    } else {
        hdlrType = "soun";
        descr = "SoundHandler";
    }

    WriteBE32(0); /* BOX size */
    WriteTag("hdlr");
    WriteBE32(0); 
    WriteBuffer((const unsigned char *)hdlr, 4); 
    WriteTag(hdlrType);							
    WriteBE32(0); /* reserved */
    WriteBE32(0); /* reserved */
    WriteBE32(0); /* reserved */
    WriteByte((unsigned char)strlen(descr));			/* string counter */
    WriteBuffer((const unsigned char *)descr, strlen(descr));	/* handler description */
    return UpdateSize(llPos);
}

/*
** iinf : Media Information Box
** This box contains all the objects that declare characteristic information of the media in the track.
**
** Parent : mdia
*/

int WriteMinfBox(CTrack *track)
{
    int64_t llPos = GetPos();
    WriteBE32(0); /* size */
    WriteTag("minf");
	if(track->m_CodecType == CODEC_TYPE_VIDEO)
        WriteVmhdBox();
    else
        WriteSmhdBox();

    WriteDinfBox();
    WriteStblBox(track);
    return UpdateSize(llPos);
}

/*
** dinf : Data Reference Box
** The data information box contains objects that declare the location of the media information in a track.
**
** Parent : minf
*/

int WriteDinfBox()
{
    int64_t llPos = GetPos();
    WriteBE32(0); /* size */
    WriteTag("dinf");
    WriteDrefBox();
    return UpdateSize(llPos);
}

/*
** dref : Data Information Box
** The data reference object contains a table of data references (normally URLs) that declare the location(s) of
** the media data used within the presentation
**
** Parent : dinf
*/

int WriteDrefBox()
{
    WriteBE32(28); /* size */
    WriteTag( "dref");
    WriteBE32( 0); /* version & flags */
    WriteBE32(1); /* entry count */

    WriteBE32(0xc); /* size */
    WriteTag("url ");
    WriteBE32(1); /* version & flags */

    return 28;
}


/*
** vmhd : Video Media Header Box
** The video media header contains general presentation information, independent of the coding, for video
** media
**
** Parent : minf
*/
int WriteVmhdBox()
{
    WriteBE32(0x14); /* size (always 0x14) */
    WriteTag( "vmhd");
    WriteBE32( 0x01); /* version & flags */
    WriteBE64( 0); /* reserved (graphics m_Mode = copy) */
    return 0x14;
}

/*
** smhd : Sound Media Header Box
** The sound media header contains general presentation information, independent of the coding, for audio
** media.
**
** Parent : minf
*/

int WriteSmhdBox()
{
    WriteBE32(16); /* size */
    WriteTag("smhd");
    WriteBE32(0); /* version & flags */
    WriteBE16(0); /* reserved (balance, normally = 0) */
    WriteBE16(0); /* reserved */
    return 16;
}


/*
** stbl : Sample Table Box
** The sample table contains all the time and data indexing of the media samples in a track
**
** Parent : minf
*/

int WriteStblBox(CTrack *track)
{
    int64_t llPos = GetPos();
    WriteBE32( 0); /* size */
    WriteTag("stbl");
    WriteStsdBox(track);
    WriteSttsBox(track);
    if (track->m_CodecType == CODEC_TYPE_VIDEO &&
        track->hasKeyframes && track->hasKeyframes < track->m_CrntChunk)
        WriteStssBox( track);
    if (track->m_CodecType == CODEC_TYPE_VIDEO &&
        track->hasBframes)
        WriteCttsBox( track);
    WriteStscBox( track);
    WriteStszBox(track);
    WriteStcoBox(track);
    return UpdateSize(llPos);
}
/*
** Parent : stbl
*/

int WriteStsdBox(CTrack *track)
{
    int64_t llPos = GetPos();
    WriteBE32(0); /* size */
    WriteTag( "stsd");
    WriteBE32(0); /* version & flags */
    WriteBE32(1); /* entry count */
    if (track->m_CodecType == CODEC_TYPE_VIDEO)
        WriteVideoBox(track);
    else if (track->m_CodecType == CODEC_TYPE_AUDIO)
        WriteAudioBox(track);
    return UpdateSize(llPos);
}

/*
** Parent : StsdBox
*/
int WriteVideoBox(CTrack *track)
{
    int64_t llPos = GetPos();
    char compressor_name[32];

	/* sampleEntry */
    WriteBE32(0); /* size */
    WriteTag((const char *)track->m_Tag); // store it byteswapped
    WriteBE32( 0);	/*const unsigned int(8)[6] reserved = 0;*/
    WriteBE16( 0); 
    WriteBE16(1);	/* unsigned int(16) data_reference_index; */

    WriteBE16( 0);	/* unsigned int(16) pre_defined */
    WriteBE16( 0);	/* unsigned int(16) reserved */
    
	WriteBE32( 0);	/* unsigned int(32)[3] pre_defined */
    WriteBE32( 0);
    WriteBE32( 0);

	WriteBE16(track->m_lWidth); /* Video width */
    WriteBE16(track->m_lHeight); /* Video height */
    WriteBE32( 0x00480000); /* Horizontal resolution 72dpi */
    WriteBE32( 0x00480000); /* Vertical resolution 72dpi */
    WriteBE32( 0); /* reserved */
    WriteBE16( 1); /* Frame count (= 1) */

    memset(compressor_name,0,32);
#if 0
	// TODO:
	strncpy_s(compressor_name,31, (const char *)track->m_szName,31);
#endif
    WriteByte( (unsigned char)strlen(compressor_name));
    WriteBuffer((const unsigned char *)compressor_name, 31);

	WriteBE16( 0x18); /* Reserved */
    WriteBE16( 0xffff); /* Reserved */
	WriteAvccBox(track);
	//if(track->m_Mode == MODE_IPOD)
	//	mov_write_uuid_tag_ipod(pb);

    return UpdateSize(llPos);
}
/*
** Parent : StsdBox
*/
int WriteAudioBox(CTrack *track)
{
    int64_t llPos = GetPos();
    WriteBE32(0); /* size */
    WriteTag((const char *)track->m_Tag); // store it byteswapped
    WriteBE32(0); /* unsigned int(8)[6] reserved */
    WriteBE16(0); 
    WriteBE16(1); /* unsigned int(16) data_reference_index */

	WriteBE32(0); /* unsigned int(32)[2] reserved = 0; */
	WriteBE32(0); 

	WriteBE16(track->m_Channels);
	WriteBE16(16);

    WriteBE16(0); /* unsigned int(16) pre_defined = 0; */
    WriteBE16(0); /* unsigned int(16) reserved = 0 */
	WriteBE32(track->m_Samplerate << 16);

	WriteAacEsdsBox(track);

    return UpdateSize(llPos);
}


/* 
** Decoding Time to Sample Box
** This box contains a compact version of a table that allows indexing from decoding time to sample number.
** Each entry in the table gives the
** number of consecutive samples with the same time delta, and the delta of those samples. By adding the
** deltas a complete time-to-sample map may be built.
*/
int WriteSttsBox(CTrack *track)
{
    stts_t *stts_entries = NULL;
    long entries = 0;
    unsigned long atom_size;
    int i;

    if (track->m_CodecType == CODEC_TYPE_AUDIO && !track->m_AudioVbr) {
        stts_entries = (stts_t *) malloc(sizeof(stts_t)); /* one entry */
        stts_entries[0].count = track->m_SampleCount;
        stts_entries[0].duration = 1;
        entries = 1;
    } else {
		if(track->m_CrntChunk > 0) {
			stts_entries = (stts_t *)malloc(track->m_CrntChunk * sizeof(stts_t)); /* worst case */
			for (i=0; i<track->m_CrntChunk; i++) {
				int64_t duration = 0;
				if( i + 1 == track->m_CrntChunk )
					duration = track->m_llTtrackDuration - track->m_ChunkList[i].m_llDts + track->m_ChunkList[0].m_llDts;
				else
					duration = track->m_ChunkList[i+1].m_llDts - track->m_ChunkList[i].m_llDts;
            
				// TODO:
				if(duration < 0)
					duration = 0;

				if (i && duration == stts_entries[entries].duration) {
					stts_entries[entries].count++; /* compress */
				} else {
					entries++;
					stts_entries[entries].duration = (int)duration;
					stts_entries[entries].count = 1;
				}
			}
			entries++; /* last one */
		}
    }
    atom_size = 16 + (entries * 8);

    WriteBE32(atom_size);	/* BOX size */
    WriteTag("stts");
    WriteBE32( 0); /* version & flags */
    WriteBE32(entries); /* entry count */
    for (i=0; i<entries; i++) {
        WriteBE32(stts_entries[i].count);
        WriteBE32(stts_entries[i].duration);
    }
	if(stts_entries)
		free(stts_entries);
    return atom_size;
}

int WriteStssBox(CTrack *track)
{
    int64_t llCurpos, llEntryPos;
    int i, index = 0;
    int64_t llPos = GetPos();
    WriteBE32( 0); // size
    WriteTag( "stss");
    WriteBE32( 0); // version & flags
    llEntryPos = GetPos();
    WriteBE32(track->m_CrntChunk); // entry count
    for (i=0; i<track->m_CrntChunk; i++) {
        if(track->m_ChunkList[i].key_frame == 1) {
            WriteBE32( i+1);
            index++;
        }
    }
    llCurpos = GetPos();
    SeekPos(llEntryPos);
    WriteBE32(index); // rewrite size
    SeekPos(llCurpos);
    return UpdateSize(llPos);
}

/*
** Composition Time to Sample Box
** This box provides the offset between decoding time and composition time. Since decoding time must be less
** than the composition time, the offsets are expressed as unsigned numbers such that CT(n) = DT(n) +
** CTTS(n) where CTTS(n) is the (uncompressed) table entry for sample n.
*/
int WriteCttsBox(CTrack *track)
{
    stts_t *pCttsEntries = NULL;
    unsigned long ulEentries = 0;
    unsigned long ulBoxSize;
    unsigned long i;
	if(track->m_CrntChunk > 0) {
		pCttsEntries = (stts_t *)malloc((track->m_CrntChunk + 1) * sizeof(stts_t)); /* worst case */
		pCttsEntries[0].count = 1;
		pCttsEntries[0].duration = track->m_ChunkList[0].m_Cts;
		for (i=1; i<track->m_CrntChunk; i++) {
			if (track->m_ChunkList[i].m_Cts == pCttsEntries[ulEentries].duration) {
				pCttsEntries[ulEentries].count++; /* compress */
			} else {
				ulEentries++;
				pCttsEntries[ulEentries].duration = track->m_ChunkList[i].m_Cts;
				pCttsEntries[ulEentries].count = 1;
			}
		}
		ulEentries++; /* last one */
	}
    ulBoxSize = 16 + (ulEentries * 8);
    WriteBE32(ulBoxSize); /* size */
    WriteTag("ctts");
    WriteBE32( 0); /* version & flags */
    WriteBE32(ulEentries); /* entry count */
    for (i=0; i<ulEentries; i++) {
        WriteBE32( pCttsEntries[i].count);
        WriteBE32( pCttsEntries[i].duration);
    }

	if(pCttsEntries) {
		free(pCttsEntries);
	}
    return ulBoxSize;
}

/*
** Sample to chunk atom 
** Samples within the media data are grouped into chunks. Chunks can be of different sizes, and the samples
** within a chunk can have different sizes. This table can be used to find the chunk that contains a sample, its
** position, and the associated sample description
** 
** The table is compactly coded. Each entry gives the index of the first chunk of a run of chunks with the same
** characteristics. By subtracting one entry here from the previous one, you can compute how many chunks are
** in this run. You can convert this to a sample count by multiplying by the appropriate samples-per-chunk.
*/
int WriteStscBox(CTrack *track)
{
    int index = 0, oldval = -1, i;
    int64_t llEntryPos, llCurpos;

    int64_t llPos = GetPos();
    WriteBE32( 0); /* size */
    WriteTag("stsc");
    WriteBE32( 0); // version & flags
    llEntryPos = GetPos();
    WriteBE32( 0); // entry count
    for (i=0; i<track->m_CrntChunk; i++) {
        if(oldval != track->m_ChunkList[i].samplesInChunk)
        {
            WriteBE32( i+1); // first chunk
            WriteBE32( track->m_ChunkList[i].samplesInChunk); // samples per chunk
            WriteBE32( 0x1); // sample description index
            oldval = track->m_ChunkList[i].samplesInChunk;
            index++;
        }
    }
    llCurpos = GetPos();
    SeekPos(llEntryPos);
    WriteBE32(index); // rewrite size
    SeekPos( llCurpos);

    return UpdateSize(llPos);
}

/* 
** Sample size atom 
** This box contains the sample count and a table giving the size in bytes of each sample. This allows the media
** data itself to be unframed. The total number of samples in the media is always indicated in the sample count.
*/
int WriteStszBox(CTrack *track)
{
    int equalChunks = 1;
    int i, j, entries = 0, tst = -1, oldtst = -1;;

    int64_t llPos = GetPos();
    WriteBE32(0); /* size */
    WriteTag("stsz");
    WriteBE32(0); /* version & flags */

	for (i=0; i<track->m_CrntChunk; i++) {
		tst = track->m_ChunkList[i].size/track->m_ChunkList[i].entries;
        if(oldtst != -1 && tst != oldtst) {
            equalChunks = 0;
        }
        oldtst = tst;
        entries += track->m_ChunkList[i].entries;
    }
    if (equalChunks) {
        int sSize = 0;
		if(track->m_CrntChunk){
			sSize = track->m_ChunkList[0].size/track->m_ChunkList[0].entries;
		}
        WriteBE32(sSize); // sample size
        WriteBE32(entries); // sample count
    }  else {
        WriteBE32( 0); // sample size
        WriteBE32(entries); // sample count
        for (i=0; i<track->m_CrntChunk; i++) {
            for (j=0; j<track->m_ChunkList[i].entries; j++) {
                WriteBE32(track->m_ChunkList[i].size / track->m_ChunkList[i].entries);
            }
        }
    }

    return UpdateSize(llPos);
}

/*
** Chunk Offset Box
** The chunk offset table gives the index of each chunk into the containing file
** Offsets are file offsets, not the offset into any box within the file
*/
int WriteStcoBox(CTrack *track)
{
    int i;
    int mode64 = 0; //   use 32 bit size variant if possible
    int64_t llPos = GetPos();
    WriteBE32(0); /* size */

    if (llPos > UINT32_MAX) {
        mode64 = 1;
        WriteTag("co64");
    } else
        WriteTag("stco");
    WriteBE32(0); /* Version and Flags */
    WriteBE32( track->m_CrntChunk); /* entry count */
    for (i=0; i<track->m_CrntChunk; i++) {
        if(mode64 == 1)
            WriteBE64(track->m_ChunkList[i].m_llPos);
        else
            WriteBE32(track->m_ChunkList[i].m_llPos);
    }
    return UpdateSize(llPos);
}

/*
*/

int WriteMvexBox(CTrack *track)
{
    int64_t llPos = GetPos();
    WriteBE32( 0); /* size */
    WriteTag("mvex");
    WriteMehd(track);
    WriteTrexBox(track);
    return UpdateSize(llPos);
}

/*
*/
int WriteMehd(CTrack *track)
{
    int i;
    int mode64 = 0; //   use 32 bit size variant if possible
    int64_t llPos = GetPos();
    WriteBE32(0); /* size */
	WriteTag("mehd");
    WriteBE32(0); /* Version and Flags */

	//fragment_duration
    WriteBE32(0);
    return UpdateSize(llPos);
}

/*
*/
int WriteTrexBox(CTrack *track)
{
    int i;
    int mode64 = 0; //   use 32 bit size variant if possible
    int64_t llPos = GetPos();
    WriteBE32(0); /* size */
	WriteTag("trex");
    WriteBE32(0); /* Version and Flags */

	///track_ID
	WriteBE32(track->trackID);
	//default_sample_description_index
	WriteBE32(1);
	//default_sample_duration
	WriteBE32(0);
	//default_sample_size
	WriteBE32(0);
	//default_sample_flags
	WriteBE32(0);

    return UpdateSize(llPos);
}

public:

virtual void AddChunkInfo(
		CTrack       *pTrk,
		int64_t      llChunkPos,
		int          nChunkSize,
		unsigned int samplesInChunk,
		int64_t      llPts,
		int64_t      llDts,
		int          fKeyFrame
		)
{
	if((m_lMdatSize + 0x20) != llChunkPos) {
		assert(0);
	}
	if(pTrk->m_ChunkList == 0){
		pTrk->m_ChunkList = (ChunkEntry *)malloc(MOV_INDEX_CLUSTER_SIZE * sizeof(ChunkEntry));
	}else  if (!(pTrk->m_CrntChunk % MOV_INDEX_CLUSTER_SIZE)) {
		// TODO: Resolve link error in Debug mode
#ifndef DEBUG
		int nNewSize = (pTrk->m_CrntChunk + MOV_INDEX_CLUSTER_SIZE) * sizeof(ChunkEntry);
        pTrk->m_ChunkList = (ChunkEntry *)realloc(pTrk->m_ChunkList, nNewSize);
		if (!pTrk->m_ChunkList){
			assert(0);
			return;
		}
#endif
    }

	pTrk->m_ChunkList[pTrk->m_CrntChunk].m_llPos = llChunkPos;
	pTrk->m_ChunkList[pTrk->m_CrntChunk].samplesInChunk = samplesInChunk;
	pTrk->m_ChunkList[pTrk->m_CrntChunk].size = nChunkSize;
	pTrk->m_ChunkList[pTrk->m_CrntChunk].entries = samplesInChunk;
	pTrk->m_ChunkList[pTrk->m_CrntChunk].m_llDts = llDts;
	pTrk->m_llTtrackDuration = llDts;

	if (llDts != llPts)
		pTrk->hasBframes = 1;
	pTrk->m_ChunkList[pTrk->m_CrntChunk].m_Cts = llPts - llDts;
	pTrk->m_ChunkList[pTrk->m_CrntChunk].key_frame = fKeyFrame;
	if(pTrk->m_ChunkList[pTrk->m_CrntChunk].key_frame)
		pTrk->hasKeyframes++;
	pTrk->m_CrntChunk++;
	pTrk->m_SampleCount += samplesInChunk;

	m_lMdatSize += nChunkSize;
	{
		long lPtsMs = llPts * ((double)m_lMovieTimescale / pTrk->m_lTimescale / 90);
		DbgOut(DBG_LVL_TRACE,("trackID=%d pts=%lld(%d ms)", pTrk->trackID, llPts, lPtsMs));
	}
}

virtual int  WritePacket(CPacket *pPkt)
{
    CTrack *pTrk = GetTrack(pPkt->m_StreamIndex);
    unsigned int samplesInChunk = 0;
	int fKeyFrame = 0;

	if(pTrk == NULL)
		return -1;

	if (pPkt->m_lSize == 0) 
		return 0; /* Discard 0 sized packets */

	// Always 1 sample for chunk
	samplesInChunk = 1;

    if (pTrk->m_CodecId == CODEC_ID_H264 ) {
		int64_t llChunkPos = GetPos();
		int64_t llPts = CovertMsToScaledUnits(pTrk->m_lTimescale, pPkt->m_llPts);
		int64_t llDts = CovertMsToScaledUnits(pTrk->m_lTimescale, pPkt->m_llDts);

        /* nal reformating needed */
        int nChunkSize = WriteNalUnits(pPkt->m_pData, &pPkt->m_lSize);
		if(nChunkSize) {
			fKeyFrame = (pPkt->m_Flags & PKT_FLAG_KEY);
			AddChunkInfo(pTrk, llChunkPos, nChunkSize, samplesInChunk, llPts, llDts, fKeyFrame);
		}
	} else if (pTrk->m_CodecId == CODEC_ID_AAC) {
		/* Process Audio */
		if(!pTrk->m_fAudCfgFound) {
			getAacConfigFromAdts((char *)pTrk->m_vosData, (char *)pPkt->m_pData);
			pTrk->m_vosLen = 2;
			pTrk->m_fAudCfgFound = 1;
		}
		WriteAacFrames(pTrk, pPkt);
	} else {
		int fUnknown = 0;
	}

    return 0;
}

unsigned long GetUtcTimeInSecs() 
{
#ifdef WIN32
	FILETIME FileTime;
	LARGE_INTEGER basetime;
	LARGE_INTEGER nowtime;
	GetSystemTimeAsFileTime(&FileTime);

	basetime.u.HighPart = 0x0153b281;
	basetime.u.LowPart = 0xe0fb4000;

	nowtime.u.HighPart = FileTime.dwHighDateTime;
	nowtime.u.LowPart = FileTime.dwLowDateTime;

	nowtime.QuadPart = nowtime.QuadPart - basetime.QuadPart;

	return ((long) (nowtime.QuadPart / 10000000));  //convert to seconds
#else
	// TODO: 
	return 0;
#endif
} /*filetimetoseconds*/

int  CreateTrack(int nIndex, int nTrackId)
{ 
	CTrack	 *pTrack = new CTrack;
	m_Tracks[nIndex] = pTrack;
	pTrack->trackID = nTrackId; //TRACK_INDEX_VIDEO;
	return 0;
}

int  InitVideoTrack(CTrack	*pTrack, unsigned char *pSpsData, long lSpsSize, unsigned char *pPpsData, long lPpsSize)
{ 
	long lWidth;
	long lHeight;

	H264::cParser Parser;
	if(Parser.ParseSequenceParameterSetMin(pSpsData,lSpsSize, &lWidth, &lHeight) == 0){
		pTrack->m_CodecId = CODEC_ID_H264;
		pTrack->m_CrntChunk = 0;
		pTrack->m_ChunkList = 0;
		pTrack->m_CodecType = CODEC_TYPE_VIDEO;

		strcpy((char *)pTrack->m_Tag, "avc1");//= 0x61766331; //avc1;
		pTrack->time = GetUtcTimeInSecs();
		// TODO
		pTrack->m_lTimescale = 90000;
	
		// TODO : Get it from SPS
		pTrack->m_lWidth = lWidth;
		// TODO : Get it from SPS
		pTrack->m_lHeight = lHeight;
		m_lMovieTimescale = 90000;

		UpdateAvccSps(&pTrack->AvcCfgRecord, pSpsData, lSpsSize);
		pTrack->m_fSpsFound = 1;
		UpdateAvccPps(&pTrack->AvcCfgRecord, pPpsData, lPpsSize);

	}
	return 0;
}

int  InitAudioTrack(CTrack *pTrack, unsigned char *pConfigData, long lSize)
{ 
	// TODO
	int nNumChannels = 2;
	int nSamplesPerSec = 48000;

	pTrack->m_CodecId = CODEC_ID_AAC;
	pTrack->m_CrntChunk = 0;
	pTrack->m_ChunkList = 0;
	pTrack->m_CodecType = CODEC_TYPE_AUDIO;

	strcpy((char *)pTrack->m_Tag, "mp4a");//= 0x61766331; //avc1;
	// TODO
	pTrack->m_lTimescale = nSamplesPerSec;
	pTrack->m_Samplerate = nSamplesPerSec;
	pTrack->m_Channels = nNumChannels;
	pTrack->time = GetUtcTimeInSecs();

	memcpy(pTrack->m_vosData, pConfigData, lSize);
	pTrack->m_vosLen = 2;
	pTrack->m_fAudCfgFound = 1;

	return 0;
}

int  DeinitVideoTrack()
{ 
	std::map<int, CTrack	 *>::iterator it;
	it = m_Tracks.find(TRACK_INDEX_VIDEO);
	if(it != m_Tracks.end()) {
		CTrack	 *pTrack = it->second;
		pTrack->m_CodecId = CODEC_ID_H264;
		if(pTrack->m_ChunkList){
			free(pTrack->m_ChunkList);
		}
		m_Tracks.erase(it);
	}
	return 0;
}
int  DeinitAudioTrack()
{ 
	std::map<int, CTrack	 *>::iterator it;
	it = m_Tracks.find(TRACK_INDEX_AUDIO);
	if(it != m_Tracks.end()) {
		CTrack	 *pTrack = it->second;
		pTrack->m_CrntChunk = 0;
		if(pTrack->m_ChunkList){
			free(pTrack->m_ChunkList);
			pTrack->m_ChunkList = 0;
		}
	}
	return 0;
}

	int GenerateInitSegment(char *pBuff, int nMaxLen)
	{
		int nLen = 0;
		m_pWriter = new CBuffWriter(pBuff, nMaxLen);
		WriteFtypBox();
		WriteMoovBox();
		nLen = m_pWriter->GetPos();
		delete m_pWriter;
		return nLen;
	}

CMp4Mux()
{
	m_lMdatSize = 0;
	m_NbStreams = 2;
	// TODO

	m_lTime = GetUtcTimeInSecs();
	m_MaxTrackID = 1;
	// TODO
	m_llMovieDuration =  0;//100000 * 600;
}

CTrack *GetTrack(int nIndex)
{
	CTrack *pTrack = NULL;
	std::map<int, CTrack *>::iterator it;
	it = m_Tracks.find(nIndex);
	if(it != m_Tracks.end()) {
		pTrack = it->second;
	}
	return pTrack;
}
	unsigned long AssignTrackID()
	{
		unsigned long ulTrackId =  m_MaxTrackID;
		m_MaxTrackID++;
		return ulTrackId;
	}
protected:

	unsigned char *m_pBuff;
	unsigned char *m_pBuffEnd;
	int64_t	 m_llMdatPos;
	int64_t	 m_llMoovPos;
	
	unsigned long 	m_NbStreams;
	unsigned long m_lMdatSize;
	unsigned long m_lTime;

	int64_t	 m_llMovieDuration;
	unsigned long m_MaxTrackID;
    long     m_lMovieTimescale;
	std::map <int, CTrack *> m_Tracks;
}; //class CMp4Mux


class CMp4Moof : public CMp4MuxBase
{
public:
	CMp4Moof(CMp4Mux *pInitSegment)
	{
		m_pMp4Mux = pInitSegment;
		m_pWriter = NULL;
		m_nMoofSeqNum = 0;
		m_llVidPts = 0;
		m_llVidDts = 0;
		m_llVidStartDts = 0;
		m_pMp4SidxParent = NULL;
		m_pMp4SidxChild = NULL;
		m_pSidxReference = NULL;
	}

	virtual ~CMp4Moof()
	{
	}
#if 0
	int GetData()
	{
		WriteFtypBox();
		WriteMoovBox();
	}
#endif
/*
** The Movie Fragment Box
** The Movie Fragment Box is a top-level box, (i.e. a peer to the Movie Box and Media Data boxes). It contains
** a Movie Fragment Header Box, and then one or more Track Fragment Boxes.
*/
int WriteMoofBox(CMoof *pMoof)
{
    int mode64 = 0; //   use 32 bit size variant if possible
    int64_t llPos = GetPos();
    WriteBE32(0); /* size */
    WriteTag("moof");
    WriteMfhdBox(pMoof);

	for(std::map<int, CTrackFragment *>::iterator it=pMoof->m_Trafs.begin(); it != pMoof->m_Trafs.end(); it++) {
		CTrackFragment *pTraf = it->second;
		WriteTrafBox(pTraf);
	}
    return UpdateSize(llPos);
}

/*
** The Movie Fragment Box
** The movie fragment header contains a sequence number, as a safety check. The sequence number usually
** starts at 1 and must increase for each movie fragment in the file, in the order in which they occur. This allows
**r eaders to verify integrity of the sequence; it is an error to construct a file where the fragments are out of
** sequence.
*/
int WriteMfhdBox(CMoof *pMoof)
{
    int mode64 = 0; //   use 32 bit size variant if possible
    int64_t llPos = GetPos();
    WriteBE32(0); /* size */
    WriteTag("mfhd");
	WriteBE32(0); /* version & flags */
	WriteBE32(pMoof->sequence_number);
    return UpdateSize(llPos);
}

/*
*/
int WriteTrafBox(CTrackFragment *pTraf)
{
    int mode64 = 0; //   use 32 bit size variant if possible
    int64_t llPos = GetPos();
    WriteBE32(0); /* size */
    WriteTag("traf");
	WriteTfhdBox(pTraf);
	WriteTfdtBox(pTraf);
	WriteTrunBox(pTraf);
    return UpdateSize(llPos);
}


/*
** The Movie Fragment Box
** The movie fragment header contains a sequence number, as a safety check. The sequence number usually
** starts at 1 and must increase for each movie fragment in the file, in the order in which they occur. This allows
**r eaders to verify integrity of the sequence; it is an error to construct a file where the fragments are out of
** sequence.
*/
int WriteTfhdBox(CTrackFragment *pTraf)
{
    int mode64 = 0; //   use 32 bit size variant if possible
    int64_t llPos = GetPos();
    WriteBE32(0); /* size */
    WriteTag("tfhd");
	WriteBE32(pTraf->tf_flags); /* version & flags */
	WriteBE32(pTraf->track_ID);
    return UpdateSize(llPos);
}

/*
** track fragment decode time Box
*/
int WriteTfdtBox(CTrackFragment *pTraf)
{
    int mode64 = 0; //   use 32 bit size variant if possible
    int64_t llPos = GetPos();
    WriteBE32(0); /* size */
    WriteTag("tfdt");
	WriteBE32(0); /* version & flags */
	WriteBE32(pTraf->decode_time); /* version & flags */
    return UpdateSize(llPos);
}

/*
** The Track Fragment Run Box
** The movie fragment header contains a sequence number, as a safety check. The sequence number usually
** starts at 1 and must increase for each movie fragment in the file, in the order in which they occur. This allows
** readers to verify integrity of the sequence; it is an error to construct a file where the fragments are out of
** sequence.
*/
int WriteTrunBox(CTrackFragment *pTraf)
{
	CTrackRun *pTrun = pTraf->m_pTrun;
	std::vector<CSample *> *parSamples =  &pTrun->m_arSamples;
    int mode64 = 0; //   use 32 bit size variant if possible
    int64_t llPos = GetPos();
    WriteBE32(0); /* size */
    WriteTag("trun");
	WriteBE32(pTrun->tr_flags); /* version & flags */
	pTrun->sample_count = parSamples->size();
	WriteBE32(pTrun->sample_count);
	pTrun->ulAddr_data_offset = GetPos();
	WriteBE32(0/*pTrun->data_offset*/);

	if(pTrun->first_sample_flags && TRUN_FIRST_SAMPLE_FLAG_PRESENT) {
		// TODO:
	}
	for(std::vector<CSample *>::iterator it =  parSamples->begin(); it !=  parSamples->end(); it++){
		CSample *pSample = *it;
		if(pTrun->tr_flags & TRUN_SAMPLE_DURATION_PRESENT) {
			WriteBE32(pSample->sample_duration);
		}
		if(pTrun->tr_flags & TRUN_SAMPLE_SIZE_PRESENT) {
			WriteBE32(pSample->sample_size);
		}
		if(pTrun->tr_flags & TRUN_SAMPLE_FLAGS_PRESENT) {
			WriteBE32(pSample->sample_flags);
		}
		if(pTrun->tr_flags & TRUN_SAMPLE_COMP_TIME_OFFSET_PRESENT) {
			WriteBE32(pSample->sample_composition_time_offset);
		}
	}
    return UpdateSize(llPos);
}

void UpdateTrunBoxDataOffset(CTrackFragment *pTraf, unsigned long ulDataOffset)
{
	CTrackRun *pTrun = pTraf->m_pTrun;
	UpdateSize(pTrun->ulAddr_data_offset, ulDataOffset);
}

int WriteMdatBox(unsigned long lDataSize)
{
    WriteBE32(lDataSize + 8);		/* for size */
    WriteTag("mdat");
    return 0;
}

void WriteSidxReference(CSidxReference *pReference)
{
	pReference->m_pos_referenced_size = GetPos();
	WriteBE32(pReference->m_reference_type | pReference->m_referenced_size);
	WriteBE32(pReference->m_subsegment_duration);
	WriteBE32((pReference->m_starts_with_SAP << 31) | (pReference->m_SAP_type << 28) |  pReference->m_SAP_delta_time);
}

void UpdateSidxReference(CSidxReference *pReference, unsigned long referenced_size)
{
	pReference->m_referenced_size = referenced_size;
	UpdateSize(pReference->m_pos_referenced_size, referenced_size);
}

int WriteSidxBox(CMp4Sidx *pSidx)
{
	if(pSidx) {
		int mode64 = 0; //   use 32 bit size variant if possible
		int nChildSidx = pSidx->m_SidxList.size();
		long  lPos = GetPos();
		WriteBE32(0); /* size */
		WriteTag("sidx");
		WriteBE32(0); /* version & flags */

		WriteBE32(pSidx->m_reference_ID);
		WriteBE32(pSidx->m_timescale);
		WriteBE32(pSidx->m_earliest_presentation_time);
		pSidx->m_pos_first_offset = GetPos();
		WriteBE32(pSidx->m_first_offset);
		WriteBE16(pSidx->reserved);

		// Write reference
		if(pSidx->m_SidxList.size()) {
			// Parent Sidx
			unsigned short reference_count = pSidx->m_SidxList.size();
			WriteBE16(reference_count);
			for (std::vector<CMp4Sidx *>::iterator it = pSidx->m_SidxList.begin(); it != pSidx->m_SidxList.end(); it++) {
				CMp4Sidx *pChildSidx = *it;
				if(pChildSidx->m_pReference)
					WriteSidxReference(pChildSidx->m_pReference);
			}
		} else {
			// Child Sidx
			if(pSidx->m_pReference)
				WriteSidxReference(pSidx->m_pReference);
		}
		return UpdateSize(lPos);
	}
	return 0;
}

void UpdateSidxBox(CMp4Sidx *pSidx, unsigned long first_offset) 
{
	pSidx->m_first_offset = first_offset;
	UpdateSize(pSidx->m_pos_first_offset,  first_offset);
}

	void UpdateTrafInfo(
	    int          nTrackId,
		unsigned long  ulFlags,
		unsigned long  ulSampleDuration,
		unsigned long  ulSampleSize,
		unsigned long  ulCtsoffset);

	int WriteNalUnits(
		  unsigned char			*pData, 
		  int					*size);

	int WriteAacFrames(CPacket  *pPkt);

	int  WritePacket(CPacket *pPkt);
	void *OpenSidx();
	int  CloseSidx(void *pSidx);
	void *OpenMoof(char *pMoofBuff, int nMaxMoofLen, char *pAudMdat, int nMaxAudLen, char *pVidMdat, int nMaxVidLen, int nTracks);
	int CloseMoof(void *pMoof, CMoofParam *pParam);

protected:
	CMoof         *m_pMoof;
	CMp4Mux       *m_pMp4Mux;
	CMp4Sidx      *m_pMp4SidxParent;
	CMp4Sidx      *m_pMp4SidxChild;
	CSidxReference *m_pSidxReference;

	int           m_nMoofSeqNum;
	CBuffWriter   *m_pAudMdatWr;
	CBuffWriter   *m_pVidMdatWr;
	long long      m_llVidPts;
	long long      m_llVidDts;
	long long      m_llVidStartDts;
	long long      m_llMoofDecodeTime;
};

void *CMp4Moof::OpenSidx()
{
	CSidxReference *pSidxReference = new CSidxReference(1,0,0,0,0,0);
	m_pMp4SidxParent = new CMp4Sidx(1, 90000,0,0);
	return m_pMp4SidxParent;
}

int CMp4Moof::CloseSidx(void *pSidx)
{
	if(m_pMp4SidxParent) {
		WriteSidxBox(m_pMp4SidxParent);
	}
	for (std::vector<CMp4Sidx *>::iterator it = m_pMp4SidxParent->m_SidxList.begin(); it != m_pMp4SidxParent->m_SidxList.end(); it++) {
		WriteSidxBox(*it);
	}

	delete m_pMp4SidxParent;
	return 0;
}

void *CMp4Moof::OpenMoof(char *pMoofBuff, int nMaxMoofLen, char *pAudMdat, int nMaxAudLen, char *pVidMdat, int nMaxVidLen, int nTracks)
{
	m_llMoofDecodeTime = m_llVidDts - m_llVidStartDts;
	m_pWriter = new CBuffWriter(pMoofBuff, nMaxMoofLen);
	m_pMoof = new CMoof();
	m_pMp4SidxChild = NULL;
	m_pVidMdatWr = NULL;
	m_pAudMdatWr = NULL;
	if(pVidMdat) {
		m_pMoof->AddTraf(CMp4MuxIf::TRACK_INDEX_VIDEO, m_pMp4Mux->GetTrack(CMp4MuxIf::TRACK_INDEX_VIDEO), m_llMoofDecodeTime);
		m_pVidMdatWr = new CBuffWriter(pVidMdat, nMaxVidLen);
		if(m_pMp4SidxParent) {
			m_pMp4SidxChild = new CMp4Sidx(1, 90000, m_llMoofDecodeTime, 0);
			m_pMp4SidxParent->AddChildSidx(m_pMp4SidxChild);
		}
	}

	if(pAudMdat) {
		m_pMoof->AddTraf(CMp4MuxIf::TRACK_INDEX_AUDIO, m_pMp4Mux->GetTrack(CMp4MuxIf::TRACK_INDEX_AUDIO), m_llMoofDecodeTime);
		m_pAudMdatWr = new CBuffWriter(pAudMdat, nMaxAudLen);
	}

	m_pMoof->sequence_number = m_nMoofSeqNum++;

	return m_pMoof;
}

int CMp4Moof::CloseMoof(void *pMoof, CMoofParam *pParam)
{
	unsigned long ulVidMdat = 0;
	unsigned long ulAudMdat = 0;
	unsigned long ulMoofSize = 0;
	unsigned long ulMoofStart = 0;
	int nLen = 0;
	CTrack *pTrack = m_pMp4Mux->GetTrack(CMp4MuxIf::TRACK_INDEX_VIDEO);

	if(m_pMoof->sequence_number == 0) {
		if(m_pMp4SidxParent) {
			CSidxReference *pSidxReference = new CSidxReference(0,0,0,0,0,0);
			m_pMp4SidxParent->SetReference(pSidxReference);
		}
	}
	if(m_pMp4SidxChild) {
		m_pSidxReference = new CSidxReference(0,0,0,0,0,0);
		m_pMp4SidxChild->SetReference(m_pSidxReference);
	}

	ulMoofStart = GetPos();
	WriteMoofBox(m_pMoof);

	// Copy Video Data
	if (m_pVidMdatWr && m_pVidMdatWr->GetPos() > 0) {
		WriteMdatBox(m_pVidMdatWr->GetPos());
		ulVidMdat = m_pWriter->GetPos();
		m_pWriter->WriteBuffer((unsigned char *)m_pVidMdatWr->m_pBuff, m_pVidMdatWr->GetPos());
	}

	// Copy Audio Data
	if (m_pAudMdatWr && m_pAudMdatWr->GetPos() > 0) {
		WriteMdatBox(m_pAudMdatWr->GetPos());
		ulAudMdat = m_pWriter->GetPos();
		m_pWriter->WriteBuffer((unsigned char *)m_pAudMdatWr->m_pBuff, m_pAudMdatWr->GetPos());
	}
	// Update mdat offsets
	{
		CTrackFragment *pTraf;
		
		pTraf = m_pMoof->GetTraf(CMp4MuxIf::TRACK_INDEX_VIDEO);
		if(pTraf){
			UpdateTrunBoxDataOffset(pTraf, ulVidMdat);
		}

		pTraf = m_pMoof->GetTraf(CMp4MuxIf::TRACK_INDEX_AUDIO);
		if(pTraf){
			UpdateTrunBoxDataOffset(pTraf, ulAudMdat);
		}
	}
	if(m_pMp4SidxChild) {
		if(m_pSidxReference)	{
			ulMoofSize = GetPos() - ulMoofStart;
			UpdateSidxReference(m_pSidxReference, ulMoofSize);
		}
		UpdateSidxBox(m_pMp4SidxChild, 0);
	}
	nLen = m_pWriter->GetPos();
	if(pParam) {
		pParam->m_nSize = nLen;
		pParam->m_nStartPtsMs = CovertScaledUnitsToMs(pTrack->m_lTimescale, m_llMoofDecodeTime); // (1000/90000);
	}
	delete m_pWriter;
	delete m_pMoof;
	return nLen;
}

int CreateEmsg(char *pEmsgData, int nEmsgMaxLen, 
			const char *scheme_id_uri, const char *value,
			unsigned long presentation_time_delta, unsigned long event_duration,
			unsigned long id, 
			unsigned char *message_data, int message_len)
{
	int EmsgLen = 0;
	CBuffWriter *pWriter = new CBuffWriter(pEmsgData, nEmsgMaxLen);
	long  lStartPos = pWriter->GetPos();
	pWriter->WriteBE32(0); /* size */
	pWriter->WriteTag("emsg");
	pWriter->WriteBE32(0); /* version & flags */
	pWriter->WriteStringWithNull(scheme_id_uri);
	pWriter->WriteStringWithNull(value);
	pWriter->WriteBE32(1); // time scale seconds
	pWriter->WriteBE32(presentation_time_delta);
	pWriter->WriteBE32(event_duration);
	pWriter->WriteBE32(id);
	if(message_len) {
		pWriter->WriteBuffer(message_data, message_len);
	}
	pWriter->UpdateSize(lStartPos);
	EmsgLen = pWriter->GetPos();
	return EmsgLen;
}

void CMp4Moof::UpdateTrafInfo(
	    int          nTrackId,
		unsigned long  ulFlags,
		unsigned long  ulSampleDuration,
		unsigned long  ulSampleSize,
		unsigned long  ulCtsoffset
		)
{
	CTrackRun *pTran = m_pMoof->GetTraf(nTrackId)->m_pTrun;
	pTran->AddSampleInf(ulFlags,ulSampleDuration,ulSampleSize,ulCtsoffset);
}

int CMp4Moof::WriteNalUnits(
	  unsigned char			*pData, 
	  int					*size)
{
    unsigned char *p = pData;
    unsigned char *end = p + *size;
    unsigned char *pNalStart, *pNalEnd;
	int lNalSize = 0;
	long lBytesWritten = 0;
    pNalStart = FindStartCode(p, end);
	CTrackFragment	 *pTraf = m_pMoof->GetTraf(CMp4MuxIf::TRACK_INDEX_VIDEO);

	if(pTraf == NULL)
		return -1;

    while (pNalStart < end) {
		/* Skip Start code */
		while(!*(pNalStart++));
        
		pNalEnd = FindStartCode(pNalStart, end);
		lNalSize = pNalEnd - pNalStart;
		m_pVidMdatWr->WriteBE32(lNalSize);
		m_pVidMdatWr->WriteBuffer(pNalStart, lNalSize);
		lBytesWritten += (4 + lNalSize);
        pNalStart = pNalEnd;
    }
    return lBytesWritten;
}

int CMp4Moof::WriteAacFrames(CPacket  *pPkt)
{
    CTrack *pTrk = m_pMp4Mux->GetTrack(pPkt->m_StreamIndex);
    unsigned char *pFrameStart =  pPkt->m_pData;
	unsigned char *pDataEnd = pFrameStart + pPkt->m_lSize;
	int nFrameSize = 0;
	long lBytesWritten = 0;

	if(pTrk == NULL)
		return -1;

	int64_t llPts = CovertMsToScaledUnits(pTrk->m_lTimescale, pPkt->m_llPts);
	int64_t llDts = llPts;

    while ((pFrameStart + 7) < pDataEnd) {
		nFrameSize = getAdtsFrameLen(pFrameStart);
		int nAdtsHdrSize = 7 + (( pFrameStart[1] & 0x01 ) ? 0 : 2 );
		unsigned char *pData = pFrameStart + nAdtsHdrSize;
		int64_t llChunkPos = GetPos();
		int nSize = nFrameSize - nAdtsHdrSize;
		assert(nSize);
		m_pAudMdatWr->WriteBuffer(pData, nSize);
		UpdateTrafInfo(pPkt->m_StreamIndex, TRUN_SAMPLE_SIZE_PRESENT, 0, nSize, 0);
		llPts += 1024;	// 1024 samples
		llDts = llPts;
		lBytesWritten += nSize;
		pFrameStart += nFrameSize;
    }
    return lBytesWritten;
}

int  CMp4Moof::WritePacket(CPacket *pPkt)
{
    CTrack *pTrk = m_pMp4Mux->GetTrack(pPkt->m_StreamIndex);
	CTrackFragment *pTraf = m_pMoof->GetTraf(pPkt->m_StreamIndex);
	int fKeyFrame = 0;

	if(pTrk == NULL || pTraf == NULL)
		return -1;
	if (pPkt->m_lSize == 0) 
		return 0; /* Discard 0 sized packets */

    if (pTrk->m_CodecId == CODEC_ID_H264 ) {
		int nFrameDuration = pTrk->m_lTimescale / 30;//3003; // initialize with default. This will be used only for the first frame
		int nCompositionTimeOffset = 0;
		int64_t llChunkPos = GetPos();
		int64_t llPts = CovertMsToScaledUnits(pTrk->m_lTimescale, pPkt->m_llPts);
		int64_t llDts = CovertMsToScaledUnits(pTrk->m_lTimescale, pPkt->m_llDts);

		if(m_llVidPts != 0) {
			nFrameDuration = llPts - m_llVidPts; 
		}
		if(m_llVidDts != 0)
			nCompositionTimeOffset = llDts - llPts;
		if(nCompositionTimeOffset < pTrk->m_lTimescale || nCompositionTimeOffset > pTrk->m_lTimescale)
			nCompositionTimeOffset = 0;
		m_llVidPts = llPts; 
		m_llVidDts = llDts;
		if(m_llVidStartDts == 0)
			m_llVidStartDts = m_llVidDts;
        /* nal reformating needed */
        int nFrameSize = WriteNalUnits(pPkt->m_pData, &pPkt->m_lSize);
		if(nFrameSize) {
			fKeyFrame = (pPkt->m_Flags & PKT_FLAG_KEY);
			UpdateTrafInfo(pPkt->m_StreamIndex, TRUN_SAMPLE_SIZE_PRESENT, nFrameDuration, nFrameSize, 0);
		}
	} else if (pTrk->m_CodecId == CODEC_ID_AAC) {
		/* Process Audio */
		if(!pTrk->m_fAudCfgFound) {
			getAacConfigFromAdts((char *)pTrk->m_vosData, (char *)pPkt->m_pData);
			pTrk->m_vosLen = 2;
			pTrk->m_fAudCfgFound = 1;
		}
		WriteAacFrames(pPkt);
	} else {
		int fUnknown = 0;
	}

    return 0;
}


#define MAX_MOOF_DAT             (256 * 1024)
#define MAX_MOOF_AUD_MDAT        (1024 * 1024)
#define MAX_MOOF_VID_MDAT        (4 * 1024 * 1024)

class CMp4Segmenter : public CMp4MuxIf
{
public:
	CMp4Segmenter(int fEnableAud, int fEnableVid)
	{
		m_pMp4Mux = new CMp4Mux;
		m_pMp4Moof = new CMp4Moof(m_pMp4Mux);

		m_fEnableVid = fEnableVid;
		m_fEnableAud = fEnableAud;
		m_pAudMdat = NULL;
		m_pVidMdat = NULL;
		if(fEnableVid) {
			m_pVidMdat = (char *)malloc(MAX_MOOF_VID_MDAT);
			m_pMp4Mux->CreateTrack(TRACK_INDEX_VIDEO, m_pMp4Mux->AssignTrackID());
		}
		if(fEnableAud) {
			m_pAudMdat = (char *)malloc(MAX_MOOF_AUD_MDAT);
			m_pMp4Mux->CreateTrack(TRACK_INDEX_AUDIO, m_pMp4Mux->AssignTrackID());
		}
	}
	~CMp4Segmenter()
	{
		delete m_pMp4Mux;
		delete m_pMp4Moof;
		if(m_pAudMdat)
			free(m_pAudMdat);
		if(m_pVidMdat)
			free(m_pVidMdat);
	}

	int InitVideoTrack(unsigned char *pSpsData, long lSpsSize, unsigned char *pPpsData, long lPpsSize) 
	{ 
		return m_pMp4Mux->InitVideoTrack(m_pMp4Mux->GetTrack(TRACK_INDEX_VIDEO), pSpsData, lSpsSize, pPpsData,lPpsSize);
	}
	int InitAudioTrack(unsigned char *pConfigData, long lSize) 
	{
		return m_pMp4Mux->InitAudioTrack(m_pMp4Mux->GetTrack(TRACK_INDEX_AUDIO), pConfigData, lSize);
	}

	int GenerateInitSegment(char *pBuff, int nMaxLen)
	{
		return m_pMp4Mux->GenerateInitSegment(pBuff, nMaxLen);
	}

	void *OpenSidx()
	{
		return m_pMp4Moof->OpenSidx();
	}
	int CloseSidx(void *pSidx)
	{
		return m_pMp4Moof->CloseSidx(pSidx);
	}

	void *OpenMoof(char *pBuff, int nMaxLen, int nTracks)
	{
		return m_pMp4Moof->OpenMoof(pBuff, nMaxLen, m_pAudMdat,  MAX_MOOF_AUD_MDAT, m_pVidMdat, MAX_MOOF_VID_MDAT,  nTracks);
	}
	int CloseMoof(void *pMoof, CMoofParam *pParam)
	{
		return m_pMp4Moof->CloseMoof(pMoof, pParam);
	}

	int WritePacket(CPacket *pPkt) 
	{
		return m_pMp4Moof->WritePacket(pPkt);
	}

private:
	CMp4Mux  *m_pMp4Mux;
	CMp4Sidx *m_pMp4Sidx;
	CMp4Moof *m_pMp4Moof;
	char     *m_pAudMdat;
	char     *m_pVidMdat;
	int       m_fEnableVid;
	int       m_fEnableAud;
};

CMp4MuxIf *GetMp4FileWriter()
{
	return new CMp4Mux;
}


CMp4MuxIf *CreateMp4Segmenter(int fEnableAud, int fEnableVid)
{
	return new CMp4Segmenter(fEnableAud, fEnableVid);
}

void DeleteMp4Segmenter(CMp4MuxIf *pSegmenter)
{
	delete pSegmenter;
}