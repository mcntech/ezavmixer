#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include <sys/types.h>
#include <fcntl.h>
#include <io.h>
#include "Mp4DemuxIf.h"
//#include "Dbg.h"

using namespace Mp4Demux;

#define MAX_TRACKS			2
#define MAX_VOS_DATA_SIZE	256
#define INT32_MAX			0x7FFFFFFF
#define UINT32_MAX			0xFFFFFFFF
#define INT64_MAX			0x7FFFFFFFFFFFFFFF
#define ADTS_HEADER_LEN     (7)

#define AV_NOPTS_VALUE		-1
#define MOV_INDEX_CLUSTER_SIZE 16384

#define IS_START_CODE(x)  (x[0] == 0x00 && x[1] == 0x00 && x[2] == 0x00 && x[3] == 0x01)

//#define USE_RESERVED_AREA
#define MOOV_RESERVED_BYTES		(1024 * 1024)

#ifdef BIG_ENDIAN
#define MAKE_FOURCC( a, b, c, d ) \
        ( ((unsigned long)d) | ( ((unsigned long)c) << 8 ) \
           | ( ((unsigned long)b) << 16 ) | ( ((unsigned long)a) << 24 ) )
#else
#define MAKE_FOURCC( a, b, c, d ) \
        ( ((unsigned long)a) | ( ((unsigned long)b) << 8 ) \
           | ( ((unsigned long)c) << 16 ) | ( ((unsigned long)d) << 24 ) )
#endif

#define ATOM_FTYP MAKE_FOURCC( 'f', 't', 'y', 'p' )
#define ATOM_MDAT MAKE_FOURCC( 'm', 'd', 'a', 't' )
#define ATOM_MOOV MAKE_FOURCC( 'm', 'o', 'o', 'v' )

#define ATOM_TRAK MAKE_FOURCC( 't', 'r', 'a', 'k' )
#define ATOM_MVHD MAKE_FOURCC( 'm', 'v', 'h', 'd' )
#define ATOM_TKHD MAKE_FOURCC( 't', 'k', 'h', 'd' )
#define ATOM_MDIA MAKE_FOURCC( 'm', 'd', 'i', 'a' )
#define ATOM_MDHD MAKE_FOURCC( 'm', 'd', 'h', 'd' )
#define ATOM_HDLR MAKE_FOURCC( 'h', 'd', 'l', 'r' )
#define ATOM_MINF MAKE_FOURCC( 'm', 'i', 'n', 'f' )
#define ATOM_VMHD MAKE_FOURCC( 'v', 'm', 'h', 'd' )
#define ATOM_SMHD MAKE_FOURCC( 's', 'm', 'h', 'd' )
#define ATOM_DINF MAKE_FOURCC( 'd', 'i', 'n', 'f' )
#define ATOM_URL  MAKE_FOURCC( 'u', 'r', 'l', ' ' )
#define ATOM_urn  MAKE_FOURCC( 'u', 'r', 'n', ' ' )
#define ATOM_DREF MAKE_FOURCC( 'd', 'r', 'e', 'f' )
#define ATOM_STBL MAKE_FOURCC( 's', 't', 'b', 'l' )
#define ATOM_STTS MAKE_FOURCC( 's', 't', 't', 's' )
#define ATOM_CTTS MAKE_FOURCC( 'c', 't', 't', 's' )
#define ATOM_STSD MAKE_FOURCC( 's', 't', 's', 'd' )
#define ATOM_STSZ MAKE_FOURCC( 's', 't', 's', 'z' )
#define ATOM_STSC MAKE_FOURCC( 's', 't', 's', 'c' )
#define ATOM_STCO MAKE_FOURCC( 's', 't', 'c', 'o' )
#define ATOM_CO64 MAKE_FOURCC( 'c', 'o', '6', '4' )
#define ATOM_STSS MAKE_FOURCC( 's', 't', 's', 's' )

#define ATOM_ESDS MAKE_FOURCC( 'e', 's', 'd', 's' )
#define ATOM_MP4A MAKE_FOURCC( 'm', 'p', '4', 'a' )
#define ATOM_vide MAKE_FOURCC( 'v', 'i', 'd', 'e' )
#define ATOM_soun MAKE_FOURCC( 's', 'o', 'u', 'n' )
#define ATOM_hint MAKE_FOURCC( 'h', 'i', 'n', 't' )
#define ATOM_ISOM MAKE_FOURCC( 'i', 's', 'o', 'm' )
#define ATOM_AVC1 MAKE_FOURCC( 'a', 'v', 'c', '1' )
#define ATOM_AVCC MAKE_FOURCC( 'a', 'v', 'c', 'C' )

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

typedef struct _ChunkOffset {
	unsigned int  m_startSample;
    int64_t       m_llPos;
} ChunkOffset;

class CTrack : public CTrackInf
{
public:
	CTrack()
	{
		m_CrntSample = 0;
		m_ChunkList = NULL;
		m_CrntChunk = 0;
		m_MaxChunk = 0;
		m_ChunkOffsetTbl = NULL;
		m_AvgBitrate = 0;
		// TODO
		m_BufferSize = 1024*1024;
		m_AvgBitrate = 0;
		m_MinBitrate = 0;
		m_MaxBitrate = 0;
		m_AudioVbr = 1;
	}
	int GetNextChunkTime(int64_t &ullDts)
	{
		if(m_CrntSample < m_MaxSample){
			ullDts = m_ChunkList[m_CrntSample].m_llDts;
			return 0;
		} else {
			return -1;
		}
	}
    
	int64_t GetSampleOffset()
	{
		int64_t llOffset = m_ChunkOffsetTbl[m_CrntChunk].m_llPos;
		int64_t llSize = 0;
		for (int i= m_ChunkOffsetTbl[m_CrntChunk].m_startSample; i < m_CrntSample; i++) {
			llSize += m_ChunkList[i].size;
		}
		return (llOffset + llSize);
	}

	int  AdvanceSample()
	{
		m_CrntSample++;
		if(m_CrntSample < m_MaxSample && m_CrntChunk < m_MaxChunk - 1) {
			if(m_CrntSample >= m_ChunkOffsetTbl[m_CrntChunk + 1].m_startSample)
				m_CrntChunk++;
		}
		return 0;
	}

    int           m_Mode;
    unsigned int  m_CrntSample;
    unsigned int  m_MaxSample;

    unsigned int  m_CrntChunk;
    unsigned int  m_MaxChunk;

    int64_t       time;

    unsigned int  language;
    unsigned int  m_trackID;

    //AVCodecContext *enc;
	unsigned int      m_lWidth;
	unsigned int      m_lHeight;
	unsigned char     m_szName[32];
	
	CAacCfgRecord     AacCfgRecord; // SPecific to Audio track
	CAvcCfgRecord     AvcCfgRecord;	// Specific to Video track
    ChunkEntry        *m_ChunkList;
	ChunkOffset       *m_ChunkOffsetTbl; 
    int               m_AudioVbr;



	unsigned int	 m_Samplerate;
	unsigned int     m_Channels;
	// Audio
	unsigned int     m_AvgBitrate;
	unsigned int     m_MinBitrate;
	unsigned int     m_MaxBitrate;
	unsigned int     m_BufferSize;
};



class CMp4Demux : public CMp4DemuxIf
{
void ReadByte(unsigned char &b)
{
	ReadBuffer(&b, 1);
}

void CheckByte(unsigned char b)
{
	unsigned char _b;
	ReadBuffer(&_b, 1);
	// TODO Check
}

void ReadBE16(unsigned int &usVal)
{
	unsigned char ulTmp[2];
	ReadBuffer(ulTmp, 2);

    usVal = ((unsigned long)(ulTmp[0] << 8))  & 0x0000FF00 |
			((unsigned long)ulTmp[1]) & 0x000000FF;
}

void CheckBE16(unsigned int usVal)
{
	unsigned short usTmp;
	ReadBuffer((unsigned char *)&usTmp, 2);
}

void WriteBE24(unsigned int &ulVal)
{
	unsigned char ulTmp[3];
	ReadBuffer(ulTmp, 3);

    ulVal = ((unsigned long)(ulTmp[0] << 16)) & 0x00FF0000 |
		((unsigned long)(ulTmp[1] << 8))  & 0x0000FF00 |
		((unsigned long)ulTmp[2]) & 0x000000FF;
}
void CheckBE24(unsigned int ulVal)
{
	unsigned char ulTmp[3];
	ReadBuffer(ulTmp, 3);

}

void ReadBE32(unsigned int &ulVal)
{
	unsigned char ulTmp[4];
	ulVal = 0;
	if(ReadBuffer(ulTmp, 4)) {
		ulVal = 
			((unsigned long)(ulTmp[0] << 24)) & 0xFF000000 |
			((unsigned long)(ulTmp[1] << 16)) & 0x00FF0000 |
			((unsigned long)(ulTmp[2] << 8))  & 0x0000FF00 |
			((unsigned long)ulTmp[3]) & 0x000000FF;
	}
}
void CheckBE32(unsigned int ulVal)
{
	unsigned char ulTmp[4];
	ReadBuffer(ulTmp, 4);

}
void ReadBE64(int64_t &val)
{
	unsigned int ulTmp1,ulTmp2;
	ReadBE32(ulTmp1);
	ReadBE32(ulTmp2);
   val =  (((int64_t)ulTmp1 << 32) & 0xFFFFFFFF00000000) |
		  ((int64_t)(ulTmp2)        & 0x00000000FFFFFFFF);
}
void CheckBE64(int64_t val)
{
}
int ReadBuffer(unsigned char *pData, int lSize)
{
	unsigned long dwBytesRead = 0;
	dwBytesRead = read(m_hFile, pData, lSize);
	return (dwBytesRead);
}

void SetStartcode(
	unsigned char *pDest )
{
	unsigned int ulLen = 0;
	*pDest++ = 0x00;
	*pDest++ = 0x00;
	*pDest++ = 0x00;
	*pDest   = 0x01;
}

int ReadBufferAndConvertToNalu(
	CTrack         *pTrk,
	unsigned char  *pDest, 
	int            lenSrc)
{
	int nConsumed = 0;
	if(pTrk->m_CrntSample == 0) {
		// Prepend SPS
		unsigned int lNalSize = pTrk->AvcCfgRecord.lSpsSize;
		if(lNalSize) {
			SetStartcode(pDest);
			memcpy(pDest + 4, pTrk->AvcCfgRecord.Sps, lNalSize);
			pDest += lNalSize + 4;
		}
		lNalSize = pTrk->AvcCfgRecord.lPpsSize;
		if(lNalSize) {
			SetStartcode(pDest);
			memcpy(pDest + 4, pTrk->AvcCfgRecord.Pps, lNalSize);
			pDest += lNalSize + 4;
		}
	}

	while (nConsumed < lenSrc){
		unsigned int lNalSize;
		int nRead = 0;
		ReadBE32(lNalSize);
		
		if(lNalSize + nConsumed > lenSrc)
			break;

		SetStartcode(pDest);
		nRead = ReadBuffer(pDest + 4, lNalSize);
		if(nRead <= 0) {
			break;
		}
		pDest += lNalSize + 4;
		nConsumed += lNalSize + 4;
    }
    return lenSrc;
}

/**
 * ASC for AAC-LC: 
 * 5bits-Audio Object type or profile ?, 4bits-Sampling Frequency Index, 4bits-Channel Configuration
 *      
 */
void  SetAdts(
	unsigned char	*pDest,
	unsigned char	*Asc,
	unsigned int    lenPayload)
{

		unsigned char profileVal = (Asc[0] >> 3) -1;
		unsigned char sampleRateId = (Asc[0] &0x07) << 1 | (Asc[1] & 0x80) >> 7;
		unsigned char channelsVal = (Asc[1] >> 3) & 0x0F;
		unsigned char *adtsHeader = pDest;
        // bits 0-11 are sync
        adtsHeader[0] = 0xff;
        adtsHeader[1] = 0xf0;

        // bit 12 is mpeg4 which means clear the bit
        // bits 13-14 is layer, always 00

        // bit 15 is protection absent (no crc)
        adtsHeader[1] |= 0x1;

        // bits 16-17 is profile which is 01 for AAC-LC
        adtsHeader[2] = profileVal;

        // bits 18-21 is sampling frequency index
        adtsHeader[2] |= (sampleRateId << 2);

        // bit 22 is private
        // bit 23-25 is channel config.  However since we are mono or stereo
        // bit 23 is always zero
        adtsHeader[3] = channelsVal << 6;

        // adjust for headers
        unsigned short frameSize = lenPayload + 7;

        // get upper 2 bits of 13 bit length and move them to lower 2 bits
        adtsHeader[3] |= (frameSize & 0x1800) >> 11;

        // get middle 8 bits of length
        adtsHeader[4] = (frameSize & 0x7f8) >> 3;

        // get lower 3 bits of length and put as 3 msb
        adtsHeader[5] = (frameSize & 0x7) << 5;

        // bits 43-53 is buffer fulless but we use vbr so 0x7f
        adtsHeader[5] |= 0x1f;
        adtsHeader[6] = 0xfc;

}

int ReadBufferAndConvertToAdts(
	  unsigned char			*pDest, 
	  unsigned char	        *Asc,
	  int					lenSrc)
{
	int nRead = 0;
#ifdef EN_ADTS
	SetAdts(pDest, Asc, lenSrc);
	ReadBuffer(pDest + 7, lenSrc);
#else
	nRead = ReadBuffer(pDest, lenSrc);
#endif
    return nRead;
}


/*
 * Calculates number of bytes required by a descriptor 
 * tag + length specifier(7bit digits ?) + length
 */
unsigned int getDescrAndTagLength(unsigned int len)
{
	return (4/*Length*/ + 1/*tag*/ + len);
}

void ReadDescrTagAndLen(unsigned char &tag, unsigned int &len)
{
	ReadByte(tag);
	len = 0;
	unsigned char b;
	do {
		ReadByte(b);
		len = (len << 7) | (b & 0x7F);
	} while (b & 0x80);
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

int ReadAacEsdsBox(CTrack *pTrack, unsigned int _ulSize) // Basic
{
	int64_t llPos = GetPos();
	/* -> 4 bytes version/flags = 8-bit hex version + 24-bit hex flags  (current = 0)*/
	CheckBE32(0);				// Version(8) & Flags(24)

	unsigned int lenDesc = 0;
	unsigned char tag;
	ReadDescrTagAndLen(tag, lenDesc);
	if(tag == 0x03) {
		/* 2*/ReadBE16(pTrack->m_trackID);
		/* 3*/CheckByte(0x00);			// flags (= no flags)

		// DecoderConfig descriptor
		ReadDescrTagAndLen(tag, lenDesc);
		if(tag == 0x04){

			unsigned char codecId;
			ReadByte (codecId);
			if(codecId == 0x40)
				pTrack->m_CodecId = CODEC_ID_AAC;
			
			CheckByte(0x15); // flags (= Audiostream)
			unsigned char tmp1;
			unsigned int tmp2;
			/* 3*/ReadByte(tmp1);    // Buffersize DB (24 bits)
			/* 5*/ReadBE16(tmp2); // Buffersize DB
			pTrack->m_BufferSize = ((unsigned int)tmp1 << (3+16)) | ((unsigned int)tmp2 << 3);
			/* 9*/ReadBE32(pTrack->m_MaxBitrate);
			
			ReadBE32(pTrack->m_AvgBitrate);

			// DecoderSpecific info descriptor
				ReadDescrTagAndLen(tag, lenDesc);
				if(tag == 0x05){
					if(lenDesc > MAX_AAC_CFG_LEN)
						lenDesc = MAX_AAC_CFG_LEN;
					pTrack->AacCfgRecord.cfgLen = lenDesc;
					ReadBuffer ((unsigned char*)pTrack->AacCfgRecord.cfgRec, lenDesc);
				}
			}
		/*  -> 1 byte SL config descriptor type tag = 8-bit hex value 0x06    // SL descriptor*/
		ReadDescrTagAndLen(tag, lenDesc);
		if(tag == 0x06){
			CheckByte(0x02);
		}
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
unsigned char *FindStartCode(
	 unsigned char *p, 
	 unsigned char *end)
{
    while( p < end - 3) {
        if( p[0] == 0 && p[1] == 0 && ((p[2] == 1) || (p[2] == 0 && p[3] == 1)) )
            return p;
		p++;
    }

    return end;
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


unsigned long ReadTag(unsigned int &ulSize)
{
	unsigned long ulTag = 0;
	ReadBE32(ulSize);
	if(ulSize) {
		ReadBuffer((unsigned char *)&ulTag, 4);
	}
	return ulTag;

}


int64_t ReadFreeBox(unsigned int ulSize)
{
	return 0;
}

int ReadFtypBox(unsigned int ulSize)
{
	int64_t llPos = GetPos();
	int res = 0;
	CheckBE32(ATOM_ISOM);	/* MajorBrand */
	CheckBE32(0);			/* MinorVersion */
	CheckBE32(ATOM_ISOM);	/* Compatible brand */
	CheckBE32(ATOM_AVC1);

    return res;
}

int ReadMdatBox(unsigned int ulSize)
{
    m_lMdatSize = ulSize;
    m_llMdatPos = GetPos();
    return 0;
}

int UnsupportedAtom()
{
	return 0;
}

int ReadMovieHeader()
{
	bool fDone = 0;
	unsigned int ulTag;
	unsigned int ulSize;
    int64_t llPos = 0;
	while(true) {
		int64_t llPos = GetPos();
		ulTag = ReadTag(ulSize);
		if(ulTag == 0)	break;

		if(ulTag == 0)
			break;
		if(ulTag == ATOM_FTYP)
			ReadFtypBox(ulSize - 8);
		else if(ulTag == ATOM_MDAT) 
			ReadMdatBox(ulSize - 8);
		else if(ulTag == ATOM_MOOV)
			ReadMoovBox(ulSize - 8);
		else
			UnsupportedAtom();

		SeekPos(llPos + ulSize);
	}
	return 0;
}


int64_t CovertScaledUnitsTo100ns(unsigned long ulTimescale, int64_t llTime)
{
	return (llTime * ((double)10000000 / ulTimescale));
}


int ReadAvcc(CAvcCfgRecord *pAvcCfgRecord, unsigned int ulSize)
{
	ReadByte( pAvcCfgRecord->bVersion); /* version */
	ReadByte(pAvcCfgRecord->Profile); /* profile */

	ReadByte(pAvcCfgRecord->ProfileCompat); /* profile compat */
	ReadByte(pAvcCfgRecord->Level); /* level */
	CheckByte( 0xff); /* 6 bits reserved (111111) + 2 bits nal size length - 1 (11) */
	CheckByte( 0xe1); /* 3 bits reserved (111) + 5 bits number of sps (00001) */

	ReadBE16(pAvcCfgRecord->lSpsSize);
	ReadBuffer( pAvcCfgRecord->Sps, pAvcCfgRecord->lSpsSize);
	CheckByte( 1); /* number of pps */
	ReadBE16(pAvcCfgRecord->lPpsSize);
	ReadBuffer(pAvcCfgRecord->Pps, pAvcCfgRecord->lPpsSize);

	return 0;
}

/*
** moov: Movie Box
** The metadata for a presentation.
** 
** Parent : File
*/

int ReadMoovBox(unsigned int _ulSize)
{
	int64_t llPosStart = GetPos();
	int64_t llPos = llPosStart;
	unsigned int ulTag;
	unsigned int ulSize;
	while(GetPos() < (llPosStart + _ulSize)) {
		llPos = GetPos();
		ulTag = ReadTag(ulSize);

		if(ulTag == 0)	break;

		if(ulTag == ATOM_MVHD) {
			ReadMvhdBox(ulSize - 8);
		} else if(ulTag == ATOM_TRAK) {
			ReadTrakBox(&m_Tracks[m_NbStreams], ulSize - 8);
			m_NbStreams++;
		}
		SeekPos(llPos + ulSize);
	}
	return 0;
}

/*
** mvhd : Movie Header Box
** This box defines overall information which is media-independent, and relevant to the entire presentation
** considered as a whole.
**
** Parent : moov
*/
int ReadMvhdBox(unsigned int ulSize)
{
	unsigned char bVersion = 0;
    CheckByte(0);				// bVersion = 0 
    CheckBE24( 0); /* flags */

	ReadBE32(m_lTime);		/*The creation time of the box,
							  expressed as seconds elapsed since
							  midnight, January 1, 1904 (UTC) */
    ReadBE32(m_lTime);		/* modification time */

	ReadBE32(m_lMovieTimescale); /* timescale */
	if(bVersion)
		ReadBE64(m_llMovieDuration); /* duration of longest track */
	else {
		unsigned int lMovieDuration;
		ReadBE32(lMovieDuration); /* duration of longest track */
		m_llMovieDuration = lMovieDuration;
	}
    CheckBE32(0x00010000);	/* reserved (preferred rate) 1.0 = normal */
    CheckBE16( 0x0100);	/* reserved (preferred volume) 1.0 = normal */
    CheckBE16( 0);		/* reserved */
    CheckBE32( 0);			/* reserved */
    CheckBE32( 0);			/* reserved */

    /* Matrix structure */
    CheckBE32(0x00010000);	/* reserved */
    CheckBE32( 0x0);		/* reserved */
    CheckBE32( 0x0);		/* reserved */
    CheckBE32( 0x0);		/* reserved */
    CheckBE32( 0x00010000); /* reserved */
    CheckBE32( 0x0);		/* reserved */
    CheckBE32( 0x0);		/* reserved */
    CheckBE32( 0x0);		/* reserved */
    CheckBE32( 0x40000000); /* reserved */

    CheckBE32( 0);			/* reserved (preview time) */
    CheckBE32( 0);			/* reserved (preview duration) */
    CheckBE32( 0);			/* reserved (poster time) */
    CheckBE32( 0);			/* reserved (selection time) */
    CheckBE32( 0);			/* reserved (selection duration) */
    CheckBE32( 0);			/* reserved (current time) */
    CheckBE32( m_MaxTrackID+1); /* Next track id */
	return 0;
}

/*
** trak : Track Box
** This is a container box for a single track of a presentation.
** The presentation carries its own temporal and spatial
** information. Each track will contain its associated Media Box.
**
** Parent : moov
*/

int ReadTrakBox(CTrack *track, unsigned int _ulSize)
{
	int ret = 0;

	int64_t llPosStart = GetPos();
	int64_t llPos = llPosStart;

	unsigned int ulTag;
	unsigned int ulSize;
	while(GetPos() < (llPosStart + _ulSize)) {
		llPos = GetPos();
		ulTag = ReadTag(ulSize);
		
		if(ulTag == 0)	break;

		if(ulTag == ATOM_TKHD) 
			ReadTkhdBox(track,ulSize - 8);
		else if(ulTag == ATOM_MDIA) 
			ReadMdiaBox(track, ulSize - 8);
		SeekPos(llPos + ulSize);
	}
	return ret;
}

/*
** tkhd : Track Header Box
** This box specifies the characteristics of a single track
**
** Parent : trak
*/

int ReadTkhdBox(CTrack *track, unsigned int ulSize)
{
	unsigned char bVersion;
    ReadByte(bVersion);
    CheckBE24(0xf); /* flags (track enabled) */
    if (bVersion == 1) {
        ReadBE64(track->time);
        ReadBE64(track->time);
    } else {
		unsigned int _time;
        ReadBE32(_time); /* creation time */
		track->time = _time;
        ReadBE32(_time); /* modification time */
		track->time = _time;
    }
    ReadBE32(track->m_trackID); /* track-id */
    CheckBE32( 0); /* reserved */

	int64_t llTtrackDuration;
	if(bVersion == 1) {
		ReadBE64(llTtrackDuration);
	} else {
		unsigned int lTtrackDuration;
		ReadBE32(lTtrackDuration);
		llTtrackDuration = lTtrackDuration;
	}
	track->m_llTtrackDuration  =  llTtrackDuration * (double)track->m_lTimescale / m_lMovieTimescale;
    CheckBE32( 0); /* reserved */
    CheckBE32( 0); /* reserved */
    CheckBE32( 0x0); /* reserved (Layer & Alternate group) */
    /* Volume, only for audio */
    //if(track->m_CodecType == CODEC_TYPE_AUDIO)
    //    CheckBE16( 0x0100);
    //else
        CheckBE16( 0);
    CheckBE16( 0); /* reserved */

    /* Matrix structure */
    CheckBE32( 0x00010000); /* reserved */
    CheckBE32( 0x0); /* reserved */
    CheckBE32( 0x0); /* reserved */
    CheckBE32( 0x0); /* reserved */
    CheckBE32( 0x00010000); /* reserved */
    CheckBE32( 0x0); /* reserved */
    CheckBE32( 0x0); /* reserved */
    CheckBE32( 0x0); /* reserved */
    CheckBE32( 0x40000000); /* reserved */

    //double sample_aspect_ratio = av_q2d(st->sample_aspect_ratio);
    //if(!sample_aspect_ratio) sample_aspect_ratio = 1;
	unsigned long sample_aspect_ratio = 1;
    ReadBE32(track->m_lWidth);
	track->m_lWidth = track->m_lWidth / 0x10000;
    ReadBE32(track->m_lHeight);
	track->m_lHeight = track->m_lHeight / 0x10000;
    return 0;
}

/*
** mdia : Media Box
** The media declaration container contains all the objects that declare information about the media data within a
** track.
**
** Parent : trak
*/

int ReadMdiaBox(CTrack *track, unsigned int _ulSize)
{
	int64_t llPosStart = GetPos();
	int64_t llPos = llPosStart;

	unsigned int ulTag;
	unsigned int ulSize;
	while( GetPos() < llPosStart + _ulSize) {
		llPos = GetPos();
		ulTag = ReadTag(ulSize);
		if(ulTag == 0)	break;
		if(ulTag == ATOM_MDHD)
			ReadMdhdBox( track, ulSize - 8);
		else if(ulTag == ATOM_HDLR)
			ReadHdlrBox( track, ulSize - 8);
		else if(ulTag == ATOM_MINF)
			ReadMinfBox( track, ulSize - 8);

		SeekPos(llPos + ulSize);
	}
	return 0;
}

/*
** mdhd : Media Header Box
** The media header declares overall information that is media-independent, and relevant to characteristics of
** the media in a track
**
** Parent : mdia
*/

int ReadMdhdBox(CTrack *track, unsigned int _ulSize)
{
    unsigned char bVersion;
    ReadByte(bVersion);
    CheckBE24( 0); /* flags */
    if (bVersion == 1) {
        ReadBE64(track->time);
        ReadBE64(track->time);
    } else {
		unsigned int ulTime;
        ReadBE32(ulTime); /* creation time */
		track->time = ulTime;
        ReadBE32(ulTime); /* modification time */
		track->time = ulTime;
    }
    ReadBE32(track->m_lTimescale); /* time scale (sample rate for audio) */
	if (bVersion == 1){
		ReadBE64(track->m_llTtrackDuration);
	} else {
		unsigned int lTtrackDuration;
		ReadBE32(lTtrackDuration); /* duration */
		 track->m_llTtrackDuration = lTtrackDuration;
	}
    ReadBE16(track->language); /* language */
    CheckBE16(0); /* reserved (quality) */

    return 32;
}
/*
** hdlr : Handler Reference Box
** This box within a Media Box declares the process by which the media-data in the track is presented, and thus,
** the nature of the media in a track. For example, a video track would be handled by a video handler.
** Parent : mdia
*/
#define MAX_DESC_LEN	1024
int ReadHdlrBox(CTrack *track, unsigned int ulSize)
{
	unsigned char hdlr[8] = {0};
	unsigned char  hdlrType[32] = {0};
	unsigned char descr[MAX_DESC_LEN] = {0};
    
	int64_t llPos = GetPos();

    CheckBE32(0); 
    ReadBuffer(hdlr, 4); 
    ReadBuffer(hdlrType, 4);							
    CheckBE32(0); /* reserved */
    CheckBE32(0); /* reserved */
    CheckBE32(0); /* reserved */
	unsigned char lenDescr;
    ReadByte(lenDescr);			/* string counter */
	assert(lenDescr < MAX_DESC_LEN);
    ReadBuffer(descr, lenDescr);	/* handler description */

	if (strcmp((char *)hdlrType, "vide") == 0){
		track->m_CodecType = CODEC_TYPE_VIDEO;
    } else if (strcmp((char *)hdlrType, "soun") == 0){
		track->m_CodecType = CODEC_TYPE_AUDIO;
    }

    return 0;
}

/*
** iinf : Media Information Box
** This box contains all the objects that declare characteristic information of the media in the track.
**
** Parent : mdia
*/

int ReadMinfBox(CTrack *track, unsigned int _ulSize)
{

	int64_t llPosStart = GetPos();
	int64_t llPos = llPosStart;

	unsigned int ulTag;
	unsigned int ulSize;
	while( GetPos() < llPosStart + _ulSize) {
		llPos = GetPos();
		ulTag = ReadTag(ulSize);
		if(ulTag == 0)	break;
		if(ulTag == ATOM_VMHD){
			ReadVmhdBox(ulSize - 8);
		} else if(ulTag == ATOM_SMHD){
			ReadSmhdBox(ulSize - 8);
		} else if(ulTag == ATOM_DINF){
			ReadDinfBox(ulSize - 8);
		} else if(ulTag == ATOM_STBL){
			ReadStblBox(track, ulSize - 8);
		}
		SeekPos(llPos + ulSize);
	}
    return 0;
}

/*
** dinf : Data Reference Box
** The data information box contains objects that declare the location of the media information in a track.
**
** Parent : minf
*/

int ReadDinfBox(unsigned int _ulSize)
{
    int64_t llPos = GetPos();
	unsigned int ulTag;
	unsigned int ulSize;
	ulTag = ReadTag(ulSize);
	if(ulTag == ATOM_DREF)
		ReadDrefBox(ulSize - 8);
    return 0;
}

/*
** dref : Data Information Box
** The data reference object contains a table of data references (normally URLs) that declare the location(s) of
** the media data used within the presentation
**
** Parent : dinf
*/

int ReadDrefBox(unsigned int _ulSize)
{
    CheckBE32( 0); /* version & flags */
    CheckBE32(1); /* entry count */
	unsigned int ulTag;
	unsigned int ulSize;
	ulTag = ReadTag(ulSize);
	if(ulTag == ATOM_URL){
		CheckBE32(1); /* version & flags */
	}

    return 28;
}


/*
** vmhd : Video Media Header Box
** The video media header contains general presentation information, independent of the coding, for video
** media
**
** Parent : minf
*/
int ReadVmhdBox(unsigned int ulSize)
{
    CheckBE32( 0x01); /* version & flags */
    CheckBE64( 0); /* reserved (graphics m_Mode = copy) */
    return 0;
}

/*
** smhd : Sound Media Header Box
** The sound media header contains general presentation information, independent of the coding, for audio
** media.
**
** Parent : minf
*/

int ReadSmhdBox(unsigned int ulSize)
{
    CheckBE32(0); /* version & flags */
    CheckBE16(0); /* reserved (balance, normally = 0) */
    CheckBE16(0); /* reserved */
    return 16;
}


/*
** stbl : Sample Table Box
** The sample table contains all the time and data indexing of the media samples in a track
**
** Parent : minf
*/

int ReadStblBox(CTrack *track, unsigned int _ulSize)
{
	int64_t llPosStart = GetPos();
	int64_t llPos = llPosStart;

	unsigned int ulTag;
	unsigned int ulSize;
	// Process StcoBox to get max chunk
	while( GetPos() < llPosStart + _ulSize) {
		llPos = GetPos();
		ulTag = ReadTag(ulSize);
		if(ulTag == 0)	break;

		if (ulTag == ATOM_STSZ) {
			ReadStszBox(track, ulSize - 8);
		} else if (ulTag == ATOM_STCO) {
			ReadStcoBox(track, ulSize - 8, 0);
		} else if (ulTag == ATOM_CO64) {
			ReadStcoBox(track, ulSize - 8, 1);
		}
		SeekPos(llPos + ulSize);
	}

	SeekPos(llPosStart);

	while( GetPos() < llPosStart + _ulSize) {
		llPos = GetPos();
		ulTag = ReadTag(ulSize);
		if(ulTag == 0)	break;
		if(ulTag == ATOM_STSD) {
			ReadStsdBox(track, ulSize - 8);
		} else if (ulTag == ATOM_STTS) {
			ReadSttsBox(track, ulSize - 8);
		} else if (ulTag == ATOM_STSS) {
			ReadStssBox( track, ulSize - 8);
		} else if (ulTag == ATOM_CTTS) {
			ReadCttsBox( track, ulSize - 8);
		} else if (ulTag == ATOM_STSC) {
			ReadStscBox( track, ulSize - 8);
		}

		SeekPos(llPos + ulSize);
	}
    return 0;
}
/*
** Parent : stbl
*/

int ReadStsdBox(CTrack *track, unsigned int _ulSize)
{
    int64_t llPos = GetPos();
    CheckBE32(0); /* version & flags */
    CheckBE32(1); /* entry count */

	unsigned int ulTag;
	unsigned int ulSize;
	ulTag = ReadTag(ulSize);

	if(ulTag == ATOM_AVC1){
		ReadVideoBox(track);
	} else 	if(ulTag == ATOM_MP4A){
        ReadAudioBox(track);
	}
    return 0;
}

/*
** Parent : StsdBox
*/
int ReadVideoBox(CTrack *track)
{
    char compressor_name[32];

    CheckBE32( 0);	/*const unsigned int(8)[6] reserved = 0;*/
    CheckBE16( 0); 
    CheckBE16(1);	/* unsigned int(16) data_reference_index; */

    CheckBE16( 0);	/* unsigned int(16) pre_defined */
    CheckBE16( 0);	/* unsigned int(16) reserved */
    
	CheckBE32( 0);	/* unsigned int(32)[3] pre_defined */
    CheckBE32( 0);
    CheckBE32( 0);

	ReadBE16(track->m_lWidth); /* Video width */
    ReadBE16(track->m_lHeight); /* Video height */
    CheckBE32( 0x00480000); /* Horizontal resolution 72dpi */
    CheckBE32( 0x00480000); /* Vertical resolution 72dpi */
    CheckBE32( 0); /* reserved */
    CheckBE16( 1); /* Frame count (= 1) */

    memset(compressor_name,0,32);
#if 0
	// TODO:
	strncpy_s(compressor_name,31, (const char *)track->m_szName,31);
#endif
	unsigned char lenDispName;
    ReadByte( lenDispName);
    ReadBuffer((unsigned char *)compressor_name, 31);

	CheckBE16( 0x18); /* Reserved */
    CheckBE16( 0xffff); /* Reserved */
	unsigned int ulTag;
	unsigned int ulSize;
	ulTag = ReadTag(ulSize);
	if(ulTag == ATOM_AVCC){
		track->m_CodecId = CODEC_ID_H264;
		ReadAvcc(&track->AvcCfgRecord, ulSize - 8);
	}
    return 0;
}
/*
** Parent : StsdBox
*/
int ReadAudioBox(CTrack *track)
{
    CheckBE32(0); /* unsigned int(8)[6] reserved */
    CheckBE16(0); 
    CheckBE16(1); /* unsigned int(16) data_reference_index */

	CheckBE32(0); /* unsigned int(32)[2] reserved = 0; */
	CheckBE32(0); 

	ReadBE16(track->m_Channels);
	CheckBE16(16);

    CheckBE16(0); /* unsigned int(16) pre_defined = 0; */
    CheckBE16(0); /* unsigned int(16) reserved = 0 */
	ReadBE32(track->m_Samplerate);
	track->m_Samplerate = track->m_Samplerate >> 16;

	unsigned int ulTag;
	unsigned int ulSize;
	ulTag = ReadTag(ulSize);
	if(ulTag == ATOM_ESDS /*?*/){
		ReadAacEsdsBox(track, ulSize - 8);
	}
    return 0;
}


/* 
** Decoding Time to Sample Box
** This box contains a compact version of a table that allows indexing from decoding time to sample number.
** Each entry in the table gives the
** number of consecutive samples with the same time delta, and the delta of those samples. By adding the
** deltas a complete time-to-sample map may be built.
*/
int ReadSttsBox(CTrack *track, unsigned int ulSize)
{
    int i;
	unsigned int entries;
    CheckBE32( 0); /* version & flags */
    ReadBE32(entries); /* entry count */
	int idxChunk = 0;

	for (i=0; i<entries; i++) {
		unsigned int count;
		unsigned int duration;

        ReadBE32(count/*stts_entries[i].count*/);
        ReadBE32(duration/*stts_entries[i].duration*/);

		if(idxChunk > 0)
			track->m_ChunkList[idxChunk].m_llDts = track->m_ChunkList[idxChunk - 1].m_llDts + duration;
		else
			track->m_ChunkList[idxChunk].m_llDts = duration;
		idxChunk++;
		for (int j =1; j < count; j++){
			track->m_ChunkList[idxChunk].m_llDts = track->m_ChunkList[idxChunk - 1].m_llDts + duration;
			idxChunk++;
		}
    }
    return 0;
}

int ReadStssBox(CTrack *track, unsigned int ulSize)
{
    int64_t llCurpos, llEntryPos;
	unsigned int ulChunkCount;
    int i, index = 0;
    CheckBE32( 0); // version & flags
    ReadBE32(ulChunkCount); // entry count
    for (i=0; i<ulChunkCount; i++) {
        if(track->m_ChunkList[i].key_frame == 1) {
			unsigned int val;
            ReadBE32( val);
        }
    }

    return 0;
}

/*
** Composition Time to Sample Box
** This box provides the offset between decoding time and composition time. Since decoding time must be less
** than the composition time, the offsets are expressed as unsigned numbers such that CT(n) = DT(n) +
** CTTS(n) where CTTS(n) is the (uncompressed) table entry for sample n.
*/
int ReadCttsBox(CTrack *track, unsigned int ulSize)
{
    stts_t *pCttsEntries;
    unsigned int ulEentries = 0;
    unsigned long i;

    CheckBE32( 0); /* version & flags */
    ReadBE32(ulEentries); /* entry count */
    for (i=0; i<ulEentries; i++) {
		unsigned count;
		unsigned duration;
        ReadBE32( count/*pCttsEntries[i].count*/);
        ReadBE32( duration/*pCttsEntries[i].duration*/);
    }
    return 0;
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
int ReadStscBox(CTrack *track, unsigned int ulSize)
{
    int index = 0, oldval = -1, i;
    int64_t llEntryPos, llCurpos;
	unsigned int ulChunkCount;
	unsigned int ulSampleCount = 0;

	unsigned int prevStartChunkIdx = 0;
	unsigned int prevCountSampleInChunk = 0;
    CheckBE32( 0); 
    ReadBE32( ulChunkCount);

	unsigned int StartChunkIdx = 0, countSampleInChunk = 0, indexSample = 0;
    for (i=0; i < ulChunkCount; i++) {
		ReadBE32(StartChunkIdx);
		ReadBE32( countSampleInChunk);
		StartChunkIdx--; // Base 0
		for(int j = prevStartChunkIdx; j < StartChunkIdx; j++){
			track->m_ChunkOffsetTbl[j].m_startSample = ulSampleCount;
			ulSampleCount += prevCountSampleInChunk;
		}
		ReadBE32( indexSample);
		prevCountSampleInChunk = countSampleInChunk;
		prevStartChunkIdx = StartChunkIdx;
    }
	for(int k = prevStartChunkIdx; k < track->m_MaxChunk; k++){
		track->m_ChunkOffsetTbl[k].m_startSample = ulSampleCount;
		ulSampleCount += prevCountSampleInChunk;
	}
	ulSampleCount += prevCountSampleInChunk;
    return 0;;
}

/* 
** Sample size atom 
** This box contains the sample count and a table giving the size in bytes of each sample. This allows the media
** data itself to be unframed. The total number of samples in the media is always indicated in the sample count.
*/
int ReadStszBox(CTrack *track, unsigned int ulSize)
{
    int equalChunks = 1;
	unsigned int entries;
	unsigned int sampleSize;

	CheckBE32(0); /* version & flags */

    ReadBE32(sampleSize); 
    ReadBE32(entries);

	track->m_MaxSample = entries;
	track->m_ChunkList = (ChunkEntry *)malloc(track->m_MaxSample * sizeof(ChunkEntry));
	memset(track->m_ChunkList, 0x00, track->m_MaxSample * sizeof(ChunkEntry));

	if(sampleSize) {
		for (int i=0; i<entries; i++) {
			track->m_ChunkList[i].size = sampleSize;
		}
	} else {
		for (int i=0; i<entries; i++) {
			ReadBE32(track->m_ChunkList[i].size);
		}
	}
    return 0;
}

/*
** Chunk Offset Box
** The chunk offset table gives the index of each chunk into the containing file
** Offsets are file offsets, not the offset into any box within the file
*/
int ReadStcoBox(CTrack *track, unsigned int ulSize, int mode64)
{
    int i;
	unsigned int entries;

    CheckBE32(0); /* version & flags */
    ReadBE32(entries);
	track->m_MaxChunk = entries;
	track->m_ChunkOffsetTbl = (ChunkOffset *)malloc(sizeof(ChunkOffset) * entries);
    for (i=0; i < entries; i++) {
		if(mode64 == 1){
            ReadBE64(track->m_ChunkOffsetTbl[i].m_llPos);
		} else {
			unsigned int ulPos;
            ReadBE32(ulPos);
			track->m_ChunkOffsetTbl[i].m_llPos = ulPos;
		}
    }
    return 0;
}

public:

void GetSampleInfo(CTrack *pTrk,
		int          nChunk,
		int64_t      &llChunkPos,
		int          &nChunkSize,
		unsigned int &samplesInChunk,
		int64_t      &llPts,
		int64_t      &llDts,
		int          &fKeyFrame
		)
{
#if 0
	llChunkPos = pTrk->m_ChunkList[nChunk].m_llPos;
#else
	llChunkPos = pTrk->GetSampleOffset();
#endif
	samplesInChunk = pTrk->m_ChunkList[nChunk].samplesInChunk;
	nChunkSize = pTrk->m_ChunkList[nChunk].size;
	samplesInChunk = pTrk->m_ChunkList[nChunk].entries;
	llDts = pTrk->m_ChunkList[nChunk].m_llDts;
	llPts = pTrk->m_ChunkList[nChunk].m_llDts + pTrk->m_ChunkList[nChunk].m_Cts;
	fKeyFrame = pTrk->m_ChunkList[nChunk].key_frame;
}

int  GetAvSample(int TrackId, CPacket *pPkt)
{
    CTrack *pTrk = &m_Tracks[TrackId];
    unsigned int samplesInChunk = 0;
	int     fKeyFrame = 0;
	int64_t llPts; // TODO get form file
	int64_t llDts;
	int64_t llChunkPos;
	int     nChunkSize;
	if(pTrk->m_CrntSample < pTrk->m_MaxSample) {
		GetSampleInfo(pTrk, pTrk->m_CrntSample, llChunkPos, nChunkSize, samplesInChunk, llPts, llDts, fKeyFrame);

		pPkt->m_llPts = CovertScaledUnitsTo100ns(pTrk->m_lTimescale, llPts);
		pPkt->m_llDts = CovertScaledUnitsTo100ns(pTrk->m_lTimescale, llDts);
		SeekPos(llChunkPos);
		/* nal reformating needed */
		if(pTrk->m_CodecId == CODEC_ID_H264) {
			pPkt->m_lSize = ReadBufferAndConvertToNalu(pTrk, pPkt->m_pData, nChunkSize);
		} else if(pTrk->m_CodecId == CODEC_ID_AAC) {
			pPkt->m_lSize = ReadBufferAndConvertToAdts(pPkt->m_pData, (unsigned char *)pTrk->AacCfgRecord.cfgRec, nChunkSize);
		}
		if(fKeyFrame) {
			fKeyFrame = (pPkt->m_Flags & PKT_FLAG_KEY);
		}
		pPkt->m_CodecId = pTrk->m_CodecId;
		pPkt->m_CodecId = pTrk->m_CodecType;
		return S_OK;
	} else {
		return -1;
	}
}

int  AdvanceSample(int TrackId)
{
    CTrack *pTrk = &m_Tracks[TrackId];
	return pTrk->AdvanceSample();
}

void Seek(int TrackId, long long llTime)
{
    CTrack *pTrk = &m_Tracks[TrackId];
	if(llTime == 0) {
		pTrk->m_CrntSample = 0;
		pTrk->m_CrntChunk = 0;
	} else {
	}
}

int GetVidWidth(int TrackId)
{
    CTrack *pTrk = &m_Tracks[TrackId];
	return pTrk->m_lWidth;

}

int GetVidHeight(int TrackId)
{
    CTrack *pTrk = &m_Tracks[TrackId];
	return pTrk->m_lHeight;
}

int OpenFileReader(const char *pszFileName)
{
#ifdef WIN32
	m_hFile = open(pszFileName, O_RDONLY|O_BINARY, 0);
#else
	m_hFile = open (pszFileName, O_RDONLY, 0);
#endif

	if(m_hFile < 0)
		return -1;

	ReadMovieHeader();

	return 0;
}


int CloseFileReader()
{
	if(m_hFile > 0)
		close(m_hFile);
	return 0;
}


CAvcCfgRecord *GetAvcCfg(int TrackId)
{
	CTrack *pTrack = &m_Tracks[TrackId];
	return &pTrack->AvcCfgRecord;
}

CAacCfgRecord *GetAacCfg(int TrackId)
{
	CTrack *pTrack = &m_Tracks[TrackId];
	return &pTrack->AacCfgRecord;
}

int GetAudSampleRate(int TrackId)
{
	CTrack *pTrack = &m_Tracks[TrackId];
	return pTrack->m_Samplerate;
}

int GetAudNumChannels(int TrackId)
{
	CTrack *pTrack = &m_Tracks[TrackId];
	return pTrack->m_Channels;
}

int GetNextPresentationTrackId()
{
	int64_t ullNextChunkTime = INT64_MAX;
	int nTrackId = -1;
	for (int i=0; i < m_NbStreams; i++){
		CTrack *pTrack = &m_Tracks[i];
		int64_t ullTmpTime;
		if(pTrack->GetNextChunkTime(ullTmpTime) == 0) {
			ullTmpTime = CovertScaledUnitsTo100ns(pTrack->m_lTimescale, ullTmpTime);
			if(ullTmpTime < ullNextChunkTime){
				ullNextChunkTime = ullTmpTime;
				nTrackId =  i;
			}
		}
	}
	return nTrackId;
}

int GetNmumTracks()
{
	return m_NbStreams;
}

CTrackInf *GetTrackInf(int TrackId)
{
	CTrackInf *pTrackInf = &m_Tracks[TrackId];
	return pTrackInf;
}

CMp4Demux()
{
	m_lMdatSize = 0;
	m_NbStreams = 0;
	m_lTime = 0;
	m_MaxTrackID = 1;
	m_llMovieDuration =  0;//100000 * 600;
}

private:
	int64_t GetPos() 
	{
		int64_t llPos;
		LONG lDistLow = 0;
		LONG lDistHigh = 0;
#if 0
		lDistLow = GetFileSize(m_hFile, &lDistHigh);
#else
		lDistLow = lseek(m_hFile, 0, SEEK_CUR);
#endif
		llPos = lDistHigh;
		llPos |= llPos << 32 | lDistLow;
		return llPos;
	}

	void SeekPos(int64_t llPos) 
	{
		lseek(m_hFile, llPos, SEEK_SET);
	}

	int           m_hFile;

	unsigned char *m_pBuff;
	unsigned char *m_pBuffEnd;
	int64_t	 m_llMdatPos;
	unsigned long 	m_NbStreams;
	unsigned int m_lMdatSize;
	unsigned int m_lTime;

	int64_t	 m_llMovieDuration;
	unsigned long m_MaxTrackID;
    unsigned int     m_lMovieTimescale;
	CTrack	 m_Tracks[MAX_TRACKS];
};

CMp4DemuxIf::CMp4DemuxIf()
{

}
CMp4DemuxIf::~CMp4DemuxIf()
{

}

CMp4DemuxIf *CMp4DemuxIf::CreateInstance()
{
	CMp4DemuxIf *pInstance = new CMp4Demux;
	return pInstance;
}