/*
Transport Stream Demultiplexer
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <map>

#ifdef WIN32
#include <Windows.h>
typedef __int64 int64_t;
#define snprintf sprintf_s
#else
#include <sys/ioctl.h>
#include <pthread.h>
#include <unistd.h>
#endif

// Undefining the following falg creates original xport binary

#include "xport.h"
#include "filesrc.h"


#include "strmcomp.h"
#include "strmclock.h"
#include "JdDbg.h"

//#define DBG_MSG printf
#define DBG_MSG(...)

#define TRUE		1
#define FALSE		0

#define VIDEO		0
#define AUDIO		1
#define PADDING		2

#define I			0
#define P			1
#define B			2
#define BI			3
#define SKIPPED		4

#define STRM_ID_AUD 1
#define STRM_ID_VID 2

#define SEQUENCE_HEADER_CODE	0x000001b3

#define OPTION_CHAR	'-'

typedef struct	psip
{
	unsigned int	psip_section_start;
	unsigned int	psip_pointer_field;
	unsigned int	psip_section_length_parse;
	unsigned int	psip_section_parse;
	unsigned int	psip_xfer_state;
	unsigned short	psip_section_length;
	unsigned int	psip_offset;
	unsigned int	psip_index;
	unsigned char	psip_table_id;
	unsigned short	psip_table_id_ext;
	unsigned char	psip_section_number, psip_last_section_number;
	unsigned char	psip_table[4096];
}psip_t;


typedef struct
{
	unsigned int frameNo;
	unsigned int frameSize;
	unsigned char *readBuf;
	unsigned char *chunkBuf;

	int        fRun;
	unsigned int bytes;
	unsigned int tmp;
	unsigned char firstParse;
	unsigned int VidChunkCnt;
	unsigned int AudChunkCnt;
	int        fEoS;
#ifdef WIN32
	HANDLE threadidDemux;
#else
	pthread_t  threadidDemux;
#endif
	ConnCtxT   *pConnSrc;
	ConnCtxT   *pConnAudOut;
	ConnCtxT   *pConnVidOut;

	int64_t read_position;	
	int64_t last_totalbytes;

	int64_t start_ts;
	int64_t last_ts;
	int64_t interval_ts;
	int64_t stat_update_interval;
	
	unsigned long long current_pcr;
	unsigned long long pcr_arrival_time;
	unsigned long long current_tsrate;
	unsigned long long crnt_vid_pts;
	unsigned long long crnt_vid_dts;
	unsigned long long crnt_aud_pts;
	int                aud_end_of_frame;
	int                fClkSrc;
	void               *pClk;
	int                nUiCmd;

	int                     detect_program_pids;
	unsigned int	        program;
	unsigned int	        video_channel;
	unsigned int	        audio_channel;

	unsigned short	        pcr_pid;
	unsigned short	        video_pid;
	unsigned short	        audio_pid;
	unsigned char	        audio_stream_type;
	unsigned char	        video_stream_type;
	
	FILE	*fpoutvideo, *fpoutaudio;

	unsigned int	pid_counter[0x2000];
	unsigned int	packet_counter;
	unsigned int	pid_first_packet[0x2000];
	unsigned int	pid_last_packet[0x2000];

	unsigned int	parse_only;
	unsigned int	dump_audio_pts;
	unsigned int	dump_video_pts;
	unsigned int	timecode_mode;
	unsigned int	dump_pids;
	unsigned int	suppress_tsrate;

	unsigned int	pes_streams;
	unsigned int	dump_psip;
	unsigned int	hdmv_mode;
	unsigned int	dump_extra;
	unsigned int	dump_pcr;
	unsigned int	lpcm_mode;

	unsigned int	running_average_bitrate;
	unsigned int	coded_frames;
	unsigned int	video_fields;
	unsigned int	video_progressive;
	unsigned long long	last_video_pts;
	unsigned long long	last_audio_pts;
	unsigned long long	last_video_pts_diff;
	unsigned long long	last_audio_pts_diff;
	unsigned long long	pts_aligned;


	unsigned int	demux_audio;

	unsigned int	sync_state;
	unsigned int	xport_packet_length;
	unsigned int	xport_header_parse;
	unsigned int	adaptation_field_state;
	unsigned int	adaptation_field_parse;
	unsigned int	adaptation_field_length;
	unsigned int	pcr_parse;
	unsigned int	pat_section_start;
	unsigned int	pmt_section_start;


	unsigned int	video_packet_length_parse;
	unsigned int	video_packet_parse;
	unsigned int	video_pts_parse;
	unsigned int	video_pts_dts_parse;
	unsigned int	video_xfer_state;
	unsigned int	video_packet_number;
	unsigned char	video_pes_header_length;

	unsigned int	audio_packet_length_parse;
	unsigned int	audio_packet_parse;
	unsigned int	audio_pts_parse;
	unsigned int	audio_pts_dts_parse;
	unsigned int	audio_lpcm_parse;
	unsigned int	audio_xfer_state;
	unsigned int	audio_packet_number;
	unsigned char	audio_pes_header_length;

	unsigned int	pat_pointer_field;
	unsigned int	pat_section_length_parse;
	unsigned int	pat_section_parse;
	unsigned int	pat_xfer_state;

	unsigned int	pmt_pointer_field;
	unsigned int	pmt_section_length_parse;
	unsigned int	pmt_section_parse;
	unsigned int	pmt_xfer_state;

	unsigned int	pmt_program_descriptor_length_parse;
	unsigned int	pmt_program_descriptor_length;

	unsigned int	pmt_ES_descriptor_length_parse;
	unsigned int	pmt_ES_descriptor_length;

	unsigned long long	previous_pcr;
	unsigned long long	pcr_bytes;

	unsigned long long	prev_video_dts;
	unsigned long long	video_pts_count;
	unsigned long long	prev_audio_pts;
	unsigned char	continuity_counter[0x2000];
	unsigned int	skipped_bytes;

	psip_t	*psip_ptr[0x2000];
	unsigned int	tp_extra_header_parse;
	unsigned int	first_pat;
	unsigned int	first_pmt;


	unsigned int	tp_extra_header_prev;

	unsigned int	video_parse;
	unsigned int	video_packet_length;
	unsigned long long	video_temp_pts;
	unsigned long long	video_temp_dts;
	unsigned long long	video_pts;
	unsigned char	video_pes_header_flags;
	unsigned int	video_dts;

	unsigned int	audio_parse;
	unsigned short	audio_packet_length;
	unsigned long long	audio_temp_pts;
	unsigned long long	audio_pts;
	unsigned char	audio_pes_header_flags;
	unsigned short	audio_lpcm_header_flags;

	unsigned short	pat_section_length;
	unsigned int	pat_offset;

	unsigned short	pmt_section_length;
	unsigned int	pmt_offset;

	unsigned char	mgt_version_number;

	unsigned short	mgt_tables_defined;
	unsigned short	mgt_table_type;
	unsigned short	mgt_table_type_pid;
	unsigned short	mgt_table_type_version;
	unsigned int	mgt_number_bytes;
	unsigned short	mgt_table_type_desc_length;
	unsigned short	mgt_desc_length;
	unsigned int	mgt_crc;


	unsigned char	vct_version_number;
	unsigned char	vct_num_channels;
	unsigned short	vct_major_channel_number;
	unsigned short	vct_minor_channel_number;
	unsigned char	vct_modulation_mode;
	unsigned short	vct_channel_tsid;
	unsigned short	vct_program_number;
	unsigned char	vct_service_type;
	unsigned short	vct_source_id;
	unsigned short	vct_desc_length;
	unsigned short	vct_add_desc_length;
	unsigned int	vct_crc;

	unsigned char	sld_desc_length;
	unsigned short	sld_pcr_pid;
	unsigned char	sld_num_elements;
	unsigned char	sld_stream_type;
	unsigned short	sld_elementary_pid;

	unsigned char	ecnd_desc_length;
	unsigned char	ac3_desc_length;
	unsigned char	csd_desc_length;
	unsigned char	cad_desc_length;
	unsigned char	rcd_desc_length;

	unsigned char	eit_version_number;

	unsigned char	eit_num_events;
	unsigned short	eit_event_id;
	unsigned int	eit_start_time;
	unsigned int	eit_length_secs;
	unsigned char	eit_title_length;
	unsigned short	eit_desc_length;

	unsigned char	transport_error_indicator, payload_unit_start_indicator;
	unsigned char	transport_priority, transport_scrambling_control;
	unsigned char	adaptation_field_control;
	unsigned short	pid;


	unsigned short	program_number;
	unsigned char	pat_section_number, pat_last_section_number;
	unsigned char	pmt_section_number, pmt_last_section_number;
	unsigned char	pmt_stream_type;
	unsigned short	pmt_elementary_pid;
	unsigned short	pmt_program_info_length;
	unsigned short	pmt_ES_info_length;
	unsigned char	program_association_table[1024];
	unsigned char	program_map_table[1024];
	unsigned short	psip_pid_table[0x2000];
	unsigned char	video_pes_header[256 + 9];
	unsigned char	audio_pes_header[256 + 9];


	unsigned char	mgt_last_version_number;
	unsigned char	vct_last_version_number;

	unsigned short	ett_pid; 
	unsigned short	eit0_pid;
	unsigned short	eit1_pid;
	unsigned short	eit2_pid;
	unsigned short	eit3_pid;
	unsigned short	ett0_pid;
	unsigned short	ett1_pid;
	unsigned short	ett2_pid;
	unsigned short	ett3_pid;
	unsigned char	eit_last_version_number[4];

	unsigned short	program_map_pid;
	unsigned short	transport_stream_id;
	unsigned char	video_pes_header_index;
	unsigned char	audio_pes_header_index;
	unsigned int	first_audio_access_unit;
	unsigned long long	pcr;
	int             fDisCont;
	int             nInstanceId;
	std::map<int, ConnCtxT *> mConnections;
} MpegTsDemuxCtx;

typedef void *(*thrdStartFcnPtr) (void *);

void	demux_mpeg2_transport_init(MpegTsDemuxCtx *pCtx);
void	demux_mpeg2_transport_deinit(MpegTsDemuxCtx *pCtx);
void	demux_mpeg2_transport(MpegTsDemuxCtx *pCtx, unsigned int, unsigned char *);

// Global Context

static int DisplayStat(MpegTsDemuxCtx *self);


void *OpenWriter(int nStrmId)
{
#ifdef UNIT_TEST_DEMUX
	FILE	*fp;
	char	videofilename[] = {"bits0001.mpv"};
	char	audiofilename[] = {"bits0001.mpa"};
	if(nStrmId == STRM_ID_VID) {
		fp = fopen(&videofilename[0], "wb");
	} else {
		fp = fopen(&audiofilename[0], "wb");
	}
	return fp;
#else
	return (void *)nStrmId;
#endif
}

int WriteData(MpegTsDemuxCtx *pCtx, unsigned char *pData, int item_size, int length, int nStrmId)
{
#ifdef UNIT_TEST_DEMUX
	char szTmp1[128] = {0};
	char szTmp2[128] = {0};
	if(nStrmId == STRM_ID_VID) {
		Clock2HMSF(pCtx->crnt_vid_pts * 1000 / 90, szTmp1,127);
		Clock2HMSF(pCtx->current_pcr / 27, szTmp2,127);
		DBG_MSG("Vid: lenth=%d pCtx->pcr=%s pts=%s\n",length, szTmp1, szTmp2);
		fwrite(pData, item_size, length,pCtx->fpoutvideo);
	} else {
		Clock2HMSF(pCtx->crnt_aud_pts* 1000 / 90, szTmp1,127);
		Clock2HMSF(pCtx->current_pcr / 27, szTmp2,127);
		DBG_MSG("Aid: lenth=%d pCtx->pcr=%s pts=%s\n",length, szTmp1, szTmp2);
		fwrite(pData, item_size, length,pCtx->fpoutaudio);
	}
#else
	int dataLen = item_size * length;
	JdDbg(CJdDbg::DBGLVL_STRM, ("strmid=%d length=%d",nStrmId, dataLen));
	if(nStrmId == STRM_ID_VID) {
		unsigned long ulFlags = 0;
		if(pCtx->fEoS) {
			ulFlags = OMX_BUFFERFLAG_EOS;
		}
		if(pCtx->fDisCont){
			ulFlags |= OMX_EXT_BUFFERFLAG_DISCONT;
		}
		while(pCtx->pConnVidOut->IsFull(pCtx->pConnVidOut)){
			JdDbg(CJdDbg::DBGLVL_STRM,("Waiting for free buffer"))
#ifdef WIN32
			Sleep(1);
#else
			usleep(1000);
#endif
		}
		JdDbg(CJdDbg::DBGLVL_PACKET, ("Sending Vid ulFlags=0x%x", ulFlags));
		pCtx->pConnVidOut->Write(pCtx->pConnVidOut, (char *)pData, dataLen,  ulFlags, pCtx->crnt_vid_pts * 1000 / 90);
		pCtx->VidChunkCnt++;
	} else {
		if(pCtx->pConnAudOut) {
			unsigned long ulFlags = 0;
			if(pCtx->fEoS) {
				ulFlags |= OMX_BUFFERFLAG_EOS;
			}
			if(pCtx->fDisCont){
				ulFlags |= OMX_EXT_BUFFERFLAG_DISCONT;
			}
			while(pCtx->pConnAudOut->IsFull(pCtx->pConnAudOut)){
				JdDbg(CJdDbg::DBGLVL_STRM,("Waiting for free buffer"));
#ifdef WIN32
				Sleep(1);
#else
				usleep(1000);
#endif
			}

			if(pCtx->aud_end_of_frame) {
				ulFlags  |= OMX_BUFFERFLAG_ENDOFFRAME;
			}
			JdDbg(CJdDbg::DBGLVL_STRM, ("Sending Aud ulFlags=0x%x", ulFlags));
			pCtx->pConnAudOut->Write(pCtx->pConnAudOut, (char *)pData, dataLen,  ulFlags, pCtx->crnt_aud_pts * 1000 / 90);
		}
		pCtx->AudChunkCnt++;
	}
#endif
	return 0;
}


int WriteStream(MpegTsDemuxCtx *pCtx, unsigned char *pData, int item_size, int length, int nPid)
{
	int dataLen = item_size * length;
	JdDbg(CJdDbg::DBGLVL_STRM, ("strmid=%d length=%d",nPid, dataLen));

	if(pCtx->mConnections.find( nPid ) != pCtx->mConnections.end()) {
		ConnCtxT *pConn = pCtx->mConnections[nPid];
		if(pConn) {
			unsigned long ulFlags = 0;
			if(pCtx->fEoS) {
				ulFlags = OMX_BUFFERFLAG_EOS;
			}
			if(pCtx->fDisCont){
				ulFlags |= OMX_EXT_BUFFERFLAG_DISCONT;
			}
			while(pConn->IsFull(pCtx->pConnVidOut)){
				JdDbg(CJdDbg::DBGLVL_STRM,("Waiting for free buffer"))
		#ifdef WIN32
				Sleep(1);
		#else
				usleep(1000);
		#endif
			}
			JdDbg(CJdDbg::DBGLVL_PACKET, ("Sending Vid ulFlags=0x%x", ulFlags));
			pConn->Write(pConn, (char *)pData, dataLen,  ulFlags, pCtx->crnt_vid_pts * 1000 / 90);
		}
	}
	return 0;
}

int demuxOpen(StrmCompIf *pComp, const char *pszOption)
{
	static int nInstanceId = 1;
	MpegTsDemuxCtx *pCtx = (MpegTsDemuxCtx *)pComp->pCtx;
	pCtx->nInstanceId = nInstanceId++;
	pCtx->fEoS = 0;

	pCtx->start_ts = 0;
	pCtx->read_position = 0;	
	pCtx->last_totalbytes = 0;
	pCtx->VidChunkCnt = 0;
	pCtx->AudChunkCnt = 0;
	pCtx->stat_update_interval = TIME_SECOND;
	// As a defualt set the auto program detection
	pCtx->detect_program_pids = 1;
	pCtx->program = 1;
	pCtx->video_channel = 1;
	pCtx->audio_channel = 1;
	pCtx->pcr_pid = 0xFFFF;
	pCtx->video_pid = 0xFFFF;
	pCtx->audio_pid = 0xFFFF;
	pCtx->audio_stream_type = 0xFFFF;
	pCtx->video_stream_type = 0xFFFF;

	demux_mpeg2_transport_init(pCtx);
}

int demuxSetOption(StrmCompIf *pComp, int nCmd, char *pOptionData)
{
	MpegTsDemuxCtx *pCtx = (MpegTsDemuxCtx *)pComp->pCtx;

	switch(nCmd)
	{ 
		case DEMUX_CMD_SELECT_PROGRAM:
		{
			DemuxSelectProgramT *pProg = (DemuxSelectProgramT *)pOptionData;
			pCtx->detect_program_pids = pProg->detect_program_pids;
			pCtx->program             = pProg->program;
			pCtx->audio_channel       = pProg->audio_channel;
			pCtx->video_channel       = pProg->video_channel;
			pCtx->pcr_pid             = pProg->pcr_pid;
			pCtx->audio_pid           = pProg->audio_pid;
			pCtx->video_pid           = pProg->video_pid;
			pCtx->audio_stream_type   = pProg->audio_stream_type;
			pCtx->video_stream_type   = pProg->video_stream_type;
			JdDbg(CJdDbg::DBGLVL_SETUP, ("detectpids=%d prog=%d vhan=%d achan=%d", pCtx->detect_program_pids, pCtx->program, pCtx->video_channel, pCtx->audio_channel));
			JdDbg(CJdDbg::DBGLVL_SETUP, ("pcr_pid=%d aud_pid=%d vid_pid=%d aud_strm_type=0x%x vid_strm_type=0x%x", pCtx->pcr_pid, pCtx->audio_pid, pCtx->video_pid,pCtx->audio_stream_type, pCtx->video_stream_type));
		}
		break;
	}

	return -1;
}

void demuxClose(StrmCompIf *pComp)
{
	MpegTsDemuxCtx *pCtx = NULL;
	JdDbg(CJdDbg::DBGLVL_TRACE, ("Enter"));
	pCtx = (MpegTsDemuxCtx *)pComp->pCtx;
	demux_mpeg2_transport_deinit(pCtx);
	JdDbg(CJdDbg::DBGLVL_TRACE, ("Leave"));
	//return 0;
}

int demuxSetInputConn(StrmCompIf *pComp, int nConnNum, ConnCtxT *pConn)
{
	MpegTsDemuxCtx *pCtx = (MpegTsDemuxCtx*)pComp->pCtx;
	pCtx->pConnSrc = pConn;
	return 0;
}

int demuxSetOutputConn(StrmCompIf *pComp, int nConnNum, ConnCtxT *pConn)
{
	MpegTsDemuxCtx *pCtx = (MpegTsDemuxCtx*)pComp->pCtx;
	pCtx->mConnections[nConnNum] = pConn;
/*
	if(nConnNum == 0) {
		pCtx->pConnAudOut = pConn;
	} else {
		pCtx->pConnVidOut = pConn;
	}
*/
	return 0;
}

static void threadDemux(void *threadsArg)
{
	MpegTsDemuxCtx *pCtx =  (MpegTsDemuxCtx *)threadsArg;
	char *pData = (char *)malloc(DMA_READ_SIZE);
	long long ullPts;
	unsigned long ulFlags = 0;
	JdDbg(CJdDbg::DBGLVL_TRACE, ("Enter"));
	while (pCtx->fRun)	{
		int length = 0;
		while(pCtx->pConnSrc->IsEmpty(pCtx->pConnSrc) && pCtx->nUiCmd != STRM_CMD_STOP){
			//JdDbg(CJdDbg::DBGLVL_WARN,("start=%d end=%d",pCtx->pConnSrc->pdpCtx->start, pCtx->pConnSrc->pdpCtx->end))
#ifdef WIN32
			Sleep(1);
#else
			usleep(1000);
#endif
		}

		if(pCtx->nUiCmd == STRM_CMD_STOP) {
			JdDbg(CJdDbg::DBGLVL_SETUP, ("Setting EoS due to User Command."));
			ulFlags = OMX_BUFFERFLAG_EOS;
		} else {
			length = pCtx->pConnSrc->Read(pCtx->pConnSrc, pData, DMA_READ_SIZE, &ulFlags, &ullPts);
		}

		if(ulFlags & OMX_BUFFERFLAG_EOS) {
			pCtx->fEoS = 1;
			WriteData(pCtx, NULL, 1, 0, STRM_ID_VID);
			WriteData(pCtx, NULL, 1, 0, STRM_ID_AUD);
		} else {
			demux_mpeg2_transport(pCtx, length, (unsigned char *)pData);
		}
		
		pCtx->read_position += length;
		//if(gDbgLevel >= DBGLVL_STAT) {
		//	DisplayStat(pCtx);
		//}

		if(pCtx->fEoS) {
			pCtx->fRun = 0;
			JdDbg(CJdDbg::DBGLVL_TRACE, ("Exiting Demux thread"));
		}
	}
	free(pData);
	JdDbg(CJdDbg::DBGLVL_TRACE, ("Leave"));
}


void MpegTsDemuxDeinit(MpegTsDemuxCtx *pCtx)
{
	// Do nothing for now
}

int demuxStart(StrmCompIf *pComp)
{
	MpegTsDemuxCtx * pCtx = (MpegTsDemuxCtx*)pComp->pCtx;
	JdDbg(CJdDbg::DBGLVL_TRACE, ("Enter"));
	pCtx->fRun = 1;
#ifdef WIN32
	{
		DWORD dwThreadId;
		pCtx->threadidDemux = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadDemux, pCtx, 0, &dwThreadId);
	}
#else
	if (pthread_create (&pCtx->threadidDemux, NULL, (thrdStartFcnPtr)threadDemux, pCtx) != 0)	{
		JdDbg(CJdDbg::DBGLVL_ERROR, ("Create_Task failed !"));
		assert(0);
	}
#endif
	JdDbg(CJdDbg::DBGLVL_TRACE, ("Leave"));
}

int demuxStop(StrmCompIf *pComp)
{
	MpegTsDemuxCtx * pCtx = (MpegTsDemuxCtx*)pComp->pCtx;
	void *ret_value;
	int nTimeOut = 1000000; // 1000 milli sec

	JdDbg(CJdDbg::DBGLVL_TRACE, ("Enter"));

	if(pCtx->fRun) {
		pCtx->nUiCmd = STRM_CMD_STOP;
		JdDbg(CJdDbg::DBGLVL_TRACE, ("fStreaming=%d", pCtx->fRun));
		while(pCtx->fRun && nTimeOut > 0) {
			nTimeOut -= 1000;
#ifdef WIN32
			Sleep(1);
#else
			usleep(1000);
#endif
		}
		// If thread did not exit, force close it
		JdDbg(CJdDbg::DBGLVL_TRACE, ("fStreaming=%d nTimeOut rem=%d", pCtx->fRun, nTimeOut));
	}

	if(pCtx->fRun) {
		pCtx->fRun = 0;
#ifdef WIN32
		WaitForSingleObject(pCtx->threadidDemux, 1000);
#else
		//pthread_cancel(pCtx->threadidDemux);
		pthread_join (pCtx->threadidDemux, (void **) &ret_value);
#endif
	}
	JdDbg(CJdDbg::DBGLVL_TRACE, ("Leave"));
}

int demuxSetClkSrc(StrmCompIf *pComp, void *pClkSrc)
{
	MpegTsDemuxCtx * pCtx = (MpegTsDemuxCtx *)pComp->pCtx;
	pCtx->pClk = pClkSrc;
	return 0;
}

void *demuxGetClkSrc(StrmCompIf *pComp)
{
	MpegTsDemuxCtx * pCtx = (MpegTsDemuxCtx *)pComp->pCtx;
	pCtx->fClkSrc = 1;//fEnable;
	pCtx->pClk = clkCreate(1);
	return pCtx->pClk;
}


void parse_ac3_audio(MpegTsDemuxCtx *pCtx, unsigned char *es_ptr, unsigned int length, unsigned long long pts, unsigned int first_access_unit, int end_of_frame)
{
#if 0
	unsigned int	i, j;
	static int		first_header = FALSE;
	static int		second_header = FALSE;
	static int		audio_synced = FALSE;
	static int		first_synced = FALSE;
	static unsigned int	parse = 0;
	static unsigned int	header_parse = 0;
	static unsigned int	frame_size = 0;
	static unsigned int	frame_size_check = 0;
	static unsigned char	frame_buffer[128][3840 + 8];
	static unsigned char	frame_buffer_start = 0x0b;
	static unsigned int	frame_buffer_index = 0;
	static unsigned int	frame_buffer_count = 0;
	static unsigned int	frame_buffer_length[128];
	static unsigned long long	frame_buffer_pts[128];
	static unsigned long long	current_pts;
	static unsigned long long	current_pts_saved;
	static unsigned int	current_pts_valid = FALSE;
	static unsigned int	audio_sampling_rate;
	static unsigned int	audio_bitrate;
	static unsigned int	audio_bsid;
	static unsigned int	audio_bsmod;
	static unsigned int	audio_acmod;

	if(pCtx->parse_only == FALSE)  {
		if(audio_synced == TRUE)  {
			WriteData(pCtx, es_ptr, 1, length, STRM_ID_AUD);
		}
	}
	if (audio_synced == FALSE)  {
		if (first_access_unit == TRUE)  {
			current_pts_saved = pts;
			current_pts_valid = TRUE;
		}
		for(i = 0; i < length; i++)  {
			parse = (parse << 8) + *es_ptr++;
			if ((parse & 0xffff) == 0x00000b77)  {
				if(current_pts_valid == TRUE)  {
					current_pts = current_pts_saved;
					current_pts_valid = FALSE;
				}
				else  {
					if (frame_size_check != 0)  {
						if (frame_buffer_index == frame_size_check)  {
							current_pts += ((1536 * 90000) / audio_sampling_rate);
						}
					}
				}
				if(first_header == FALSE)  {
					header_parse = 5;
					first_header = TRUE;
					frame_buffer_pts[frame_buffer_count] = current_pts;
				}
				else if (second_header == FALSE)  {
					if(frame_size == 5)  {
						second_header = TRUE;
						DBG_MSG("Audio Bitrate = %d, Audio Sampling Rate = %d\n", audio_bitrate, audio_sampling_rate);
						switch (audio_acmod & 0x7)  {
							case 7:
								DBG_MSG("Audio Mode = 3/2, bsid = %d, bsmod = %d\n", audio_bsid, audio_bsmod);
								break;
							case 6:
								DBG_MSG("Audio Mode = 2/2, bsid = %d, bsmod = %d\n", audio_bsid, audio_bsmod);
								break;
							case 5:
								DBG_MSG("Audio Mode = 3/1, bsid = %d, bsmod = %d\n", audio_bsid, audio_bsmod);
								break;
							case 4:
								DBG_MSG("Audio Mode = 2/1, bsid = %d, bsmod = %d\n", audio_bsid, audio_bsmod);
								break;
							case 3:
								DBG_MSG("Audio Mode = 3/0, bsid = %d, bsmod = %d\n", audio_bsid, audio_bsmod);
								break;
							case 2:
								DBG_MSG("Audio Mode = 2/0, bsid = %d, bsmod = %d\n", audio_bsid, audio_bsmod);
								break;
							case 1:
								DBG_MSG("Audio Mode = 1/0, bsid = %d, bsmod = %d\n", audio_bsid, audio_bsmod);
								break;
							case 0:
								DBG_MSG("Audio Mode = 1+1, bsid = %d, bsmod = %d\n", audio_bsid, audio_bsmod);
								break;
						}
						if (audio_synced == FALSE)  {
							frame_buffer_length[frame_buffer_count] = frame_buffer_index;
							frame_buffer_index = 0;
							frame_buffer_count++;
							frame_buffer_count &= 0x7f;
							frame_buffer_pts[frame_buffer_count] = current_pts;
						}
					}
					else  {
						first_header = FALSE;
						frame_buffer_count = 0;
						frame_buffer_index = 0;
					}
				}
				else  {
					if (audio_synced == FALSE)  {
						if (frame_buffer_index == frame_size_check)  {
							frame_buffer_length[frame_buffer_count] = frame_buffer_index;
							frame_buffer_index = 0;
							frame_buffer_count++;
							frame_buffer_count &= 0x7f;
							frame_buffer_pts[frame_buffer_count] = current_pts;
						}
					}
				}
			}
			else if (header_parse != 0)  {
				--header_parse;
				if(header_parse == 2)  {
					switch ((parse & 0xc0) >> 6)  {
						case 3:
							audio_sampling_rate = 0;
							break;
						case 2:
							audio_sampling_rate = 32000;
							break;
						case 1:
							audio_sampling_rate = 44100;
							break;
						case 0:
							audio_sampling_rate = 48000;
							break;
					}
					switch ((parse & 0x3f) >> 1)  {
						case 18:
							audio_bitrate = 640000;
							break;
						case 17:
							audio_bitrate = 576000;
							break;
						case 16:
							audio_bitrate = 512000;
							break;
						case 15:
							audio_bitrate = 448000;
							break;
						case 14:
							audio_bitrate = 384000;
							break;
						case 13:
							audio_bitrate = 320000;
							break;
						case 12:
							audio_bitrate = 256000;
							break;
						case 11:
							audio_bitrate = 224000;
							break;
						case 10:
							audio_bitrate = 192000;
							break;
						case 9:
							audio_bitrate = 160000;
							break;
						case 8:
							audio_bitrate = 128000;
							break;
						case 7:
							audio_bitrate = 112000;
							break;
						case 6:
							audio_bitrate = 96000;
							break;
						case 5:
							audio_bitrate = 80000;
							break;
						case 4:
							audio_bitrate = 64000;
							break;
						case 3:
							audio_bitrate = 56000;
							break;
						case 2:
							audio_bitrate = 48000;
							break;
						case 1:
							audio_bitrate = 40000;
							break;
						case 0:
							audio_bitrate = 32000;
							break;
						default:
							audio_bitrate = 0;
					}
				}
				else if(header_parse == 1)  {
					audio_bsid = (parse & 0xf8) >> 3;
					audio_bsmod = parse & 0x7;
				}
				else if(header_parse == 0)  {
					audio_acmod = (parse & 0xe0) >> 5;
					if(audio_sampling_rate == 0 || audio_bitrate == 0)  {
						first_header = FALSE;
					}
					else  {
						frame_size = audio_bitrate * 192 / audio_sampling_rate;
						frame_size_check = audio_bitrate * 192 / audio_sampling_rate;
					}
				}
			}
			if(audio_synced == FALSE && first_header == TRUE && second_header == TRUE)  {
				if (pts_aligned != 0xffffffffffffffffLL || pCtx->video_channel == 0)  {
					if (current_pts >= pts_aligned || pCtx->video_channel == 0)  {
						audio_synced = TRUE;
						frame_buffer_length[frame_buffer_count] = frame_buffer_index;
						for (j = 0; j <= frame_buffer_count; j++)  {
							if ((frame_buffer_pts[j] + 2800) > pts_aligned || pCtx->video_channel == 0)  {
#if 0
								DBG_MSG("j = %d, pts = 0x%08x, length = %d\n", j, (unsigned int)frame_buffer_pts[j], frame_buffer_length[j]);
#endif
								if (first_synced == FALSE)  {
									first_synced = TRUE;
									if (pCtx->video_channel == 0)  {
										DBG_MSG("First Audio PTS = 0x%08x\n", (unsigned int)frame_buffer_pts[j]);
									}
									else  {
										DBG_MSG("First Audio PTS = 0x%08x, %d\n", (unsigned int)frame_buffer_pts[j], (unsigned int)(frame_buffer_pts[j] - pts_aligned));
									}
									if (pCtx->parse_only == FALSE)  {
										WriteData(pCtx, &frame_buffer_start, 1, 1, STRM_ID_AUD);
									}
								}
								if (pCtx->parse_only == FALSE)  {
									WriteData(pCtx, &frame_buffer[j][0], 1, frame_buffer_length[j], STRM_ID_AUD);
								}
							}
						}
						if (pCtx->parse_only == FALSE)  {
							WriteData(pCtx, es_ptr - 1, 1, length - i, STRM_ID_AUD);
						}
					}
					else  {
						--frame_size;
						frame_buffer[frame_buffer_count][frame_buffer_index++] = (unsigned char)parse & 0xff;
						if(frame_buffer_index == (3840 + 8))  {
							--frame_buffer_index;
						}
					}
				}
				else  {
					--frame_size;
					frame_buffer[frame_buffer_count][frame_buffer_index++] = (unsigned char)parse & 0xff;
					if(frame_buffer_index == (3840 + 8))  {
						--frame_buffer_index;
					}
				}
			}
			else if (first_header == TRUE)  {
				--frame_size;
				frame_buffer[frame_buffer_count][frame_buffer_index++] = (unsigned char)parse & 0xff;
				if(frame_buffer_index == (3840 + 8))  {
					--frame_buffer_index;
				}
			}
		}
	}
#else
	pCtx->crnt_aud_pts = pts;
	pCtx->aud_end_of_frame = end_of_frame;
	//Swap data
	{
		int i;
		for(i=0; i < length - 1; i = i + 2){
			unsigned char tmp;
			tmp = es_ptr[i];
			es_ptr[i] = es_ptr[i+1];
			es_ptr[i+1] = tmp;
		}
	}
	WriteData(pCtx, es_ptr, 1, length, STRM_ID_AUD);
#endif
}

void parse_mp2_audio(unsigned char *es_ptr, unsigned int length, unsigned long long pts, unsigned int first_access_unit)
{
#if 0
	unsigned int	i, j;
	static int		first_header = FALSE;
	static int		second_header = FALSE;
	static int		audio_synced = FALSE;
	static int		first_synced = FALSE;
	static unsigned int	parse = 0;
	static unsigned int	header_parse = 0;
	static unsigned int	frame_size = 0;
	static unsigned int	frame_size_check = 0;
	static unsigned char	frame_buffer[128][3840 + 8];
	static unsigned char	frame_buffer_start = 0xff;
	static unsigned int	frame_buffer_index = 0;
	static unsigned int	frame_buffer_count = 0;
	static unsigned int	frame_buffer_length[128];
	static unsigned long long	frame_buffer_pts[128];
	static unsigned long long	current_pts;
	static unsigned long long	current_pts_saved;
	static unsigned int	current_pts_valid = FALSE;
	static unsigned int	audio_sampling_rate;
	static unsigned int	audio_bitrate;
	static unsigned int	audio_mode;
	static unsigned int	audio_mode_ext;
	static unsigned int	audio_copyright;
	static unsigned int	audio_original;
	static unsigned int	audio_emphasis;

	MpegTsDemuxCtx *pCtx = pCtx;

	if(pCtx->parse_only == FALSE)  {
		if(audio_synced == TRUE)  {
			WriteData(pCtx, es_ptr, 1, length, STRM_ID_AUD);
		}
	}
	if (audio_synced == FALSE)  {
		if (first_access_unit == TRUE)  {
			current_pts_saved = pts;
			current_pts_valid = TRUE;
		}
		for(i = 0; i < length; i++)  {
			parse = (parse << 8) + *es_ptr++;
#if 1
			if ((parse & 0xffff) == 0x0000fffc || (parse & 0xffff) == 0x0000fffd)  {
#else
			if ((parse & 0xffff) == 0x0000fffc)  {
#endif
				if(current_pts_valid == TRUE)  {
					current_pts = current_pts_saved;
					current_pts_valid = FALSE;
				}
				else  {
					if (frame_size_check != 0)  {
						if (frame_buffer_index == frame_size_check)  {
							current_pts += ((1152 * 90000) / audio_sampling_rate);
						}
					}
				}
				if(first_header == FALSE)  {
					header_parse = 2;
					first_header = TRUE;
					frame_buffer_pts[frame_buffer_count] = current_pts;
				}
				else if (second_header == FALSE)  {
					if(frame_size == 2)  {
						second_header = TRUE;
						DBG_MSG("Audio Bitrate = %d, Audio Sampling Rate = %d\n", audio_bitrate, audio_sampling_rate);
						switch (audio_mode & 0x3)  {
							case 3:
								DBG_MSG("Audio Mode = Single Channel, mode_extension = %d\n", audio_mode_ext);
								break;
							case 2:
								DBG_MSG("Audio Mode = Dual Channel, mode_extension = %d\n", audio_mode_ext);
								break;
							case 1:
								DBG_MSG("Audio Mode = Joint Stereo, mode_extension = %d\n", audio_mode_ext);
								break;
							case 0:
								DBG_MSG("Audio Mode = Stereo, mode_extension = %d\n", audio_mode_ext);
								break;
						}
						switch (audio_emphasis & 0x3)  {
							case 3:
								DBG_MSG("Audio Emphasis = CCITT J.17, copyright = %d, original = %d\n", audio_copyright, audio_original);
								break;
							case 2:
								DBG_MSG("Audio Emphasis = Reserved, copyright = %d, original = %d\n", audio_copyright, audio_original);
								break;
							case 1:
								DBG_MSG("Audio Emphasis = 50/15 usec, copyright = %d, original = %d\n", audio_copyright, audio_original);
								break;
							case 0:
								DBG_MSG("Audio Emphasis = None, copyright = %d, original = %d\n", audio_copyright, audio_original);
								break;
						}
						if (audio_synced == FALSE)  {
							frame_buffer_length[frame_buffer_count] = frame_buffer_index;
							frame_buffer_index = 0;
							frame_buffer_count++;
							frame_buffer_count &= 0x7f;
							frame_buffer_pts[frame_buffer_count] = current_pts;
						}
					}
					else  {
						first_header = FALSE;
						frame_buffer_count = 0;
						frame_buffer_index = 0;
					}
				}
				else  {
					if (audio_synced == FALSE)  {
						if (frame_buffer_index == frame_size_check)  {
							frame_buffer_length[frame_buffer_count] = frame_buffer_index;
							frame_buffer_index = 0;
							frame_buffer_count++;
							frame_buffer_count &= 0x7f;
							frame_buffer_pts[frame_buffer_count] = current_pts;
						}
					}
				}
			}
			else if (header_parse != 0)  {
				--header_parse;
				if(header_parse == 1)  {
					switch ((parse & 0xc) >> 2)  {
						case 3:
							audio_sampling_rate = 0;
							break;
						case 2:
							audio_sampling_rate = 32000;
							break;
						case 1:
							audio_sampling_rate = 48000;
							break;
						case 0:
							audio_sampling_rate = 44100;
							break;
					}
					switch ((parse & 0xf0) >> 4)  {
						case 14:
							audio_bitrate = 384000;
							break;
						case 13:
							audio_bitrate = 320000;
							break;
						case 12:
							audio_bitrate = 256000;
							break;
						case 11:
							audio_bitrate = 224000;
							break;
						case 10:
							audio_bitrate = 192000;
							break;
						case 9:
							audio_bitrate = 160000;
							break;
						case 8:
							audio_bitrate = 128000;
							break;
						case 7:
							audio_bitrate = 112000;
							break;
						case 6:
							audio_bitrate = 96000;
							break;
						case 5:
							audio_bitrate = 80000;
							break;
						case 4:
							audio_bitrate = 64000;
							break;
						case 3:
							audio_bitrate = 56000;
							break;
						case 2:
							audio_bitrate = 48000;
							break;
						case 1:
							audio_bitrate = 32000;
							break;
						case 0:
							audio_bitrate = 0;
							break;
						default:
							audio_bitrate = 0;
					}
				}
				else if(header_parse == 0)  {
					audio_mode = (parse & 0xc0) >> 6;
					audio_mode_ext = (parse & 0x30) >> 4;
					audio_copyright = (parse & 0x8) >> 3;
					audio_original = (parse & 0x4) >> 2;
					audio_emphasis = parse & 0x3;
					if(audio_sampling_rate == 0 || audio_bitrate == 0)  {
						first_header = FALSE;
					}
					else  {
						frame_size = audio_bitrate * 144 / audio_sampling_rate;
						frame_size_check = audio_bitrate * 144 / audio_sampling_rate;
					}
				}
			}
			if(audio_synced == FALSE && first_header == TRUE && second_header == TRUE)  {
				if (pCtx->pts_aligned != 0xffffffffffffffffLL || pCtx->video_channel == 0)  {
					if (current_pts >= pCtx->pts_aligned || pCtx->video_channel == 0)  {
						audio_synced = TRUE;
						frame_buffer_length[frame_buffer_count] = frame_buffer_index;
						for (j = 0; j <= frame_buffer_count; j++)  {
							if ((frame_buffer_pts[j] + 2160) > pCtx->pts_aligned || pCtx->video_channel == 0)  {
#if 0
								DBG_MSG("j = %d, pts = 0x%08x, length = %d\n", j, (unsigned int)frame_buffer_pts[j], frame_buffer_length[j]);
#endif
								if (first_synced == FALSE)  {
									first_synced = TRUE;
									if (pCtx->video_channel == 0)  {
										DBG_MSG("First Audio PTS = 0x%08x\n", (unsigned int)frame_buffer_pts[j]);
									}
									else  {
										DBG_MSG("First Audio PTS = 0x%08x, %d\n", (unsigned int)frame_buffer_pts[j], (unsigned int)(frame_buffer_pts[j] - pCtx->pts_aligned));
									}
									if (pCtx->parse_only == FALSE)  {
										WriteData(pCtx, &frame_buffer_start, 1, 1, STRM_ID_AUD);
									}
								}
								if (pCtx->parse_only == FALSE)  {
									WriteData(pCtx, &frame_buffer[j][0], 1, frame_buffer_length[j], STRM_ID_AUD);
								}
							}
						}
						if (pCtx->parse_only == FALSE)  {
							WriteData(pCtx, es_ptr - 1, 1, length - i, STRM_ID_AUD);
						}
					}
					else  {
						--frame_size;
						frame_buffer[frame_buffer_count][frame_buffer_index++] = (unsigned char)parse & 0xff;
						if(frame_buffer_index == (3840 + 8))  {
							--frame_buffer_index;
						}
					}
				}
				else  {
					--frame_size;
					frame_buffer[frame_buffer_count][frame_buffer_index++] = (unsigned char)parse & 0xff;
					if(frame_buffer_index == (3840 + 8))  {
						--frame_buffer_index;
					}
				}
			}
			else if (first_header == TRUE)  {
				--frame_size;
				frame_buffer[frame_buffer_count][frame_buffer_index++] = (unsigned char)parse & 0xff;
				if(frame_buffer_index == (3840 + 8))  {
					--frame_buffer_index;
				}
			}
		}
	}
#else
	//return 0;
#endif
}

void parse_lpcm_audio(MpegTsDemuxCtx *pCtx, unsigned char *es_ptr, unsigned int length, unsigned long long pts, unsigned int first_access_unit, unsigned short lpcm_header_flags)
{
#if 0
	unsigned int	i, j, channels, sample_bytes, adjusted_length, index = 0;
	static unsigned int	sample;
	static unsigned int	first_header_dump = FALSE;
	static unsigned int	extra_bytes = 0;
	static unsigned char	extra_bytes_buffer[4];
	static unsigned char	temp_buffer[188];
	static unsigned char	null_bytes[4] = {0, 0, 0, 0};

	if (first_access_unit == TRUE)  {
		if (sample != 0)  {
			extra_bytes = 0;
		}
	}
	for(i = 0; i < extra_bytes; i++)  {
		temp_buffer[index++] = extra_bytes_buffer[i];
	}
	for(i = 0; i < length; i++)  {
		temp_buffer[index++] = *es_ptr++;
	}
	length = length + extra_bytes;
	es_ptr = &temp_buffer[0];
	if (first_header_dump == FALSE)  {
		first_header_dump = TRUE;
		switch ((lpcm_header_flags & 0xf000) >> 12)  {
			case 1:
				DBG_MSG("LPCM Audio Mode = 1/0\n");
				break;
			case 3:
				DBG_MSG("LPCM Audio Mode = 2/0\n");
				break;
			case 4:
				DBG_MSG("LPCM Audio Mode = 3/0\n");
				break;
			case 5:
				DBG_MSG("LPCM Audio Mode = 2/1\n");
				break;
			case 6:
				DBG_MSG("LPCM Audio Mode = 3/1\n");
				break;
			case 7:
				DBG_MSG("LPCM Audio Mode = 2/2\n");
				break;
			case 8:
				DBG_MSG("LPCM Audio Mode = 3/2\n");
				break;
			case 9:
				DBG_MSG("LPCM Audio Mode = 3/2+lfe\n");
				break;
			case 10:
				DBG_MSG("LPCM Audio Mode = 3/4\n");
				break;
			case 11:
				DBG_MSG("LPCM Audio Mode = 3/4+lfe\n");
				break;
			default:
				DBG_MSG("LPCM Audio Mode = reserved\n");
				break;
		}
		switch ((lpcm_header_flags & 0xc0) >> 6)  {
			case 1:
				DBG_MSG("LPCM Audio Bits/sample = 16\n");
				break;
			case 2:
				DBG_MSG("LPCM Audio Bits/sample = 20\n");
				break;
			case 3:
				DBG_MSG("LPCM Audio Bits/sample = 24\n");
				break;
			default:
				DBG_MSG("LPCM Audio Bits/sample = reserved\n");
				break;
		}
		switch ((lpcm_header_flags & 0xf00) >> 8)  {
			case 1:
				DBG_MSG("LPCM Audio Sample Rate = 48000\n");
				break;
			case 4:
				DBG_MSG("LPCM Audio Sample Rate = 96000\n");
				break;
			case 5:
				DBG_MSG("LPCM Audio Sample Rate = 192000\n");
				break;
			default:
				DBG_MSG("LPCM Audio Sample Rate = reserved\n");
				break;
		}
	}
	switch ((lpcm_header_flags & 0xf000) >> 12)  {
		case 1:
			channels = 2;
			break;
		case 3:
			channels = 2;
			break;
		case 4:
			channels = 4;
			break;
		case 5:
			channels = 4;
			break;
		case 6:
			channels = 4;
			break;
		case 7:
			channels = 4;
			break;
		case 8:
			channels = 6;
			break;
		case 9:
			channels = 6;
			break;
		case 10:
			channels = 8;
			break;
		case 11:
			channels = 8;
			break;
	}
	switch ((lpcm_header_flags & 0xc0) >> 6)  {
		case 1:
			sample_bytes = 2;
			break;
		case 2:
			sample_bytes = 3;
			break;
		case 3:
			sample_bytes = 3;
			break;
	}
	if (first_access_unit == TRUE)  {
		if (sample != 0)  {
			DBG_MSG("LPCM sample resync, adding %d samples\n", channels - sample);
			for (i = 0; i < (channels - sample); i++)  {
				WriteData(pCtx, &null_bytes[0], 1, sample_bytes, STRM_ID_AUD);
			}
			sample = 0;
		}
	}
	i = 0;
	j = length / sample_bytes;
	adjusted_length = j * sample_bytes;
	extra_bytes = length - adjusted_length;
	while (i < adjusted_length)  {
		switch (sample)  {
			case 0:
				if(pCtx->parse_only == FALSE)  {
					WriteData(pCtx, es_ptr, 1, sample_bytes, STRM_ID_AUD);
				}
				es_ptr += sample_bytes;
				i += sample_bytes;
				sample++;
				if (sample == channels)  {
					sample = 0;
				}
				break;
			case 1:
				if(pCtx->parse_only == FALSE)  {
					WriteData(pCtx, es_ptr, 1, sample_bytes, STRM_ID_AUD);
				}
				es_ptr += sample_bytes;
				i += sample_bytes;
				sample++;
				if (sample == channels)  {
					sample = 0;
				}
				break;
			case 2:
				if(pCtx->parse_only == FALSE && pCtx->lpcm_mode == FALSE)  {
					WriteData(pCtx, es_ptr, 1, sample_bytes, STRM_ID_AUD);
				}
				es_ptr += sample_bytes;
				i += sample_bytes;
				sample++;
				if (sample == channels)  {
					sample = 0;
				}
				break;
			case 3:
				if(pCtx->parse_only == FALSE && pCtx->lpcm_mode == FALSE)  {
					WriteData(pCtx, es_ptr, 1, sample_bytes, STRM_ID_AUD);
				}
				es_ptr += sample_bytes;
				i += sample_bytes;
				sample++;
				if (sample == channels)  {
					sample = 0;
				}
				break;
			case 4:
				if(pCtx->parse_only == FALSE && pCtx->lpcm_mode == FALSE)  {
					WriteData(pCtx, es_ptr, 1, sample_bytes, STRM_ID_AUD);
				}
				es_ptr += sample_bytes;
				i += sample_bytes;
				sample++;
				if (sample == channels)  {
					sample = 0;
				}
				break;
			case 5:
				if(pCtx->parse_only == FALSE && pCtx->lpcm_mode == FALSE)  {
					WriteData(pCtx, es_ptr, 1, sample_bytes, STRM_ID_AUD);
				}
				es_ptr += sample_bytes;
				i += sample_bytes;
				sample++;
				if (sample == channels)  {
					sample = 0;
				}
				break;
			case 6:
				if(pCtx->parse_only == FALSE && pCtx->lpcm_mode == FALSE)  {
					WriteData(pCtx, es_ptr, 1, sample_bytes, STRM_ID_AUD);
				}
				es_ptr += sample_bytes;
				i += sample_bytes;
				sample++;
				if (sample == channels)  {
					sample = 0;
				}
				break;
			case 7:
				if(pCtx->parse_only == FALSE && pCtx->lpcm_mode == FALSE)  {
					WriteData(pCtx, es_ptr, 1, sample_bytes, STRM_ID_AUD);
				}
				es_ptr += sample_bytes;
				i += sample_bytes;
				sample++;
				if (sample == channels)  {
					sample = 0;
				}
				break;
		}
	}
	for(i = 0; i < extra_bytes; i++)  {
		extra_bytes_buffer[i] = *es_ptr++;
	}
#else
	//return 0;
#endif
}

void parse_aac_audio(MpegTsDemuxCtx *pCtx, unsigned char *es_ptr, unsigned int length, unsigned long long pts, unsigned int first_access_unit, int end_of_frame)
{
	pCtx->crnt_aud_pts = pts;
	pCtx->aud_end_of_frame = end_of_frame;
	WriteData(pCtx, es_ptr, 1, length, STRM_ID_AUD);
}

void parse_mpeg2_video(MpegTsDemuxCtx *pCtx, unsigned char *es_ptr, unsigned int length, unsigned long long pts, unsigned int dts)
{
#if UNIT_TEST_DEMUX
	unsigned int	i, j;
	static int		first = TRUE;
	static int		first_sequence = FALSE;
	static int		first_sequence_dump = FALSE;
	static int		look_for_gop = FALSE;
	static int		gop_found = FALSE;
	static unsigned int	parse = 0;
	static unsigned int	picture_parse = 0;
	static unsigned int	extension_parse = 0;
	static unsigned int	picture_coding_parse = 0;
	static unsigned int	sequence_header_parse = 0;
	static unsigned int	sequence_extension_parse = 0;
	static unsigned int	picture_size = 0;
	static unsigned int	picture_count = 0;
	static unsigned int	time_code_field = 0;
	static unsigned int	time_code_rate = 1;
	static long double	frame_rate = 1.0;
	static unsigned char	header[3] = {0x0, 0x0, 0x1};
	static unsigned char	gop_header[9] = {0xb8, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00};
	static unsigned int	progressive_sequence;
	static unsigned long long	first_pts;
	static unsigned int	first_pts_count = 0;
	static unsigned int	extra_byte = FALSE;
	static unsigned int	last_temporal_reference = 0;
	static unsigned int	last_gop_temporal_reference = 0;
	static unsigned int	running_average_start = 0;
	static unsigned int	running_average_count = 0;
	static unsigned int	running_average_frames = 0;
	static unsigned int	running_average_samples[1024];
	static unsigned int	running_average_fields[1024];
	unsigned int	temporal_reference, temp_flags;
	unsigned int	picture_coding_type;
	unsigned int	whole_buffer = TRUE;
	unsigned char	*start_es_ptr;
	unsigned char	*middle_es_ptr;
	unsigned int	middle_length = 0x55555555;
	unsigned char	hours, minutes, seconds, pictures;
	unsigned char	temp_temporal_reference;
	long double		temp_running_average;
	long double		temp_running_fields;
	int			result, offset;

	start_es_ptr = es_ptr;
	for(i = 0; i < length; i++)  {
		parse = (parse << 8) + *es_ptr++;
		if (parse == 0x00000100)  {
			picture_parse = 2;
			if (first_sequence == TRUE)  {
				pCtx->coded_frames++;
			}
			if (first == TRUE)  {
				picture_size = 0;
			}
			else  {
#if 0
				DBG_MSG("%8ld\n", picture_size * 8);
#endif
				running_average_samples[running_average_frames] = picture_size * 8;
				picture_size = 0;
			}
			if(look_for_gop == TRUE)  {
				look_for_gop = FALSE;
				if(gop_found == FALSE)  {
					if(pCtx->parse_only == FALSE)  {
						j = time_code_rate * 60 * 60;
						hours = ((time_code_field / 2) / j) % 24;
						j /= 60;
						minutes = ((time_code_field / 2) / j) % 60;
						j /= 60;
						seconds = ((time_code_field / 2) / j) % 60;
						pictures = ((time_code_field / 2) % j);
						gop_header[1] = 0x00;
						gop_header[2] = 0x08;
						gop_header[3] = 0x00;
						gop_header[4] = 0x00;
						gop_header[1] |= (hours << 2) & 0x7c;
						gop_header[1] |= (minutes >> 4) & 0x03;
						gop_header[2] |= (minutes << 4) & 0xf0;
						gop_header[2] |= (seconds >> 3) & 0x07;
						gop_header[3] |= (seconds << 5) & 0xe0;
						gop_header[3] |= (pictures >> 1) & 0x1f;
						gop_header[4] |= (pictures << 7) & 0x80;
						if(middle_length == 0x55555555)  {
							WriteData(pCtx, start_es_ptr, 1, i, STRM_ID_VID);
						}
						else  {
							WriteData(pCtx, middle_es_ptr, 1, middle_length - (length - i), STRM_ID_VID);
						}
						WriteData(pCtx, (unsigned char *)&gop_header, 1, 9, STRM_ID_VID);
						middle_es_ptr = es_ptr;
						middle_length = length - i - 1;
						whole_buffer = FALSE;
					}
				}
			}
		}
		else if (parse == 0x000001b3)  {
			sequence_header_parse = 7;
			look_for_gop = TRUE;
			gop_found = FALSE;
			last_gop_temporal_reference = last_temporal_reference;
			if (first_sequence == FALSE)  {
				DBG_MSG("Sequence Header found\n");
				DBG_MSG("%d frames before first Sequence Header\n", picture_count);
				if(pCtx->parse_only == FALSE)  {
					WriteData(pCtx, (unsigned char *)&header, 1, 3, STRM_ID_VID);
					middle_es_ptr = es_ptr - 1;
					middle_length = length - i;
					whole_buffer = FALSE;
				}
				first_sequence = TRUE;
				picture_count = 0;
				time_code_field = 0;
				first_pts_count = 2;
			}
			else  {
				picture_count = 0;
			}
		}
		else if (sequence_header_parse != 0)  {
			--sequence_header_parse;
			if (first_sequence_dump == FALSE)  {
				switch (sequence_header_parse)  {
					case 6:
						break;
					case 5:
						break;
					case 4:
						DBG_MSG("Horizontal size = %d\n", (parse & 0xfff000) >> 12);
						DBG_MSG("Vertical size = %d\n", parse & 0xfff);
						break;
					case 3:
						switch ((parse & 0xf0) >> 4)  {
							case 0:
								DBG_MSG("Aspect ratio = forbidden\n");
								break;
							case 1:
								DBG_MSG("Aspect ratio = square samples\n");
								break;
							case 2:
								DBG_MSG("Aspect ratio = 4:3\n");
								break;
							case 3:
								DBG_MSG("Aspect ratio = 16:9\n");
								break;
							case 4:
								DBG_MSG("Aspect ratio = 2.21:1\n");
								break;
							default:
								DBG_MSG("Aspect ratio = reserved\n");
								break;
						}
						switch (parse & 0xf)  {
							case 0:
								DBG_MSG("Frame rate = forbidden\n");
								time_code_rate = 1;
								frame_rate = 1.0;
								break;
							case 1:
								DBG_MSG("Frame rate = 23.976\n");
								time_code_rate = 24;
								frame_rate = 24.0 * (1000.0 / 1001.0);
								break;
							case 2:
								DBG_MSG("Frame rate = 24\n");
								time_code_rate = 24;
								frame_rate = 24.0;
								break;
							case 3:
								DBG_MSG("Frame rate = 25\n");
								time_code_rate = 25;
								frame_rate = 25.0;
								break;
							case 4:
								DBG_MSG("Frame rate = 29.97\n");
								time_code_rate = 30;
								frame_rate = 30.0 * (1000.0 / 1001.0);
								break;
							case 5:
								DBG_MSG("Frame rate = 30\n");
								time_code_rate = 30;
								frame_rate = 30.0;
								break;
							case 6:
								DBG_MSG("Frame rate = 50\n");
								time_code_rate = 50;
								frame_rate = 50.0;
								break;
							case 7:
								DBG_MSG("Frame rate = 59.94\n");
								time_code_rate = 60;
								frame_rate = 60.0 * (1000.0 / 1001.0);
								break;
							case 8:
								DBG_MSG("Frame rate = 60\n");
								time_code_rate = 60;
								frame_rate = 60.0;
								break;
							default:
								DBG_MSG("Frame rate = reserved\n");
								break;
						}
						break;
					case 2:
						break;
					case 1:
						break;
					case 0:
						DBG_MSG("Sequence header bitrate = %d bps\n", ((parse & 0xffffc0) >> 6) * 400);
						break;
				}
			}
		}
		else if (picture_parse != 0)  {
			--picture_parse;
			switch (picture_parse)  {
				case 1:
					if(gop_found == FALSE)  {
						if(i == (length - 1))  {
							length -= 1;
							if(whole_buffer == FALSE)  {
								middle_length -= 1;
							}
							extra_byte = TRUE;
						}
					}
					break;
				case 0:
					temporal_reference = (parse & 0xffff) >> 6;
					if(dts == 1)  {
						last_temporal_reference = temporal_reference;
					}
					if(temporal_reference >= (last_gop_temporal_reference + 1))  {
						temporal_reference = temporal_reference - (last_gop_temporal_reference + 1);
					}
					else  {
						temporal_reference = (temporal_reference + 1024) - (last_gop_temporal_reference + 1);
					}
					if(extra_byte == TRUE)  {
						extra_byte = FALSE;
						temp_temporal_reference = (temporal_reference >> 2) & 0xff;
						if(gop_found == FALSE)  {
							if(pCtx->parse_only == FALSE)  {
								WriteData(pCtx, &temp_temporal_reference, 1, 1, STRM_ID_VID);
							}
							*(es_ptr - 1) = (unsigned char)(((temporal_reference & 0x3) << 6) | (parse & 0x3f));
						}
					}
					else  {
						if(gop_found == FALSE)  {
							*(es_ptr - 2) = (temporal_reference >> 2) & 0xff;
							*(es_ptr - 1) = (unsigned char)(((temporal_reference & 0x3) << 6) | (parse & 0x3f));
						}
					}
					picture_coding_type = (parse & 0x38) >> 3;
					if(picture_coding_type == 0 || picture_coding_type > 3)  {
						DBG_MSG("illegal picture_coding_type = %d\n", picture_coding_type);
					}
					break;
			}
		}
		else if (parse == 0x000001b5)  {
			extension_parse = 1;
		}
		else if (extension_parse != 0)  {
			--extension_parse;
			switch (extension_parse)  {
				case 0:
					if ((parse & 0xf0) == 0x80)  {
						if (first_sequence == TRUE)  {
							picture_coding_parse = 5;
						}
						picture_count++;
					}
					else if ((parse & 0xf0) == 0x10)  {
						sequence_extension_parse = 1;
					}
					break;
			}
		}
		else if (picture_coding_parse != 0)  {
			--picture_coding_parse;
			switch (picture_coding_parse)  {
				case 4:
					break;
				case 3:
					break;
				case 2:
					break;
				case 1:
					if(pCtx->timecode_mode == TRUE)  {
						if(progressive_sequence == 0)  {
							if(parse & 0x200)  {
								time_code_field += 3;
							}
							else  {
								time_code_field += 2;
							}
						}
						else  {
							temp_flags = ((parse & 0x8000) >> 14) | ((parse & 0x200) >> 9);
							switch(temp_flags & 0x3)  {
								case 3:
									time_code_field += 6;
									break;
								case 2:
									time_code_field += 0;
									break;
								case 1:
									time_code_field += 4;
									break;
								case 0:
									time_code_field += 2;
									break;
								default:
									time_code_field += 0;
									break;
							}
						}
					}
					else  {
						time_code_field += 2;
					}
					if(progressive_sequence == 0)  {
						if(parse & 0x200)  {
							pCtx->video_fields += 3;
							running_average_fields[running_average_frames] = 3;
						}
						else  {
							pCtx->video_fields += 2;
							running_average_fields[running_average_frames] = 2;
						}
					}
					else  {
						temp_flags = ((parse & 0x8000) >> 14) | ((parse & 0x200) >> 9);
						switch(temp_flags & 0x3)  {
							case 3:
								pCtx->video_fields += 3;
								running_average_fields[running_average_frames] = 3;
								break;
							case 2:
								pCtx->video_fields += 0;
								break;
							case 1:
								pCtx->video_fields += 2;
								running_average_fields[running_average_frames] = 2;
								break;
							case 0:
								pCtx->video_fields += 1;
								running_average_fields[running_average_frames] = 1;
								break;
							default:
								pCtx->video_fields += 0;
								break;
						}
					}
					if(first == TRUE)  {
						first = FALSE;
					}
					else  {
						running_average_frames = (running_average_frames + 1) & 1023;
						running_average_count++;
						if(running_average_count == 300)  {
							running_average_count = 299;
							temp_running_average = 0;
							temp_running_fields = 0.0;
							for(j = 0; j < 300; j++)  {
								temp_running_average += running_average_samples[(running_average_start + j) & 1023];
								temp_running_fields += running_average_fields[(running_average_start + j) & 1023];
							}
							running_average_start = (running_average_start + 1) & 1023;
							if(progressive_sequence == 0)  {
								pCtx->running_average_bitrate = (unsigned int)((temp_running_average / 300.0) * ((600.0 / temp_running_fields) * frame_rate));
							}
							else  {
								pCtx->running_average_bitrate = (unsigned int)((temp_running_average / 300.0) * ((300.0 / temp_running_fields) * frame_rate));
							}
						}
					}
					if(first_pts_count != 0)  {
						if(first_pts_count == 2)  {
							first_pts = pts;
						}
						--first_pts_count;
						if(first_pts_count == 0)  {
							if(first_pts > pts)  {
								first_pts = pts;
								pCtx->pts_aligned = first_pts;
							}
							else  {
								pCtx->pts_aligned = first_pts;
							}
							DBG_MSG("First Video PTS = 0x%08x\n", (unsigned int)pCtx->pts_aligned);
						}
					}
					break;
				case 0:
					break;
			}
		}
		else if (sequence_extension_parse != 0)  {
			--sequence_extension_parse;
			if (first_sequence_dump == FALSE)  {
				switch (sequence_extension_parse)  {
					case 0:
						DBG_MSG("Progressive Sequence = %d\n", (parse & 0x8) >> 3);
						progressive_sequence = (parse & 0x8) >> 3;
						pCtx->video_progressive = progressive_sequence;
						first_sequence_dump = TRUE;
						break;
				}
			}
		}
		else if (parse == 0x000001b8)  {
			gop_found = TRUE;
		}
		else if (parse == 0x000001b7)  {
			if(pCtx->parse_only == FALSE)  {
				if (i < 3)  {
					offset = 0 - (3 - i);
					//result = fseek(STRM_ID_VID, offset, SEEK_CUR);
					// TODO ASSERT
					whole_buffer = FALSE;
					middle_es_ptr = es_ptr;
					middle_length = length - (i + 1);
				}
				else  {
					WriteData(pCtx, start_es_ptr, 1, i - 3, STRM_ID_VID);
					whole_buffer = FALSE;
					middle_es_ptr = es_ptr;
					middle_length = length - i - 1;
				}
			}
		}
		picture_size++;
	}
	if(pCtx->parse_only == FALSE)  {
		if(first_sequence == TRUE)  {
			if(whole_buffer == TRUE)  {
				WriteData(pCtx, start_es_ptr, 1, length, STRM_ID_VID);
			}
			else  {
				WriteData(pCtx, middle_es_ptr, 1, middle_length, STRM_ID_VID);
			}
		}
	}
#else
	pCtx->crnt_vid_pts = pts;
	pCtx->crnt_vid_dts = dts;

	WriteData(pCtx, es_ptr, 1, length, STRM_ID_VID);
#endif
}

void parse_h264_video(MpegTsDemuxCtx *pCtx, unsigned char *es_ptr, unsigned int length, unsigned long long pts, unsigned long long dts)
{
#if UNIT_TEST_DEMUX
	unsigned int	i;
	static int		first = TRUE;
	static int		first_sequence = FALSE;
	static int		first_sequence_dump = FALSE;
	static unsigned int	parse = 0;
	static unsigned int	access_unit_delimiter_parse = 0;
	static unsigned int	sequence_parameter_set_parse = 0;
	static unsigned int	picture_count = 0;
	static unsigned char	header[5] = {0x0, 0x0, 0x0, 0x1, 0x9};
	static unsigned long long	first_pts;
	static unsigned int	first_pts_count = 0;
	static unsigned char	constraint_set3_flag;
	unsigned char	primary_pic_type;
	unsigned int	whole_buffer = TRUE;
	unsigned char	*start_es_ptr;
	unsigned char	*middle_es_ptr;
	unsigned int	middle_length = 0x55555555;

	pCtx->crnt_vid_pts = pts;
	pCtx->crnt_vid_dts = dts;

	start_es_ptr = es_ptr;
	for(i = 0; i < length; i++)  {
		parse = (parse << 8) + *es_ptr++;
		if ((parse & 0xffffff00) == 0x00000100)  {
#if 0
			DBG_MSG("parse = 0x%08x\n", parse);
#endif
		}
		if (parse == 0x00000109)  {
			access_unit_delimiter_parse = 1;
			pCtx->coded_frames++;
		}
		else if (access_unit_delimiter_parse != 0)  {
			--access_unit_delimiter_parse;
			primary_pic_type = (unsigned char)(parse & 0xff) >> 5;
#if 0
			DBG_MSG("primary_pic_type = %d\n", primary_pic_type);
#endif
			if (first_sequence == FALSE && primary_pic_type == 0)  {
				DBG_MSG("%d frames before first I-frame\n", picture_count);
				if(pCtx->parse_only == FALSE)  {
					WriteData(pCtx, (unsigned char *)&header, 1, 5, STRM_ID_VID);
					middle_es_ptr = es_ptr - 1;
					middle_length = length - i;
					whole_buffer = FALSE;
				}
				first_sequence = TRUE;
				first_pts_count = 2;
			}
			if(first_pts_count != 0)  {
				if(first_pts_count == 2)  {
					first_pts = pts;
				}
				--first_pts_count;
				if(first_pts_count == 0)  {
					if(first_pts > pts)  {
						first_pts = pts;
						pCtx->pts_aligned = first_pts;
					}
					else  {
						pCtx->pts_aligned = first_pts;
					}
					DBG_MSG("First Video PTS = 0x%08x\n", (unsigned int)pCtx->pts_aligned);
				}
			}
			picture_count++;
		}
		else if (parse == 0x00000127 ||  parse == 0x00000167)  {
			sequence_parameter_set_parse = 3;
		}
		else if (sequence_parameter_set_parse != 0)  {
			--sequence_parameter_set_parse;
			if (first_sequence_dump == FALSE)  {
				switch (sequence_parameter_set_parse)  {
					case 2:
						switch (parse & 0xff)  {
							case 66:
								DBG_MSG("Baseline Profile\n");
								break;
							case 77:
								DBG_MSG("Main Profile\n");
								break;
							case 88:
								DBG_MSG("Extended Profile\n");
								break;
							case 100:
								DBG_MSG("High Profile\n");
								break;
							case 110:
								DBG_MSG("High 10 Profile\n");
								break;
							case 122:
								DBG_MSG("High 4:2:2 Profile\n");
								break;
							case 144:
								DBG_MSG("High 4:4:4 Profile\n");
								break;
							default:
								DBG_MSG("Unknown Profile\n");
								break;
						}
						break;
					case 1:
						constraint_set3_flag = (unsigned char)(parse & 0x10) >> 4;
						break;
					case 0:
						if (((parse & 0xff) == 11) && (constraint_set3_flag == 1))  {
							DBG_MSG("Level = 1b\n");
						}
						else  {
							DBG_MSG("Level = %d.%d\n", (parse & 0xff) / 10, ((parse & 0xff) - (((parse & 0xff) / 10) * 10)));
						}
						first_sequence_dump = TRUE;
						break;
				}
			}
		}
		else if (parse == 0x0000010a ||  parse == 0x0000010b)  {
			if(pCtx->parse_only == FALSE)  {
				middle_es_ptr = es_ptr - 1;
				*middle_es_ptr = 0xc;
			}
		}
	}
	if(pCtx->parse_only == FALSE)  {
		if(first_sequence == TRUE)  {
			if(whole_buffer == TRUE)  {
				WriteData(pCtx, start_es_ptr, 1, length, STRM_ID_VID);
			}
			else  {
				WriteData(pCtx, middle_es_ptr, 1, middle_length, STRM_ID_VID);
			}
		}
	}
#else
	pCtx->crnt_vid_pts = pts;
	pCtx->crnt_vid_dts = dts;

	WriteData(pCtx, es_ptr, 1, length, STRM_ID_VID);
#endif
}


void	demux_mpeg2_transport_init(MpegTsDemuxCtx *pCtx)
{
	int	i;

	pCtx->sync_state = FALSE;
	pCtx->xport_header_parse = 0;
	pCtx->adaptation_field_state = FALSE;
	pCtx->adaptation_field_parse = 0;
	pCtx->adaptation_field_length = 0;
	pCtx->pcr_parse = 0;
	pCtx->pat_section_start = FALSE;
	pCtx->pmt_section_start = FALSE;

	pCtx->video_packet_length_parse = 0;
	pCtx->video_packet_parse = 0;
	pCtx->video_pts_parse = 0;
	pCtx->video_pts_dts_parse = 0;
	pCtx->video_xfer_state = FALSE;
	pCtx->video_packet_number = 0;
	pCtx->video_pes_header_length = 0;

	pCtx->audio_packet_length_parse = 0;
	pCtx->audio_packet_parse = 0;
	pCtx->audio_pts_parse = 0;
	pCtx->audio_pts_dts_parse = 0;
	pCtx->audio_lpcm_parse = 0;
	pCtx->audio_xfer_state = FALSE;
	pCtx->audio_packet_number = 0;
	pCtx->audio_pes_header_length = 0;

	pCtx->pat_pointer_field = 0;
	pCtx->pat_section_length_parse = 0;
	pCtx->pat_section_parse = 0;
	pCtx->pat_xfer_state = FALSE;

	pCtx->pmt_program_descriptor_length_parse = 0;
	pCtx->pmt_program_descriptor_length = 0;

	pCtx->pmt_ES_descriptor_length_parse = 0;
	pCtx->pmt_ES_descriptor_length = 0;

	pCtx->pmt_pointer_field = 0;
	pCtx->pmt_section_length_parse = 0;
	pCtx->pmt_section_parse = 0;
	pCtx->pmt_xfer_state = FALSE;

	pCtx->psip_ptr[0x1ffb] = (psip_t*)malloc(sizeof(struct psip));
	pCtx->psip_ptr[0x1ffb]->psip_section_start = FALSE;
	pCtx->psip_ptr[0x1ffb]->psip_pointer_field = 0;
	pCtx->psip_ptr[0x1ffb]->psip_section_length_parse = 0;
	pCtx->psip_ptr[0x1ffb]->psip_section_parse = 0;
	pCtx->psip_ptr[0x1ffb]->psip_xfer_state = FALSE;

	for (i = 0; i < 0x2000; i++)  {
		pCtx->continuity_counter[i] = 0xff;
		pCtx->pid_counter[i] = 0;
		pCtx->pid_first_packet[i] = 0;
	}

	pCtx->tp_extra_header_parse = 4;
	pCtx->pts_aligned = 0xffffffffffffffffLL;
	pCtx->demux_audio = TRUE;


	pCtx->mgt_last_version_number = 0xff;
	pCtx->vct_last_version_number = 0xff;

	pCtx->ett_pid = 0xffff;
	pCtx->eit0_pid = 0xffff;
	pCtx->eit1_pid = 0xffff;
	pCtx->eit2_pid = 0xffff;
	pCtx->eit3_pid = 0xffff;
	pCtx->ett0_pid = 0xffff;
	pCtx->ett1_pid = 0xffff;
	pCtx->ett2_pid = 0xffff;
	pCtx->ett3_pid = 0xffff;
	memset(pCtx->eit_last_version_number, 0xFF, 4); //= {0xff, 0xff, 0xff, 0xff};

	pCtx->program_map_pid = 0xffff;
	pCtx->transport_stream_id = 0xffff;
	pCtx->video_pes_header_index = 0;
	pCtx->audio_pes_header_index = 0;
	pCtx->first_audio_access_unit = FALSE;

}

void demux_mpeg2_transport_deinit(MpegTsDemuxCtx *pCtx)
{
	int i;
	for(i=0; i < 0x2000; i++) {
		if(pCtx->psip_ptr[i])
			free(pCtx->psip_ptr[i]);
	}
}
void	demux_mpeg2_transport(MpegTsDemuxCtx *pCtx, unsigned int length, unsigned char *buffer)
{
	unsigned int	i, j, k, m, n, q, xfer_length;
	unsigned int	video_channel_count, audio_channel_count;
	unsigned long long	ts_rate, pcr_ext;
	unsigned long long	pcrsave;
	unsigned char	sync, temp;
	unsigned short	temp_program_map_pid;

	unsigned int	tp_extra_header;
	unsigned long long	tp_extra_header_pcr_bytes;


	if (1)  {
		for (i = 0; i < length; i++)  {
			if(pCtx->sync_state == TRUE)  {
				if (pCtx->xport_header_parse != 0)  {
					--pCtx->xport_packet_length;
					pCtx->pcr_bytes++;
					--pCtx->xport_header_parse;
					switch (pCtx->xport_header_parse)  {
						case 2:
							temp = buffer[i];
							pCtx->transport_error_indicator = (temp >> 7) & 0x1;
							pCtx->payload_unit_start_indicator = (temp >> 6) & 0x1;
							pCtx->transport_priority = (temp >> 5) & 0x1;
							pCtx->pid = (temp & 0x1f) << 8;
							break;
						case 1:
							temp = buffer[i];
							pCtx->pid |= temp;
							pCtx->pid_counter[pCtx->pid]++;
							pCtx->packet_counter++;
							if(pCtx->pid_first_packet[pCtx->pid] == 0)  {
								pCtx->pid_first_packet[pCtx->pid] = pCtx->packet_counter;
							}
							pCtx->pid_last_packet[pCtx->pid] = pCtx->packet_counter;
							if(pCtx->dump_pids == TRUE)  {
								DBG_MSG("  PID=%4x", pCtx->pid);
							}
							break;
						case 0:
							temp = buffer[i];
							pCtx->transport_scrambling_control = (temp >> 6) & 0x3;
							pCtx->adaptation_field_control = (temp >> 4) & 0x3;
							if (((pCtx->continuity_counter[pCtx->pid] + 1) & 0xf) != (temp & 0xf))  {
								if (pCtx->adaptation_field_control & 0x1 && pCtx->pid != 0x1fff)  {
									if (pCtx->continuity_counter[pCtx->pid] != 0xff)  {
										pCtx->fDisCont = 1;
										DBG_MSG("Discontinuity!, pCtx->pid = %d <0x%04x>, received = %2d, expected = %2d, at %d\n", pCtx->pid, pCtx->pid, (temp & 0xf), (pCtx->continuity_counter[pCtx->pid] + 1) & 0xf, pCtx->packet_counter);
									}
								}
							}
							if (pCtx->adaptation_field_control & 0x1 && pCtx->pid)  {
								pCtx->continuity_counter[pCtx->pid] = temp & 0xf;
							}
							if ((pCtx->adaptation_field_control & 0x2) == 0x2)  {
								pCtx->adaptation_field_state = TRUE;
							}
							if (pCtx->pid == 0 && pCtx->payload_unit_start_indicator == 1)  {
								pCtx->pat_section_start = TRUE;
							}
							if (pCtx->pid == pCtx->program_map_pid && pCtx->payload_unit_start_indicator == 1)  {
								pCtx->pmt_section_start = TRUE;
							}
							if(pCtx->dump_psip == TRUE)  {
								if ((pCtx->pid == 0x1ffb || pCtx->pid == pCtx->ett_pid || pCtx->pid == pCtx->eit0_pid || pCtx->pid == pCtx->eit1_pid || pCtx->pid == pCtx->eit2_pid || pCtx->pid == pCtx->eit3_pid || pCtx->pid == pCtx->ett0_pid || pCtx->pid == pCtx->ett1_pid || pCtx->pid == pCtx->eit2_pid || pCtx->pid == pCtx->eit3_pid) && pCtx->payload_unit_start_indicator == 1) {
									pCtx->psip_ptr[pCtx->pid]->psip_section_start = TRUE;
								}
							}
							if (pCtx->pid == pCtx->video_pid && pCtx->payload_unit_start_indicator == 1)  {
								pCtx->video_xfer_state = FALSE;
							}
							break;
					}
				}
				else if (pCtx->adaptation_field_state == TRUE)  {
					--pCtx->xport_packet_length;
					pCtx->pcr_bytes++;
					pCtx->adaptation_field_parse = buffer[i];
					pCtx->adaptation_field_length = pCtx->adaptation_field_parse;
					DBG_MSG("Adaptation field length = %d\n", pCtx->adaptation_field_length);
					pCtx->adaptation_field_state = FALSE;
				}
				else if (pCtx->adaptation_field_parse != 0)  {
					--pCtx->xport_packet_length;
					pCtx->pcr_bytes++;
					--pCtx->adaptation_field_parse;
					if ((pCtx->adaptation_field_length - pCtx->adaptation_field_parse) == 1)  {
						if ((buffer[i] & 0x10) == 0x10)  {
							pCtx->pcr_parse = 6;
							pCtx->pcr = 0;
							DBG_MSG("Adaptation field length = %d\n", pCtx->adaptation_field_length);
						}
					}
					else if (pCtx->pcr_parse != 0)  {
						--pCtx->pcr_parse;
						pCtx->pcr = (pCtx->pcr << 8) + buffer[i];
						if (pCtx->pcr_parse == 0 && pCtx->pid == pCtx->pcr_pid)  {
							pcr_ext = pCtx->pcr & 0x1ff;
							if (pCtx->dump_pcr)  {
								DBG_MSG("pCtx->pcr = %d at packet number %d\n", (unsigned int)(pCtx->pcr >> 15), pCtx->packet_counter);
							}
							pCtx->pcr = (pCtx->pcr >> 15) * 300;
							pCtx->pcr = pCtx->pcr + pcr_ext;
							pcrsave = pCtx->pcr;
							if (pCtx->pcr < pCtx->previous_pcr)  {
								pCtx->pcr = pCtx->pcr + (((long long)1) << 42);
							}
							if (pCtx->pcr - pCtx->previous_pcr != 0)  {
								if(pCtx->suppress_tsrate == FALSE)  {
									if (pCtx->hdmv_mode == TRUE)  {
										if (((pCtx->pcr & 0x3fffffff) - tp_extra_header) == 0)  {
											if(pCtx->running_average_bitrate != 0)  {
												DBG_MSG("ts rate = unspecified, video rate = %9d\r", pCtx->running_average_bitrate);
											}
											else  {
												DBG_MSG("ts rate = unspecified\r");
											}
										}
										else  {
											ts_rate = ((((pCtx->pcr_bytes - 2) - tp_extra_header_pcr_bytes) * 27000000) / ((pCtx->pcr & 0x3fffffff) - tp_extra_header));
											if(pCtx->running_average_bitrate != 0)  {
												DBG_MSG("ts rate = %9d, video rate = %9d\r", (unsigned int)ts_rate * 8, pCtx->running_average_bitrate);
											}
											else  {
												DBG_MSG("ts rate = %9d\r", (unsigned int)ts_rate * 8);
											}
										}
									}
									else  {
										ts_rate = ((pCtx->pcr_bytes * 27000000) / (pCtx->pcr - pCtx->previous_pcr));
										if(pCtx->running_average_bitrate != 0)  {
											DBG_MSG("ts rate = %9d, video rate = %9d\r", (unsigned int)ts_rate * 8, pCtx->running_average_bitrate);
										}
										else  {
											DBG_MSG("ts rate = %9d\r", (unsigned int)ts_rate * 8);
										}
									}
								}
							}

							if(pCtx) {
								pCtx->current_pcr = pCtx->pcr;
								pCtx->pcr_arrival_time = ClockGetInternalTime(pCtx->pClk);
								pCtx->current_tsrate = ts_rate;
								if(pCtx->fClkSrc) {
									if(ClockGetState(pCtx->pClk) == CLOCK_STOPPED){
										DBG_MSG("Satrting clock\n");
										ClockStart(pCtx->pClk, pCtx->pcr / 27);
									} else {
										ClockAdjust(pCtx->pClk, pCtx->pcr_arrival_time, pCtx->pcr / 27);
									}
								}
							}
							pCtx->previous_pcr = pcrsave;
							pCtx->pcr_bytes = 0;
						}
					}
					if (pCtx->xport_packet_length == 0)  {
						pCtx->sync_state = FALSE;
					}
				}
				else if (pCtx->pid == 0 && pCtx->detect_program_pids)  {
					if (pCtx->pat_xfer_state == TRUE)  {
						if ((length - i) >= pCtx->pat_section_length)  {
							j = pCtx->pat_section_length;
							if (pCtx->xport_packet_length <= j)  {
								j = pCtx->xport_packet_length;
							}
						}
						else  {
							j = length - i;
							if (pCtx->xport_packet_length <= j)  {
								j = pCtx->xport_packet_length;
							}
						}
						DBG_MSG("Burst length = %d\r\n", j);
						for (k = 0; k < j; k++)  {
							pCtx->program_association_table[pCtx->pat_offset] = buffer[i];
							DBG_MSG("PAT byte %d = 0x%02x\n", pCtx->pat_offset, buffer[i]);
							pCtx->pat_offset++;
							i++;
							--pCtx->pat_section_length;
							--pCtx->xport_packet_length;
							pCtx->pcr_bytes++;
						}
						--i;	/* adjust because of for loop */
						if (pCtx->pat_section_length == 0)  {
							DBG_MSG("End of PSI section = %d\r\n",i);
							pCtx->pat_xfer_state = FALSE;
							if (pCtx->pat_section_number == pCtx->pat_last_section_number)  {
								for (k = 0; k < (pCtx->pat_offset - 4); k+=4)  {
									pCtx->program_number = pCtx->program_association_table[k] << 8;
									pCtx->program_number |= pCtx->program_association_table[k + 1];
									if (pCtx->first_pat == TRUE)  {
										temp_program_map_pid = (pCtx->program_association_table[k + 2] & 0x1f) << 8;
										temp_program_map_pid |= pCtx->program_association_table[k + 3];
										DBG_MSG("Program Number = %d (0x%04x), Program Map PID = %d (0x%04x)\n", pCtx->program_number, pCtx->program_number, temp_program_map_pid, temp_program_map_pid);
									}
									if (pCtx->program_number == pCtx->program)  {
										pCtx->program_map_pid = (pCtx->program_association_table[k + 2] & 0x1f) << 8;
										pCtx->program_map_pid |= pCtx->program_association_table[k + 3];
										DBG_MSG("Program Map PID = %d\r\n",pCtx->program_map_pid);
									}
								}
								pCtx->first_pat = FALSE;
							}
						}
					}
					else  {
						--pCtx->xport_packet_length;
						pCtx->pcr_bytes++;
						if (pCtx->pat_section_start == TRUE)  {
							pCtx->pat_pointer_field = buffer[i];
							if (pCtx->pat_pointer_field == 0)  {
								pCtx->pat_section_length_parse = 3;
							}
							pCtx->pat_section_start = FALSE;
						}
						else if (pCtx->pat_pointer_field != 0)  {
							--pCtx->pat_pointer_field;
							switch (pCtx->pat_pointer_field)  {
								case 0:
									pCtx->pat_section_length_parse = 3;
									break;
							}
						}
						else if (pCtx->pat_section_length_parse != 0)  {
							--pCtx->pat_section_length_parse;
							switch (pCtx->pat_section_length_parse)  {
								case 2:
									break;
								case 1:
									pCtx->pat_section_length = (buffer[i] & 0xf) << 8;
									break;
								case 0:
									pCtx->pat_section_length |= buffer[i];
									DBG_MSG("Section length = %d\r\n", pCtx->pat_section_length);
									if (pCtx->pat_section_length > 1021)  {
										DBG_MSG("PAT Section length = %d\r\n", pCtx->pat_section_length);
										pCtx->pat_section_length = 0;
									}
									else  {
										pCtx->pat_section_parse = 5;
									}
									break;
							}
						}
						else if (pCtx->pat_section_parse != 0)  {
							--pCtx->pat_section_length;
							--pCtx->pat_section_parse;
							switch (pCtx->pat_section_parse)  {
								case 4:
									pCtx->transport_stream_id = buffer[i] << 8;
									break;
								case 3:
									pCtx->transport_stream_id |= buffer[i];
									break;
								case 2:
									break;
								case 1:
									pCtx->pat_section_number = buffer[i];
									if (pCtx->pat_section_number == 0)  {
										pCtx->pat_offset = 0;
									}
									break;
								case 0:
									pCtx->pat_last_section_number = buffer[i];
									pCtx->pat_xfer_state = TRUE;
									break;
							}
						}
					}
					if (pCtx->xport_packet_length == 0)  {
						pCtx->sync_state = FALSE;
					}
				}
				else if (pCtx->pid == pCtx->program_map_pid && pCtx->detect_program_pids)  {
					if (pCtx->pmt_xfer_state == TRUE)  {
						if ((length - i) >= pCtx->pmt_section_length)  {
							j = pCtx->pmt_section_length;
							if (pCtx->xport_packet_length <= j)  {
								j = pCtx->xport_packet_length;
							}
						}
						else  {
							j = length - i;
							if (pCtx->xport_packet_length <= j)  {
								j = pCtx->xport_packet_length;
							}
						}
						DBG_MSG("Burst length = %d\r\n", j);
						for (k = 0; k < j; k++)  {
							pCtx->program_map_table[pCtx->pmt_offset] = buffer[i];
							DBG_MSG("PMT byte %d = 0x%02x\n", pCtx->pmt_offset, buffer[i]);
							pCtx->pmt_offset++;
							i++;
							--pCtx->pmt_section_length;
							--pCtx->xport_packet_length;
							pCtx->pcr_bytes++;
						}
						--i;	/* adjust because of for loop */
						if (pCtx->pmt_section_length == 0)  {
							DBG_MSG("End of PSI section = %d\r\n",i);
							pCtx->pmt_xfer_state = FALSE;
							if (pCtx->pmt_section_number == pCtx->pmt_last_section_number)  {
								video_channel_count = 0;
								audio_channel_count = 0;
								for (k = 0; k < (pCtx->pmt_offset - 4); k+=5)  {
									pCtx->pmt_stream_type = pCtx->program_map_table[k];
									pCtx->pmt_elementary_pid = (pCtx->program_map_table[k+1] & 0x1f) << 8;
									pCtx->pmt_elementary_pid |= pCtx->program_map_table[k+2];
									if (pCtx->pmt_stream_type == 0x1 || pCtx->pmt_stream_type == 0x2 || (pCtx->pmt_stream_type == 0x80 && pCtx->hdmv_mode == FALSE) || pCtx->pmt_stream_type == 0x1b || pCtx->pmt_stream_type == 0xea)  {
										video_channel_count++;
										if (video_channel_count == pCtx->video_channel)  {
											pCtx->video_pid = pCtx->pmt_elementary_pid;
											if (pCtx->first_pmt == TRUE)  {
												DBG_MSG("Video PID = %4d <0x%04x>, type = 0x%02x\r\n", pCtx->video_pid, pCtx->video_pid, pCtx->pmt_stream_type);
											}
											pCtx->video_stream_type = pCtx->pmt_stream_type;
										}
									}
									else if (pCtx->pmt_stream_type == 0x3 || pCtx->pmt_stream_type == 0x4 || pCtx->pmt_stream_type == 0x80 || pCtx->pmt_stream_type == 0x81 || pCtx->pmt_stream_type == 0x6 || pCtx->pmt_stream_type == 0x82 || pCtx->pmt_stream_type == 0x83 || pCtx->pmt_stream_type == 0x84 || pCtx->pmt_stream_type == 0x85 || pCtx->pmt_stream_type == 0x86 || pCtx->pmt_stream_type == 0xa1 || pCtx->pmt_stream_type == 0xa2 || pCtx->pmt_stream_type == 0x11 ||  pCtx->pmt_stream_type == 0x0F)  {
										audio_channel_count++;
										if (audio_channel_count == pCtx->audio_channel)  {
											pCtx->audio_pid = pCtx->pmt_elementary_pid;
											if (pCtx->first_pmt == TRUE)  {
												DBG_MSG("Audio PID = %4d <0x%04x>, type = 0x%02x\r\n", pCtx->audio_pid, pCtx->audio_pid, pCtx->pmt_stream_type);
											}
											pCtx->audio_stream_type = pCtx->pmt_stream_type;
										}
									}
									pCtx->pmt_ES_info_length = (pCtx->program_map_table[k+3] & 0xf) << 8;
									pCtx->pmt_ES_info_length |= pCtx->program_map_table[k+4];
									if (pCtx->pmt_ES_info_length != 0)  {
										pCtx->pmt_ES_descriptor_length_parse = 2;
										for (q = 0; q < pCtx->pmt_ES_info_length; q++)  {
											if (pCtx->pmt_ES_descriptor_length_parse != 0)  {
												--pCtx->pmt_ES_descriptor_length_parse;
												switch (pCtx->pmt_ES_descriptor_length_parse)  {
													case 1:
														if (pCtx->first_pmt == TRUE)  {
															DBG_MSG("ES descriptor for stream type 0x%02x = 0x%02x", pCtx->pmt_stream_type, pCtx->program_map_table[k+5+q]);
														}
														break;
													case 0:
														pCtx->pmt_ES_descriptor_length = pCtx->program_map_table[k+5+q];
														if (pCtx->first_pmt == TRUE)  {
															DBG_MSG(", 0x%02x", pCtx->program_map_table[k+5+q]);
															if (pCtx->pmt_ES_descriptor_length == 0)  {
																DBG_MSG("\n");
															}
														}
														break;
												}
											}
											else if (pCtx->pmt_ES_descriptor_length != 0)  {
												--pCtx->pmt_ES_descriptor_length;
												if (pCtx->first_pmt == TRUE)  {
													DBG_MSG(", 0x%02x", pCtx->program_map_table[k+5+q]);
												}
												if (pCtx->pmt_ES_descriptor_length == 0)  {
													if (pCtx->first_pmt == TRUE)  {
														DBG_MSG("\n");
													}
													if (q < pCtx->pmt_ES_info_length)  {
														pCtx->pmt_ES_descriptor_length_parse = 2;
													}
												}
											}
										}
									}
									k += pCtx->pmt_ES_info_length;
								}
								pCtx->first_pmt = FALSE;
							}
						}
					}
					else  {
						--pCtx->xport_packet_length;
						pCtx->pcr_bytes++;
						if (pCtx->pmt_section_start == TRUE)  {
							pCtx->pmt_pointer_field = buffer[i];
							if (pCtx->pmt_pointer_field == 0)  {
								pCtx->pmt_section_length_parse = 3;
							}
							pCtx->pmt_section_start = FALSE;
						}
						else if (pCtx->pmt_pointer_field != 0)  {
							--pCtx->pmt_pointer_field;
							switch (pCtx->pmt_pointer_field)  {
								case 0:
									pCtx->pmt_section_length_parse = 3;
									break;
							}
						}
						else if (pCtx->pmt_section_length_parse != 0)  {
							--pCtx->pmt_section_length_parse;
							switch (pCtx->pmt_section_length_parse)  {
								case 2:
									if (buffer[i] != 0x2)  {
										pCtx->pmt_section_length_parse = 0;
									}
									break;
								case 1:
									pCtx->pmt_section_length = (buffer[i] & 0xf) << 8;
									break;
								case 0:
									pCtx->pmt_section_length |= buffer[i];
									DBG_MSG("Section length = %d\r\n", pCtx->pmt_section_length);
									if (pCtx->pmt_section_length > 1021)  {
										DBG_MSG("PMT Section length = %d\r\n", pCtx->pmt_section_length);
										pCtx->pmt_section_length = 0;
									}
									else  {
										pCtx->pmt_section_parse = 9;
									}
									break;
							}
						}
						else if (pCtx->pmt_section_parse != 0)  {
							--pCtx->pmt_section_length;
							--pCtx->pmt_section_parse;
							switch (pCtx->pmt_section_parse)  {
								case 8:
									break;
								case 7:
									break;
								case 6:
									break;
								case 5:
									pCtx->pmt_section_number = buffer[i];
									if (pCtx->pmt_section_number == 0)  {
										pCtx->pmt_offset = 0;
									}
									break;
								case 4:
									pCtx->pmt_last_section_number = buffer[i];
									break;
								case 3:
									pCtx->pcr_pid = (buffer[i] & 0x1f) << 8;
									break;
								case 2:
									pCtx->pcr_pid |= buffer[i];
									DBG_MSG("PCR PID = %d\r\n", pcr_pid);
									break;
								case 1:
									pCtx->pmt_program_info_length = (buffer[i] & 0xf) << 8;
									break;
								case 0:
									pCtx->pmt_program_info_length |= buffer[i];
									if (pCtx->pmt_program_info_length == 0)  {
										pCtx->pmt_xfer_state = TRUE;
									}
									else  {
										pCtx->pmt_program_descriptor_length_parse = 2;
									}
									break;
							}
						}
						else if (pCtx->pmt_program_info_length != 0)  {
							--pCtx->pmt_section_length;
							--pCtx->pmt_program_info_length;
							if (pCtx->pmt_program_descriptor_length_parse != 0)  {
								--pCtx->pmt_program_descriptor_length_parse;
								switch (pCtx->pmt_program_descriptor_length_parse)  {
									case 1:
										if (pCtx->first_pmt == TRUE)  {
											DBG_MSG("program descriptor = 0x%02x", buffer[i]);
										}
										break;
									case 0:
										pCtx->pmt_program_descriptor_length = buffer[i];
										if (pCtx->first_pmt == TRUE)  {
											DBG_MSG(", 0x%02x", buffer[i]);
											if (pCtx->pmt_program_descriptor_length == 0)  {
												DBG_MSG("\n");
											}
										}
										break;
								}
							}
							else if (pCtx->pmt_program_descriptor_length != 0)  {
								--pCtx->pmt_program_descriptor_length;
								if (pCtx->first_pmt == TRUE)  {
									DBG_MSG(", 0x%02x", buffer[i]);
								}
								if (pCtx->pmt_program_descriptor_length == 0)  {
									if (pCtx->first_pmt == TRUE)  {
										DBG_MSG("\n");
									}
									if (pCtx->pmt_program_info_length != 0)  {
										pCtx->pmt_program_descriptor_length_parse = 2;
									}
								}
							}
							switch (pCtx->pmt_program_info_length)  {
								case 0:
									pCtx->pmt_xfer_state = TRUE;
									break;
							}
						}
					}
					if (pCtx->xport_packet_length == 0)  {
						pCtx->sync_state = FALSE;
					}
				}
				else if (pCtx->pid == pCtx->video_pid && pCtx->transport_scrambling_control == 0) {
					pCtx->video_parse = (pCtx->video_parse << 8) + buffer[i];
					if (pCtx->video_xfer_state == TRUE)  {
						if ((length - i) >= pCtx->video_packet_length)  {
							j = pCtx->video_packet_length;
							if (pCtx->xport_packet_length <= j)  {
								j = pCtx->xport_packet_length;
							}
						}
						else  {
							j = length - i;
							if (pCtx->xport_packet_length <= j)  {
								j = pCtx->xport_packet_length;
							}
						}
						xfer_length = j;
						DBG_MSG("Burst length = %d\r\n", j);
						if(pCtx->video_stream_type == 0x1 || pCtx->video_stream_type == 0x2 || pCtx->video_stream_type == 0x80)  {
							parse_mpeg2_video(pCtx, &buffer[i], xfer_length, pCtx->video_pts, pCtx->video_dts);
						}
						else if(pCtx->video_stream_type == 0x1b)  {
							parse_h264_video(pCtx, &buffer[i], xfer_length, pCtx->video_pts, pCtx->video_dts);
						}
						else  {
							if(pCtx->parse_only == FALSE)  {
								WriteData(pCtx, &buffer[i], 1, xfer_length, STRM_ID_VID);
							}
						}
						i = i + xfer_length;
						pCtx->video_packet_length = pCtx->video_packet_length - xfer_length;
						pCtx->xport_packet_length = pCtx->xport_packet_length - xfer_length;
						pCtx->pcr_bytes = pCtx->pcr_bytes + xfer_length;
						--i;	/* adjust because of for loop */
						if (pCtx->video_packet_length == 0)  {
							DBG_MSG("End of Packet = %d\r\n",i);
							pCtx->video_xfer_state = FALSE;
						}
					}
					else  {
						--pCtx->xport_packet_length;
						pCtx->pcr_bytes++;
						if ((pCtx->video_parse >= 0x000001e0 && pCtx->video_parse <= 0x000001ef) || pCtx->video_parse == 0x000001fd)  {
							DBG_MSG("PES start code, Video Stream number = %d.\r\n", (pCtx->video_parse & 0x0f));
							pCtx->video_packet_length_parse = 2;
							pCtx->video_packet_number++;
							pCtx->video_pes_header_index = 0;
							pCtx->video_pes_header[pCtx->video_pes_header_index++] = (pCtx->video_parse >> 24) & 0xff;
							pCtx->video_pes_header[pCtx->video_pes_header_index++] = (pCtx->video_parse >> 16) & 0xff;
							pCtx->video_pes_header[pCtx->video_pes_header_index++] = (pCtx->video_parse >> 8) & 0xff;
							pCtx->video_pes_header[pCtx->video_pes_header_index++] = pCtx->video_parse & 0xff;
						}
						else if (pCtx->video_packet_length_parse == 2)  {
							--pCtx->video_packet_length_parse;
							pCtx->video_pes_header[pCtx->video_pes_header_index++] = pCtx->video_parse & 0xff;
						}
						else if (pCtx->video_packet_length_parse == 1)  {
							--pCtx->video_packet_length_parse;
							pCtx->video_packet_length = pCtx->video_parse & 0xffff;
							if (pCtx->video_packet_length == 0)  {
								pCtx->video_packet_length = 0xffffffff;
							}
							DBG_MSG("Packet length = %d\r\n", pCtx->video_packet_length);
							pCtx->video_packet_parse = 3;
							pCtx->video_pes_header[pCtx->video_pes_header_index++] = pCtx->video_parse & 0xff;
						}
						else if (pCtx->video_packet_parse != 0)  {
							--pCtx->video_packet_length;
							--pCtx->video_packet_parse;
							switch (pCtx->video_packet_parse)  {
								case 2:
									pCtx->video_pes_header[pCtx->video_pes_header_index++] = pCtx->video_parse & 0xff;
									break;
								case 1:
									pCtx->video_pes_header_flags = pCtx->video_parse & 0xff;
									pCtx->video_pes_header[pCtx->video_pes_header_index++] = pCtx->video_parse & 0xff;
									break;
								case 0:
									pCtx->video_pes_header_length = pCtx->video_parse & 0xff;
									pCtx->video_pes_header[pCtx->video_pes_header_index++] = pCtx->video_parse & 0xff;
									DBG_MSG("Video PES header length = %d\r\n", pCtx->video_pes_header_length);
									if ((pCtx->video_pes_header_flags & 0xc0) == 0x80)  {
										pCtx->video_pts_parse = 5;
									}
									else if ((pCtx->video_pes_header_flags & 0xc0) == 0xc0)  {
										pCtx->video_pts_dts_parse = 10;
									}
									if (pCtx->video_pes_header_length == 0)  {
										pCtx->video_xfer_state = TRUE;
										if(pCtx->parse_only == FALSE && pCtx->pes_streams == TRUE)  {
											WriteData(pCtx, &pCtx->video_pes_header[0], 1, pCtx->video_pes_header_index, STRM_ID_VID);
										}
									}
									break;
							}
						}
						else if (pCtx->video_pts_parse != 0)  {
							--pCtx->video_packet_length;
							--pCtx->video_pes_header_length;
							--pCtx->video_pts_parse;
							switch (pCtx->video_pts_parse)  {
								case 4:
									pCtx->video_temp_pts = 0;
									pCtx->video_temp_pts = pCtx->video_temp_pts | ((unsigned long long)(pCtx->video_parse & 0xe) << 29);
									pCtx->video_pes_header[pCtx->video_pes_header_index++] = pCtx->video_parse & 0xff;
									break;
								case 3:
									pCtx->video_temp_pts = pCtx->video_temp_pts | ((unsigned long long)(pCtx->video_parse & 0xff) << 22);
									pCtx->video_pes_header[pCtx->video_pes_header_index++] = pCtx->video_parse & 0xff;
									break;
								case 2:
									pCtx->video_temp_pts = pCtx->video_temp_pts | ((unsigned long long)(pCtx->video_parse & 0xfe) << 14);
									pCtx->video_pes_header[pCtx->video_pes_header_index++] = pCtx->video_parse & 0xff;
									break;
								case 1:
									pCtx->video_temp_pts = pCtx->video_temp_pts | ((unsigned long long)(pCtx->video_parse & 0xff) << 7);
									pCtx->video_pes_header[pCtx->video_pes_header_index++] = pCtx->video_parse & 0xff;
									break;
								case 0:
									pCtx->video_temp_pts = pCtx->video_temp_pts | ((unsigned long long)(pCtx->video_parse & 0xfe) >> 1);
									pCtx->video_pes_header[pCtx->video_pes_header_index++] = pCtx->video_parse & 0xff;
									pCtx->video_pts = pCtx->video_temp_pts;
									if (pCtx->video_pts > pCtx->last_video_pts)  {
										pCtx->last_video_pts = pCtx->video_pts;
									}
									pCtx->video_dts = 0;
									if(pCtx->dump_video_pts == TRUE)  {
										if (pCtx->video_pts_count == 0)
										{
											DBG_MSG("Video PTS(B) = %d\n", (unsigned int)pCtx->video_pts);
										}
										else
										{
											DBG_MSG("Video PTS(B) = %d, %d\n", (unsigned int)pCtx->video_pts, (unsigned int)(pCtx->video_pts - pCtx->prev_video_dts));
										}
									}
									pCtx->last_video_pts_diff = pCtx->video_pts - pCtx->prev_video_dts;
									pCtx->prev_video_dts = pCtx->video_pts;
									pCtx->video_pts_count++;
									if (pCtx->video_pes_header_length == 0)  {
										pCtx->video_xfer_state = TRUE;
										if(pCtx->parse_only == FALSE && pCtx->pes_streams == TRUE)  {
											WriteData(pCtx, &pCtx->video_pes_header[0], 1, pCtx->video_pes_header_index, STRM_ID_VID);
										}
									}
									break;
							}
						}
						else if (pCtx->video_pts_dts_parse != 0)  {
							--pCtx->video_packet_length;
							--pCtx->video_pes_header_length;
							--pCtx->video_pts_dts_parse;
							switch (pCtx->video_pts_dts_parse)  {
								case 9:
									pCtx->video_temp_pts = 0;
									pCtx->video_temp_pts = pCtx->video_temp_pts | ((unsigned long long)(pCtx->video_parse & 0xe) << 29);
									pCtx->video_pes_header[pCtx->video_pes_header_index++] = pCtx->video_parse & 0xff;
									break;
								case 8:
									pCtx->video_temp_pts = pCtx->video_temp_pts | ((unsigned long long)(pCtx->video_parse & 0xff) << 22);
									pCtx->video_pes_header[pCtx->video_pes_header_index++] = pCtx->video_parse & 0xff;
									break;
								case 7:
									pCtx->video_temp_pts = pCtx->video_temp_pts | ((unsigned long long)(pCtx->video_parse & 0xfe) << 14);
									pCtx->video_pes_header[pCtx->video_pes_header_index++] = pCtx->video_parse & 0xff;
									break;
								case 6:
									pCtx->video_temp_pts = pCtx->video_temp_pts | ((unsigned long long)(pCtx->video_parse & 0xff) << 7);
									pCtx->video_pes_header[pCtx->video_pes_header_index++] = pCtx->video_parse & 0xff;
									break;
								case 5:
									pCtx->video_temp_pts = pCtx->video_temp_pts | ((unsigned long long)(pCtx->video_parse & 0xfe) >> 1);
									pCtx->video_pes_header[pCtx->video_pes_header_index++] = pCtx->video_parse & 0xff;
									pCtx->video_pts = pCtx->video_temp_pts;
									if (pCtx->video_pts > pCtx->last_video_pts)  {
										pCtx->last_video_pts = pCtx->video_pts;
									}
									pCtx->video_dts = 1;
									break;
								case 4:
									pCtx->video_temp_dts = 0;
									pCtx->video_temp_dts = pCtx->video_temp_dts | ((unsigned long long)(pCtx->video_parse & 0xe) << 29);
									pCtx->video_pes_header[pCtx->video_pes_header_index++] = pCtx->video_parse & 0xff;
									break;
								case 3:
									pCtx->video_temp_dts = pCtx->video_temp_dts | ((unsigned long long)(pCtx->video_parse & 0xff) << 22);
									pCtx->video_pes_header[pCtx->video_pes_header_index++] = pCtx->video_parse & 0xff;
									break;
								case 2:
									pCtx->video_temp_dts = pCtx->video_temp_dts | ((unsigned long long)(pCtx->video_parse & 0xfe) << 14);
									pCtx->video_pes_header[pCtx->video_pes_header_index++] = pCtx->video_parse & 0xff;
									break;
								case 1:
									pCtx->video_temp_dts = pCtx->video_temp_dts | ((unsigned long long)(pCtx->video_parse & 0xff) << 7);
									pCtx->video_pes_header[pCtx->video_pes_header_index++] = pCtx->video_parse & 0xff;
									break;
								case 0:
									pCtx->video_temp_dts = pCtx->video_temp_dts | ((unsigned long long)(pCtx->video_parse & 0xfe) >> 1);
									pCtx->video_pes_header[pCtx->video_pes_header_index++] = pCtx->video_parse & 0xff;
									if(pCtx->dump_video_pts == TRUE)  {
										if (pCtx->video_pts_count == 0)  {
											DBG_MSG("Video PTS(P) = %d, DTS(P) = %d\n", (unsigned int)pCtx->video_pts, (unsigned int)pCtx->video_temp_dts);
										}
										else  {
											DBG_MSG("Video PTS(P) = %d, DTS(P) = %d, %d\n", (unsigned int)pCtx->video_pts, (unsigned int)pCtx->video_temp_dts, (unsigned int)(pCtx->video_temp_dts - pCtx->prev_video_dts));
										}
									}
									pCtx->last_video_pts_diff = pCtx->video_temp_dts - pCtx->prev_video_dts;
									pCtx->prev_video_dts = pCtx->video_temp_dts;
									pCtx->video_pts_count++;
									if (pCtx->video_pes_header_length == 0)  {
										pCtx->video_xfer_state = TRUE;
										if(pCtx->parse_only == FALSE && pCtx->pes_streams == TRUE)  {
											WriteData(pCtx, &pCtx->video_pes_header[0], 1, pCtx->video_pes_header_index, STRM_ID_VID);
										}
									}
									break;
							}
						}
						else if (pCtx->video_pes_header_length != 0)  {
							--pCtx->video_packet_length;
							--pCtx->video_pes_header_length;
							pCtx->video_pes_header[pCtx->video_pes_header_index++] = pCtx->video_parse & 0xff;
							switch (pCtx->video_pes_header_length)  {
								case 0:
									pCtx->video_xfer_state = TRUE;
									if(pCtx->parse_only == FALSE && pCtx->pes_streams == TRUE)  {
										WriteData(pCtx, &pCtx->video_pes_header[0], 1, pCtx->video_pes_header_index, STRM_ID_VID);
									}
									break;
							}
						}
					}
					if (pCtx->xport_packet_length == 0)  {
						pCtx->sync_state = FALSE;
					}
				}
				else if (pCtx->pid == pCtx->audio_pid && pCtx->transport_scrambling_control == 0) {
					pCtx->audio_parse = (pCtx->audio_parse << 8) + buffer[i];
					if (pCtx->audio_xfer_state == TRUE)  {
						if ((length - i) >= pCtx->audio_packet_length)  {
							j = pCtx->audio_packet_length;
							if (pCtx->xport_packet_length <= j)  {
								j = pCtx->xport_packet_length;
							}
						}
						else  {
							j = length - i;
							if (pCtx->xport_packet_length <= j)  {
								j = pCtx->xport_packet_length;
							}
						}
						xfer_length = j;
						DBG_MSG("Burst length = %d\r\n", j);
						if (pCtx->demux_audio == TRUE)  {
							int end_of_frame = (pCtx->audio_packet_length == xfer_length);
							if(pCtx->audio_stream_type == 0x81 || pCtx->audio_stream_type == 0x6)  {
								parse_ac3_audio(pCtx, &buffer[i], xfer_length, pCtx->audio_pts, pCtx->first_audio_access_unit, end_of_frame);
							}
							else if(pCtx->audio_stream_type == 0x3 || pCtx->audio_stream_type == 0x4)  {
								parse_mp2_audio(&buffer[i], xfer_length, pCtx->audio_pts, pCtx->first_audio_access_unit);
							}
							else if(pCtx->audio_stream_type == 0x80)  {
								parse_lpcm_audio(pCtx, &buffer[i], xfer_length, pCtx->audio_pts, pCtx->first_audio_access_unit, pCtx->audio_lpcm_header_flags);
							}
							else if(pCtx->audio_stream_type == 0x0f)  {
								parse_aac_audio(pCtx, &buffer[i], xfer_length, pCtx->audio_pts, pCtx->first_audio_access_unit, end_of_frame);
							}
							else  {
								if(pCtx->parse_only == FALSE)  {
									WriteData(pCtx, &buffer[i], 1, xfer_length, STRM_ID_AUD);
								}
							}

							pCtx->first_audio_access_unit = FALSE;
							i = i + xfer_length;
							pCtx->audio_packet_length = pCtx->audio_packet_length - xfer_length;
							pCtx->xport_packet_length = pCtx->xport_packet_length - xfer_length;
							pCtx->pcr_bytes = pCtx->pcr_bytes + xfer_length;
							--i;	/* adjust because of for loop */
							if (pCtx->audio_packet_length == 0)  {
								DBG_MSG("End of Packet = %d\r\n",i);
								pCtx->audio_xfer_state = FALSE;
							}
						}
						else  {
							--pCtx->xport_packet_length;
							pCtx->pcr_bytes++;
							if ((length - i) >= pCtx->xport_packet_length)  {
								i = i + pCtx->xport_packet_length;
								pCtx->pcr_bytes = pCtx->pcr_bytes + pCtx->xport_packet_length;
								pCtx->xport_packet_length = 0;
							}
							else  {
								pCtx->xport_packet_length = pCtx->xport_packet_length - (length - i) + 1;
								pCtx->pcr_bytes = pCtx->pcr_bytes + (length - i) - 1;
								i = length;
							}
							pCtx->audio_packet_length = pCtx->audio_packet_length - j;
							if (pCtx->audio_packet_length == 0)  {
								pCtx->audio_xfer_state = FALSE;
							}
						}
					}
					else  {
						--pCtx->xport_packet_length;
						pCtx->pcr_bytes++;
						if (((pCtx->audio_parse >= 0x000001c0 && pCtx->audio_parse <= 0x000001df) && (pCtx->audio_stream_type == 0x3 || pCtx->audio_stream_type == 0x4 || pCtx->audio_stream_type == 0x6 || pCtx->audio_stream_type == 0x0f)) || pCtx->audio_parse == 0x000001bd || pCtx->audio_parse == 0x000001fd || pCtx->audio_parse == 0x000001fa)  {
							DBG_MSG("PES start code = 0x%08x, stream_type = 0x%02x\r\n", pCtx->audio_parse, audio_stream_type);
							pCtx->audio_packet_length_parse = 2;
							pCtx->audio_packet_number++;
							pCtx->audio_pes_header_index = 0;
							pCtx->audio_pes_header[pCtx->audio_pes_header_index++] = (pCtx->audio_parse >> 24) & 0xff;
							pCtx->audio_pes_header[pCtx->audio_pes_header_index++] = (pCtx->audio_parse >> 16) & 0xff;
							pCtx->audio_pes_header[pCtx->audio_pes_header_index++] = (pCtx->audio_parse >> 8) & 0xff;
							pCtx->audio_pes_header[pCtx->audio_pes_header_index++] = pCtx->audio_parse & 0xff;
						}
						else if (pCtx->audio_packet_length_parse == 2)  {
							--pCtx->audio_packet_length_parse;
							pCtx->audio_pes_header[pCtx->audio_pes_header_index++] = pCtx->audio_parse & 0xff;
						}
						else if (pCtx->audio_packet_length_parse == 1)  {
							--pCtx->audio_packet_length_parse;
							pCtx->audio_packet_length = pCtx->audio_parse & 0xffff;
							DBG_MSG("Packet length = %d\r\n", pCtx->audio_packet_length);
							pCtx->audio_packet_parse = 3;
							pCtx->audio_pes_header[pCtx->audio_pes_header_index++] = pCtx->audio_parse & 0xff;
						}
						else if (pCtx->audio_packet_parse != 0)  {
							--pCtx->audio_packet_length;
							--pCtx->audio_packet_parse;
							switch (pCtx->audio_packet_parse)  {
								case 2:
									pCtx->audio_pes_header[pCtx->audio_pes_header_index++] = pCtx->audio_parse & 0xff;
									break;
								case 1:
									pCtx->audio_pes_header_flags = pCtx->audio_parse & 0xff;
									pCtx->audio_pes_header[pCtx->audio_pes_header_index++] = pCtx->audio_parse & 0xff;
									break;
								case 0:
									pCtx->audio_pes_header_length = pCtx->audio_parse & 0xff;
									pCtx->audio_pes_header[pCtx->audio_pes_header_index++] = pCtx->audio_parse & 0xff;
									DBG_MSG("Audio PES header length = %d\r\n", pCtx->audio_pes_header_length);
									if ((pCtx->audio_pes_header_flags & 0xc0) == 0x80)  {
										pCtx->audio_pts_parse = 5;
									}
									else if ((pCtx->audio_pes_header_flags & 0xc0) == 0xc0)  {
										pCtx->audio_pts_dts_parse = 10;
									}
									if (pCtx->audio_pes_header_length == 0)  {
										pCtx->audio_xfer_state = TRUE;
										if(pCtx->parse_only == FALSE && pCtx->pes_streams == TRUE)  {
											WriteData(pCtx, &pCtx->audio_pes_header[0], 1, pCtx->audio_pes_header_index, STRM_ID_AUD);
										}
									}
									break;
							}
						}
						else if (pCtx->audio_pts_parse != 0)  {
							--pCtx->audio_packet_length;
							--pCtx->audio_pes_header_length;
							--pCtx->audio_pts_parse;
							switch (pCtx->audio_pts_parse)  {
								case 4:
									pCtx->audio_temp_pts = 0;
									pCtx->audio_temp_pts = pCtx->audio_temp_pts | ((unsigned long long)(pCtx->audio_parse & 0xe) << 29);
									pCtx->audio_pes_header[pCtx->audio_pes_header_index++] = pCtx->audio_parse & 0xff;
									break;
								case 3:
									pCtx->audio_temp_pts = pCtx->audio_temp_pts | ((unsigned long long)(pCtx->audio_parse & 0xff) << 22);
									pCtx->audio_pes_header[pCtx->audio_pes_header_index++] = pCtx->audio_parse & 0xff;
									break;
								case 2:
									pCtx->audio_temp_pts = pCtx->audio_temp_pts | ((unsigned long long)(pCtx->audio_parse & 0xfe) << 14);
									pCtx->audio_pes_header[pCtx->audio_pes_header_index++] = pCtx->audio_parse & 0xff;
									break;
								case 1:
									pCtx->audio_temp_pts = pCtx->audio_temp_pts | ((unsigned long long)(pCtx->audio_parse & 0xff) << 7);
									pCtx->audio_pes_header[pCtx->audio_pes_header_index++] = pCtx->audio_parse & 0xff;
									break;
								case 0:
									pCtx->audio_temp_pts = pCtx->audio_temp_pts | ((unsigned long long)(pCtx->audio_parse & 0xfe) >> 1);
									pCtx->audio_pes_header[pCtx->audio_pes_header_index++] = pCtx->audio_parse & 0xff;
									pCtx->audio_pts = pCtx->audio_temp_pts;
									if (pCtx->audio_pts > pCtx->last_audio_pts)  {
										pCtx->last_audio_pts = pCtx->audio_pts;
									}
									pCtx->first_audio_access_unit = TRUE;
									if(pCtx->dump_audio_pts == TRUE)  {
										DBG_MSG("Audio PTS = %d, %d\r\n", (unsigned int)pCtx->audio_pts, (unsigned int)(pCtx->audio_pts - pCtx->prev_audio_pts));
									}
									pCtx->last_audio_pts_diff = pCtx->audio_pts - pCtx->prev_audio_pts;
									pCtx->prev_audio_pts = pCtx->audio_pts;
									if (pCtx->audio_pes_header_length == 0)  {
										if (pCtx->audio_stream_type == 0x80)  {
											pCtx->audio_lpcm_parse = 4;
										}
										else  {
											pCtx->audio_xfer_state = TRUE;
											if(pCtx->parse_only == FALSE && pCtx->pes_streams == TRUE)  {
												WriteData(pCtx, &pCtx->audio_pes_header[0], 1, pCtx->audio_pes_header_index, STRM_ID_AUD);
											}
										}
									}
									break;
							}
						}
						else if (pCtx->audio_pts_dts_parse != 0)  {
							--pCtx->audio_packet_length;
							--pCtx->audio_pes_header_length;
							--pCtx->audio_pts_dts_parse;
							switch (pCtx->audio_pts_dts_parse)  {
								case 9:
									pCtx->audio_temp_pts = 0;
									pCtx->audio_temp_pts = pCtx->audio_temp_pts | ((unsigned long long)(pCtx->audio_parse & 0xe) << 29);
									pCtx->audio_pes_header[pCtx->audio_pes_header_index++] = pCtx->audio_parse & 0xff;
									break;
								case 8:
									pCtx->audio_temp_pts = pCtx->audio_temp_pts | ((unsigned long long)(pCtx->audio_parse & 0xff) << 22);
									pCtx->audio_pes_header[pCtx->audio_pes_header_index++] = pCtx->audio_parse & 0xff;
									break;
								case 7:
									pCtx->audio_temp_pts = pCtx->audio_temp_pts | ((unsigned long long)(pCtx->audio_parse & 0xfe) << 14);
									pCtx->audio_pes_header[pCtx->audio_pes_header_index++] = pCtx->audio_parse & 0xff;
									break;
								case 6:
									pCtx->audio_temp_pts = pCtx->audio_temp_pts | ((unsigned long long)(pCtx->audio_parse & 0xff) << 7);
									pCtx->audio_pes_header[pCtx->audio_pes_header_index++] = pCtx->audio_parse & 0xff;
									break;
								case 5:
									pCtx->audio_temp_pts = pCtx->audio_temp_pts | ((unsigned long long)(pCtx->audio_parse & 0xfe) >> 1);
									pCtx->audio_pes_header[pCtx->audio_pes_header_index++] = pCtx->audio_parse & 0xff;
									pCtx->audio_pts = pCtx->audio_temp_pts;
									if (pCtx->audio_pts > pCtx->last_audio_pts)  {
										pCtx->last_audio_pts = pCtx->audio_pts;
									}
									pCtx->first_audio_access_unit = TRUE;
									pCtx->last_audio_pts_diff = pCtx->audio_pts - pCtx->prev_audio_pts;
									pCtx->prev_audio_pts = pCtx->audio_pts;
									DBG_MSG("Audio PTS(DTS) = %d\r\n", pCtx->audio_pts);
									break;
								case 4:
									pCtx->audio_pes_header[pCtx->audio_pes_header_index++] = pCtx->audio_parse & 0xff;
									break;
								case 3:
									pCtx->audio_pes_header[pCtx->audio_pes_header_index++] = pCtx->audio_parse & 0xff;
									break;
								case 2:
									pCtx->audio_pes_header[pCtx->audio_pes_header_index++] = pCtx->audio_parse & 0xff;
									break;
								case 1:
									pCtx->audio_pes_header[pCtx->audio_pes_header_index++] = pCtx->audio_parse & 0xff;
									break;
								case 0:
									pCtx->audio_pes_header[pCtx->audio_pes_header_index++] = pCtx->audio_parse & 0xff;
									if (pCtx->audio_pes_header_length == 0)  {
										pCtx->audio_xfer_state = TRUE;
										if(pCtx->parse_only == FALSE && pCtx->pes_streams == TRUE)  {
											WriteData(pCtx, &pCtx->audio_pes_header[0], 1, pCtx->audio_pes_header_index, STRM_ID_AUD);
										}
									}
									break;
							}
						}
						else if (pCtx->audio_lpcm_parse != 0)  {
							--pCtx->audio_packet_length;
							--pCtx->audio_lpcm_parse;
							switch (pCtx->audio_lpcm_parse)  {
								case 3:
									break;
								case 2:
									break;
								case 1:
									break;
								case 0:
									pCtx->audio_lpcm_header_flags = pCtx->audio_parse & 0xffff;
									pCtx->audio_xfer_state = TRUE;
									if(pCtx->parse_only == FALSE && pCtx->pes_streams == TRUE)  {
										WriteData(pCtx, &pCtx->audio_pes_header[0], 1, pCtx->audio_pes_header_index, STRM_ID_AUD);
									}
									break;
							}
						}
						else if (pCtx->audio_pes_header_length != 0)  {
							--pCtx->audio_packet_length;
							--pCtx->audio_pes_header_length;
							pCtx->audio_pes_header[pCtx->audio_pes_header_index++] = pCtx->audio_parse & 0xff;
#if 0
							audio_demux_ptr[audio_demux_buffer_index] = 0xff;
							audio_demux_buffer_index = (audio_demux_buffer_index + 1) & audio_buf_size;
#endif
							switch (pCtx->audio_pes_header_length)  {
								case 0:
									pCtx->audio_xfer_state = TRUE;
									if(pCtx->parse_only == FALSE && pCtx->pes_streams == TRUE)  {
										WriteData(pCtx, &pCtx->audio_pes_header[0], 1, pCtx->audio_pes_header_index, STRM_ID_AUD);
									}
									break;
							}
						}
					}
					if (pCtx->xport_packet_length == 0)  {
						pCtx->sync_state = FALSE;
					}
				}
				else if (pCtx->pid == 0x1ffb || pCtx->pid == pCtx->ett_pid || pCtx->pid == pCtx->eit0_pid || pCtx->pid == pCtx->eit1_pid || pCtx->pid == pCtx->eit2_pid || pCtx->pid == pCtx->eit3_pid || pCtx->pid == pCtx->ett0_pid || pCtx->pid == pCtx->ett1_pid || pCtx->pid == pCtx->eit2_pid || pCtx->pid == pCtx->eit3_pid) {
					if (pCtx->psip_ptr[pCtx->pid]->psip_xfer_state == TRUE)  {
						if ((length - i) >= pCtx->psip_ptr[pCtx->pid]->psip_section_length)  {
							j = pCtx->psip_ptr[pCtx->pid]->psip_section_length;
							if (pCtx->xport_packet_length <= j)  {
								j = pCtx->xport_packet_length;
							}
						}
						else  {
							j = length - i;
							if (pCtx->xport_packet_length <= j)  {
								j = pCtx->xport_packet_length;
							}
						}

						DBG_MSG("Burst length = %d\r\n", j);
						for (k = 0; k < j; k++)  {
							pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_offset] = buffer[i];
							DBG_MSG("PSIP byte %d = 0x%02x\n", pCtx->psip_ptr[pCtx->pid]->psip_offset, buffer[i]);
							pCtx->psip_ptr[pCtx->pid]->psip_offset++;
							i++;
							--pCtx->psip_ptr[pCtx->pid]->psip_section_length;
							--pCtx->xport_packet_length;
							pCtx->pcr_bytes++;
						}
						--i;	/* adjust because of for loop */
						if (pCtx->psip_ptr[pCtx->pid]->psip_section_length == 0)  {
							DBG_MSG("End of PSIP section = %d\r\n",i);
							pCtx->psip_ptr[pCtx->pid]->psip_xfer_state = FALSE;
							if (pCtx->psip_ptr[pCtx->pid]->psip_section_number == pCtx->psip_ptr[pCtx->pid]->psip_last_section_number)  {
								if (pCtx->psip_ptr[pCtx->pid]->psip_table_id == 0xc7 && pCtx->mgt_version_number != pCtx->mgt_last_version_number)  {
									pCtx->mgt_last_version_number = pCtx->mgt_version_number;
									pCtx->psip_ptr[pCtx->pid]->psip_index = 0;
									pCtx->mgt_tables_defined = pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++] << 8;
									pCtx->mgt_tables_defined |= pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++];
									DBG_MSG("MGT tables defined = %d\n\n", pCtx->mgt_tables_defined);
									for (k = 0; k < pCtx->mgt_tables_defined; k++)  {
										pCtx->mgt_table_type = pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++] << 8;
										pCtx->mgt_table_type |= pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++];
										DBG_MSG("MGT table type = 0x%04x\n", pCtx->mgt_table_type);
										pCtx->mgt_table_type_pid = (pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++] & 0x1f) << 8;
										pCtx->mgt_table_type_pid |= pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++];
										DBG_MSG("MGT table type pCtx->pid = 0x%04x\n", pCtx->mgt_table_type_pid);
										if (pCtx->mgt_table_type == 0x4)  {
											pCtx->ett_pid = pCtx->mgt_table_type_pid;
											pCtx->psip_pid_table[pCtx->ett_pid] = pCtx->mgt_table_type;
											pCtx->psip_ptr[pCtx->ett_pid] = (psip_t*)malloc(sizeof(struct psip));
											pCtx->psip_ptr[pCtx->ett_pid]->psip_section_start = FALSE;
											pCtx->psip_ptr[pCtx->ett_pid]->psip_pointer_field = 0;
											pCtx->psip_ptr[pCtx->ett_pid]->psip_section_length_parse = 0;
											pCtx->psip_ptr[pCtx->ett_pid]->psip_section_parse = 0;
											pCtx->psip_ptr[pCtx->ett_pid]->psip_xfer_state = FALSE;
										}
										else if (pCtx->mgt_table_type == 0x100)  {
											pCtx->eit0_pid = pCtx->mgt_table_type_pid;
											pCtx->psip_pid_table[pCtx->eit0_pid] = pCtx->mgt_table_type;
											pCtx->psip_ptr[pCtx->eit0_pid] = (psip_t*)malloc(sizeof(struct psip));
											pCtx->psip_ptr[pCtx->eit0_pid]->psip_section_start = FALSE;
											pCtx->psip_ptr[pCtx->eit0_pid]->psip_pointer_field = 0;
											pCtx->psip_ptr[pCtx->eit0_pid]->psip_section_length_parse = 0;
											pCtx->psip_ptr[pCtx->eit0_pid]->psip_section_parse = 0;
											pCtx->psip_ptr[pCtx->eit0_pid]->psip_xfer_state = FALSE;
										}
										else if (pCtx->mgt_table_type == 0x101)  {
											pCtx->eit1_pid = pCtx->mgt_table_type_pid;
											pCtx->psip_pid_table[pCtx->eit1_pid] = pCtx->mgt_table_type;
											pCtx->psip_ptr[pCtx->eit1_pid] = (psip_t*)malloc(sizeof(struct psip));
											pCtx->psip_ptr[pCtx->eit1_pid]->psip_section_start = FALSE;
											pCtx->psip_ptr[pCtx->eit1_pid]->psip_pointer_field = 0;
											pCtx->psip_ptr[pCtx->eit1_pid]->psip_section_length_parse = 0;
											pCtx->psip_ptr[pCtx->eit1_pid]->psip_section_parse = 0;
											pCtx->psip_ptr[pCtx->eit1_pid]->psip_xfer_state = FALSE;
										}
										else if (pCtx->mgt_table_type == 0x102)  {
											pCtx->eit2_pid = pCtx->mgt_table_type_pid;
											pCtx->psip_pid_table[pCtx->eit2_pid] = pCtx->mgt_table_type;
											pCtx->psip_ptr[pCtx->eit2_pid] = (psip_t*)malloc(sizeof(struct psip));
											pCtx->psip_ptr[pCtx->eit2_pid]->psip_section_start = FALSE;
											pCtx->psip_ptr[pCtx->eit2_pid]->psip_pointer_field = 0;
											pCtx->psip_ptr[pCtx->eit2_pid]->psip_section_length_parse = 0;
											pCtx->psip_ptr[pCtx->eit2_pid]->psip_section_parse = 0;
											pCtx->psip_ptr[pCtx->eit2_pid]->psip_xfer_state = FALSE;
										}
										else if (pCtx->mgt_table_type == 0x103)  {
											pCtx->eit3_pid = pCtx->mgt_table_type_pid;
											pCtx->psip_pid_table[pCtx->eit3_pid] = pCtx->mgt_table_type;
											pCtx->psip_ptr[pCtx->eit3_pid] = (psip_t*)malloc(sizeof(struct psip));
											pCtx->psip_ptr[pCtx->eit3_pid]->psip_section_start = FALSE;
											pCtx->psip_ptr[pCtx->eit3_pid]->psip_pointer_field = 0;
											pCtx->psip_ptr[pCtx->eit3_pid]->psip_section_length_parse = 0;
											pCtx->psip_ptr[pCtx->eit3_pid]->psip_section_parse = 0;
											pCtx->psip_ptr[pCtx->eit3_pid]->psip_xfer_state = FALSE;
										}
										else if (pCtx->mgt_table_type == 0x200)  {
											pCtx->ett0_pid = pCtx->mgt_table_type_pid;
											pCtx->psip_pid_table[pCtx->ett0_pid] = pCtx->mgt_table_type;
											pCtx->psip_ptr[pCtx->ett0_pid] = (psip_t*)malloc(sizeof(struct psip));
											pCtx->psip_ptr[pCtx->ett0_pid]->psip_section_start = FALSE;
											pCtx->psip_ptr[pCtx->ett0_pid]->psip_pointer_field = 0;
											pCtx->psip_ptr[pCtx->ett0_pid]->psip_section_length_parse = 0;
											pCtx->psip_ptr[pCtx->ett0_pid]->psip_section_parse = 0;
											pCtx->psip_ptr[pCtx->ett0_pid]->psip_xfer_state = FALSE;
										}
										else if (pCtx->mgt_table_type == 0x201)  {
											pCtx->ett1_pid = pCtx->mgt_table_type_pid;
											pCtx->psip_pid_table[pCtx->ett1_pid] = pCtx->mgt_table_type;
											pCtx->psip_ptr[pCtx->ett1_pid] = (psip_t*)malloc(sizeof(struct psip));
											pCtx->psip_ptr[pCtx->ett1_pid]->psip_section_start = FALSE;
											pCtx->psip_ptr[pCtx->ett1_pid]->psip_pointer_field = 0;
											pCtx->psip_ptr[pCtx->ett1_pid]->psip_section_length_parse = 0;
											pCtx->psip_ptr[pCtx->ett1_pid]->psip_section_parse = 0;
											pCtx->psip_ptr[pCtx->ett1_pid]->psip_xfer_state = FALSE;
										}
										else if (pCtx->mgt_table_type == 0x202)  {
											pCtx->ett2_pid = pCtx->mgt_table_type_pid;
											pCtx->psip_pid_table[pCtx->ett2_pid] = pCtx->mgt_table_type;
											pCtx->psip_ptr[pCtx->ett2_pid] = (psip_t*)malloc(sizeof(struct psip));
											pCtx->psip_ptr[pCtx->ett2_pid]->psip_section_start = FALSE;
											pCtx->psip_ptr[pCtx->ett2_pid]->psip_pointer_field = 0;
											pCtx->psip_ptr[pCtx->ett2_pid]->psip_section_length_parse = 0;
											pCtx->psip_ptr[pCtx->ett2_pid]->psip_section_parse = 0;
											pCtx->psip_ptr[pCtx->ett2_pid]->psip_xfer_state = FALSE;
										}
										else if (pCtx->mgt_table_type == 0x203)  {
											pCtx->ett3_pid = pCtx->mgt_table_type_pid;
											pCtx->psip_pid_table[pCtx->ett3_pid] = pCtx->mgt_table_type;
											pCtx->psip_ptr[pCtx->ett3_pid] = (psip_t*)malloc(sizeof(struct psip));
											pCtx->psip_ptr[pCtx->ett3_pid]->psip_section_start = FALSE;
											pCtx->psip_ptr[pCtx->ett3_pid]->psip_pointer_field = 0;
											pCtx->psip_ptr[pCtx->ett3_pid]->psip_section_length_parse = 0;
											pCtx->psip_ptr[pCtx->ett3_pid]->psip_section_parse = 0;
											pCtx->psip_ptr[pCtx->ett3_pid]->psip_xfer_state = FALSE;
										}
										pCtx->mgt_table_type_version = pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++] & 0x1f;
										DBG_MSG("MGT table type version = 0x%02x\n", pCtx->mgt_table_type_version);
										pCtx->mgt_number_bytes = pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++] << 24;
										pCtx->mgt_number_bytes |= pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++] << 16;
										pCtx->mgt_number_bytes |= pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++] << 8;
										pCtx->mgt_number_bytes |= pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++];
										DBG_MSG("MGT table bytes = 0x%08x\n", pCtx->mgt_number_bytes);
										pCtx->mgt_table_type_desc_length = (pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++] & 0xf) << 8;
										pCtx->mgt_table_type_desc_length |= pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++];
										DBG_MSG("MGT table desc bytes = 0x%04x\n\n", pCtx->mgt_table_type_desc_length);
										for (m = 0; m < pCtx->mgt_table_type_desc_length; m++)  {
											pCtx->psip_ptr[pCtx->pid]->psip_index++;
										}
									}
									pCtx->mgt_desc_length = (pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++] & 0xf) << 8;
									pCtx->mgt_desc_length |= pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++];
									DBG_MSG("MGT desc bytes = 0x%04x\n", pCtx->mgt_desc_length);
									for (m = 0; m < pCtx->mgt_desc_length; m++)  {
										pCtx->psip_ptr[pCtx->pid]->psip_index++;
									}
									pCtx->mgt_crc = pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++] << 24;
									pCtx->mgt_crc |= pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++] << 16;
									pCtx->mgt_crc |= pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++] << 8;
									pCtx->mgt_crc |= pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++];
									DBG_MSG("MGT CRC = 0x%08x, %d, %d\n", pCtx->mgt_crc, pCtx->psip_ptr[pCtx->pid]->psip_offset, pCtx->psip_ptr[pCtx->pid]->psip_index);
									DBG_MSG("\n");
								}
								else if (pCtx->psip_ptr[pCtx->pid]->psip_table_id == 0xc8 && pCtx->vct_version_number != pCtx->vct_last_version_number)  {
									pCtx->vct_last_version_number = pCtx->vct_version_number;
									pCtx->psip_ptr[pCtx->pid]->psip_index = 0;
									pCtx->vct_num_channels = pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++];
									DBG_MSG("TVCT number of channels = %d\n\n", pCtx->vct_num_channels);
									for (k = 0; k < pCtx->vct_num_channels; k++)  {
										DBG_MSG("TVCT short name = ");
										for (m = 0; m < 14; m++)  {
											if (pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index] != 0)  {
												DBG_MSG("%c", pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++]);
											}
											else  {
												pCtx->psip_ptr[pCtx->pid]->psip_index++;
											}
										}
										DBG_MSG("\n");
										pCtx->vct_major_channel_number = (pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++] & 0xf) << 8;
										pCtx->vct_major_channel_number |= pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index] & 0xfc;
										pCtx->vct_major_channel_number >>= 2;
										pCtx->vct_minor_channel_number = (pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++] & 0x3) << 8;
										pCtx->vct_minor_channel_number |= pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++];
										DBG_MSG("TVCT channel number = %d.%d\n", pCtx->vct_major_channel_number, pCtx->vct_minor_channel_number);
										pCtx->vct_modulation_mode = pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++];
										DBG_MSG("TVCT modulation mode = 0x%02x\n", pCtx->vct_modulation_mode);
										pCtx->psip_ptr[pCtx->pid]->psip_index += 4;
										pCtx->vct_channel_tsid = pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++] << 8;
										pCtx->vct_channel_tsid |= pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++];
										DBG_MSG("TVCT channel TSID = 0x%04x\n", pCtx->vct_channel_tsid);
										pCtx->vct_program_number = pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++] << 8;
										pCtx->vct_program_number |= pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++];
										DBG_MSG("TVCT program number = 0x%04x\n", pCtx->vct_program_number);
										pCtx->psip_ptr[pCtx->pid]->psip_index++;
										pCtx->vct_service_type = pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++] & 0x3f;
										DBG_MSG("TVCT service type = 0x%04x\n", pCtx->vct_service_type);
										pCtx->vct_source_id = pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++] << 8;
										pCtx->vct_source_id |= pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++];
										DBG_MSG("TVCT source id = 0x%04x\n", pCtx->vct_source_id);
										pCtx->vct_desc_length = (pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++] & 0x3) << 8;
										pCtx->vct_desc_length |= pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++];
										DBG_MSG("TVCT desc bytes = 0x%04x\n\n", pCtx->vct_desc_length);
										while (pCtx->vct_desc_length != 0)  {
											if (pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index] == 0xa0)  {
												pCtx->psip_ptr[pCtx->pid]->psip_index++;
												pCtx->ecnd_desc_length = pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++];
												pCtx->vct_desc_length -= (pCtx->ecnd_desc_length + 2);
												DBG_MSG("Extended Channel Name = ");
												for (m = 0; m < pCtx->ecnd_desc_length; m++)  {
													DBG_MSG("%c", pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++]);
												}
												DBG_MSG("\n\n");
											}
											else if (pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index] == 0xa1)  {
												pCtx->psip_ptr[pCtx->pid]->psip_index++;
												pCtx->sld_desc_length = pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++];
												pCtx->vct_desc_length -= (pCtx->sld_desc_length + 2);
												pCtx->sld_pcr_pid = (pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++] & 0x1f) << 8;
												pCtx->sld_pcr_pid |= pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++];
												DBG_MSG("SLD PCR pCtx->pid = 0x%04x\n", pCtx->sld_pcr_pid);
												pCtx->sld_num_elements = pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++];
												for (m = 0; m < pCtx->sld_num_elements; m++)  {
													pCtx->sld_stream_type = pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++];
													DBG_MSG("SLD stream type = 0x%02x\n", pCtx->sld_stream_type);
													pCtx->sld_elementary_pid = (pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++] & 0x1f) << 8;
													pCtx->sld_elementary_pid |= pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++];
													DBG_MSG("SLD elementary pCtx->pid = 0x%04x\n", pCtx->sld_elementary_pid);
													DBG_MSG("SLD language code = ");
													for (n = 0; n < 3; n++)  {
														if (pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index] != 0)  {
															DBG_MSG("%c", pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++]);
														}
														else  {
															pCtx->psip_ptr[pCtx->pid]->psip_index++;
														}
													}
													DBG_MSG("\n\n");
												}
											}
											else if (pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index] == 0xa2)  {
												pCtx->psip_ptr[pCtx->pid]->psip_index++;
											}
										}
									}
									pCtx->vct_add_desc_length = (pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++] & 0x3) << 8;
									pCtx->vct_add_desc_length |= pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++];
									DBG_MSG("TVCT additional desc bytes = 0x%04x\n", pCtx->vct_add_desc_length);
									for (m = 0; m < pCtx->vct_add_desc_length; m++)  {
										pCtx->psip_ptr[pCtx->pid]->psip_index++;
									}
									pCtx->vct_crc = pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++] << 24;
									pCtx->vct_crc |= pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++] << 16;
									pCtx->vct_crc |= pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++] << 8;
									pCtx->vct_crc |= pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++];
									DBG_MSG("TVCT CRC = 0x%08x, %d, %d\n", pCtx->vct_crc, pCtx->psip_ptr[pCtx->pid]->psip_offset, pCtx->psip_ptr[pCtx->pid]->psip_index);
									DBG_MSG("\n");
								}
								else if (pCtx->psip_ptr[pCtx->pid]->psip_table_id == 0xca)  {
								}
								else if (pCtx->psip_ptr[pCtx->pid]->psip_table_id == 0xcb && pCtx->eit_version_number != pCtx->eit_last_version_number[pCtx->psip_pid_table[pCtx->pid] & 0x3])  {
									pCtx->eit_last_version_number[pCtx->psip_pid_table[pCtx->pid] & 0x3] = pCtx->eit_version_number;
									pCtx->psip_ptr[pCtx->pid]->psip_index = 0;
									pCtx->eit_num_events = pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++];
									DBG_MSG("EIT%d events defined = %d\n\n", pCtx->psip_pid_table[pCtx->pid] & 0x3, pCtx->eit_num_events);
									for (k = 0; k < pCtx->eit_num_events; k++)  {
										pCtx->eit_event_id = (pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++] & 0x3f) << 8;
										pCtx->eit_event_id |= pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++];
										DBG_MSG("EIT event id = 0x%04x\n", pCtx->eit_event_id);
										pCtx->eit_start_time = pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++] << 24;
										pCtx->eit_start_time |= pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++] << 16;
										pCtx->eit_start_time |= pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++] << 8;
										pCtx->eit_start_time |= pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++];
										DBG_MSG("EIT start time = 0x%08x\n", pCtx->eit_start_time);
										pCtx->eit_length_secs = (pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++] & 0xf) << 16;
										pCtx->eit_length_secs |= pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++] << 8;
										pCtx->eit_length_secs |= pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++];
										DBG_MSG("EIT length in seconds = %d\n", pCtx->eit_length_secs);
										pCtx->eit_title_length = pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++];
										DBG_MSG("EIT title length = 0x%02x\n", pCtx->eit_title_length);
										for (m = 0; m < pCtx->eit_title_length; m++)  {
											if ((pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index] >= 0x20) && (pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index] < 0x7f))  {
												DBG_MSG("%c", pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index]);
											}
											pCtx->psip_ptr[pCtx->pid]->psip_index++;
										}
										DBG_MSG("\n");
										pCtx->eit_desc_length = (pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++] & 0xf) << 8;
										pCtx->eit_desc_length |= pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++];
										DBG_MSG("EIT desc bytes = 0x%04x\n", pCtx->eit_desc_length);
										while (pCtx->eit_desc_length != 0)  {
											if (pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index] == 0x81)  {
												DBG_MSG("AC-3 Audio Descriptor\n");
												pCtx->psip_ptr[pCtx->pid]->psip_index++;
												pCtx->ac3_desc_length = pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++];
												pCtx->eit_desc_length -= (pCtx->ac3_desc_length + 2);
												pCtx->psip_ptr[pCtx->pid]->psip_index += pCtx->ac3_desc_length;
											}
											else if (pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index] == 0x86)  {
												DBG_MSG("Caption Service Descriptor\n");
												pCtx->psip_ptr[pCtx->pid]->psip_index++;
												pCtx->csd_desc_length = pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++];
												pCtx->eit_desc_length -= (pCtx->csd_desc_length + 2);
												pCtx->psip_ptr[pCtx->pid]->psip_index += pCtx->csd_desc_length;
											}
											else if (pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index] == 0x87)  {
												DBG_MSG("Content Advisory Descriptor\n");
												pCtx->psip_ptr[pCtx->pid]->psip_index++;
												pCtx->cad_desc_length = pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++];
												pCtx->eit_desc_length -= (pCtx->cad_desc_length + 2);
												pCtx->psip_ptr[pCtx->pid]->psip_index += pCtx->cad_desc_length;
											}
											else if (pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index] == 0xaa)  {
												pCtx->psip_ptr[pCtx->pid]->psip_index++;
												pCtx->rcd_desc_length = pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++];
												pCtx->eit_desc_length -= (pCtx->rcd_desc_length + 2);
												DBG_MSG("RCD information = ");
												for (m = 0; m < pCtx->rcd_desc_length; m++)  {
													DBG_MSG("0x%02x, ", pCtx->psip_ptr[pCtx->pid]->psip_table[pCtx->psip_ptr[pCtx->pid]->psip_index++]);
												}
												DBG_MSG("\n");
											}
										}
										DBG_MSG("\n");
									}
									DBG_MSG("\n");
								}
								else if (pCtx->psip_ptr[pCtx->pid]->psip_table_id == 0xcd)  {
								}
								else if (pCtx->psip_ptr[pCtx->pid]->psip_table_id == 0xd3)  {
								}
								else if (pCtx->psip_ptr[pCtx->pid]->psip_table_id == 0xd4)  {
								}
							}
						}
					}
					else  {
						--pCtx->xport_packet_length;
						pCtx->pcr_bytes++;
						if (pCtx->psip_ptr[pCtx->pid]->psip_section_start == TRUE)  {
							pCtx->psip_ptr[pCtx->pid]->psip_pointer_field = buffer[i];
							if (pCtx->psip_ptr[pCtx->pid]->psip_pointer_field == 0)  {
								pCtx->psip_ptr[pCtx->pid]->psip_section_length_parse = 3;
							}
							pCtx->psip_ptr[pCtx->pid]->psip_section_start = FALSE;
						}
						else if (pCtx->psip_ptr[pCtx->pid]->psip_pointer_field != 0)  {
							--pCtx->psip_ptr[pCtx->pid]->psip_pointer_field;
							switch (pCtx->psip_ptr[pCtx->pid]->psip_pointer_field)  {
								case 0:
									pCtx->psip_ptr[pCtx->pid]->psip_section_length_parse = 3;
									break;
							}
						}
						else if (pCtx->psip_ptr[pCtx->pid]->psip_section_length_parse != 0)  {
							--pCtx->psip_ptr[pCtx->pid]->psip_section_length_parse;
							switch (pCtx->psip_ptr[pCtx->pid]->psip_section_length_parse)  {
								case 2:
									pCtx->psip_ptr[pCtx->pid]->psip_table_id = buffer[i];
									break;
								case 1:
									pCtx->psip_ptr[pCtx->pid]->psip_section_length = (buffer[i] & 0xf) << 8;
									break;
								case 0:
									pCtx->psip_ptr[pCtx->pid]->psip_section_length |= buffer[i];

									DBG_MSG("Section length = %d\r\n", pCtx->psip_ptr[pCtx->pid]->psip_section_length);
									pCtx->psip_ptr[pCtx->pid]->psip_section_parse = 6;
									break;
							}
						}
						else if (pCtx->psip_ptr[pCtx->pid]->psip_section_parse != 0)  {
							--pCtx->psip_ptr[pCtx->pid]->psip_section_length;
							--pCtx->psip_ptr[pCtx->pid]->psip_section_parse;
							switch (pCtx->psip_ptr[pCtx->pid]->psip_section_parse)  {
								case 5:
									pCtx->psip_ptr[pCtx->pid]->psip_table_id_ext = buffer[i] << 8;
									break;
								case 4:
									pCtx->psip_ptr[pCtx->pid]->psip_table_id_ext |= buffer[i];
									break;
								case 3:
									switch (pCtx->psip_ptr[pCtx->pid]->psip_table_id)  {
										case 0xc7:
											pCtx->mgt_version_number = buffer[i] & 0x1f;
											break;
										case 0xc8:
											pCtx->vct_version_number = buffer[i] & 0x1f;
											break;
										case 0xca:
											break;
										case 0xcb:
											pCtx->eit_version_number = buffer[i] & 0x1f;
											break;
										case 0xcd:
											break;
										case 0xd3:
											break;
										case 0xd4:
											break;
									}
									break;
								case 2:
									pCtx->psip_ptr[pCtx->pid]->psip_section_number = buffer[i];
									if (pCtx->psip_ptr[pCtx->pid]->psip_section_number == 0)  {
										pCtx->psip_ptr[pCtx->pid]->psip_offset = 0;
									}
									break;
								case 1:
									pCtx->psip_ptr[pCtx->pid]->psip_last_section_number = buffer[i];
									break;
								case 0:
									pCtx->psip_ptr[pCtx->pid]->psip_xfer_state = TRUE;
									break;
							}
						}
					}
					if (pCtx->xport_packet_length == 0)  {
						pCtx->sync_state = FALSE;
					}
				}
				else  {
					// Ram: Unknown PID ?
					--pCtx->xport_packet_length;
					pCtx->pcr_bytes++;
					if ((length - i) >= pCtx->xport_packet_length)  {
						i = i + pCtx->xport_packet_length;
						pCtx->pcr_bytes = pCtx->pcr_bytes + pCtx->xport_packet_length;
						pCtx->xport_packet_length = 0;
					}
					else  {
						pCtx->xport_packet_length = pCtx->xport_packet_length - (length - i) + 1;
						pCtx->pcr_bytes = pCtx->pcr_bytes + (length - i) - 1;
						i = length;
					}
					if (pCtx->xport_packet_length == 0)  {
						pCtx->sync_state = FALSE;
					}
				}
			}
			else  {
				// Ram: Seach for Sync byte
				sync = buffer[i];
				if (pCtx->hdmv_mode == FALSE)  {
					if (sync == 0x47)  {
						pCtx->fDisCont = 0;
						pCtx->sync_state = TRUE;
						pCtx->xport_packet_length = 187;
						pCtx->pcr_bytes++;
						pCtx->xport_header_parse = 3;
						if (pCtx->skipped_bytes != 0)  {
							DBG_MSG("Transport Sync Error, skipped %d bytes, at %d\n", pCtx->skipped_bytes, pCtx->packet_counter);
							pCtx->skipped_bytes = 0;
						}
					}
					else  {
						pCtx->skipped_bytes++;
					}
				}
				else  {
					if (pCtx->tp_extra_header_parse != 0)  {
						--pCtx->tp_extra_header_parse;
						switch (pCtx->tp_extra_header_parse)  {
							case 3:
								tp_extra_header = 0;
								tp_extra_header |= (buffer[i] & 0x3f) << 24;
								break;
							case 2:
								tp_extra_header |= (buffer[i] & 0xff) << 16;
								break;
							case 1:
								tp_extra_header |= (buffer[i] & 0xff) << 8;
								break;
							case 0:
								tp_extra_header |= (buffer[i] & 0xff);
								if (pCtx->dump_extra == TRUE)  {
									DBG_MSG("arrival_time_stamp delta = %d\n", tp_extra_header - pCtx->tp_extra_header_prev);
								}
								pCtx->tp_extra_header_prev = tp_extra_header;
								break;
						}
					}
					else if (sync == 0x47)  {
						pCtx->fDisCont = 0;
						pCtx->sync_state = TRUE;
						pCtx->xport_packet_length = 187;
						tp_extra_header_pcr_bytes = pCtx->pcr_bytes;
						pCtx->pcr_bytes++;
						pCtx->xport_header_parse = 3;
						if (pCtx->skipped_bytes != 0)  {
							DBG_MSG("Transport Sync Error, skipped %d bytes, at %d\n", pCtx->skipped_bytes, pCtx->packet_counter);
							pCtx->skipped_bytes = 0;
						}
						pCtx->tp_extra_header_parse = 4;
					}
					else  {
						pCtx->skipped_bytes++;
					}
				}
			}
		}
	}
	else  {
	}
}


int
DisplayStat(MpegTsDemuxCtx *self)
{
    int64_t ts;
	int64_t stream_ts;
    ts = ClockGetInternalTime(self->pClk);
	stream_ts = ClockGetTime(self->pClk);

    if (self->start_ts == 0) {
         self->interval_ts = self->last_ts = self->start_ts = ts;
    }

    if (ts - self->interval_ts  > self->stat_update_interval) {
		int64_t totalbytes;
		double rr, average_bitrate;
		char stat_message[512];
		char stat_time[256];
		char stat_pcr[256];
		char stat_pcr_arrival_time[256];
		char stat_stream_time[256];
		double time_diff, time_elapsed;
		int clockDiff;

		int64_t current_ts = ts;
        self->interval_ts = ts;

		totalbytes = self->read_position;

		time_diff = (double) (current_ts - self->last_ts) / TIME_SECOND;
		time_elapsed = (double) (current_ts - self->start_ts) / TIME_SECOND;

		rr = (double) (totalbytes - self->last_totalbytes) * 8 / time_diff;
		average_bitrate = (double) totalbytes * 8 / time_elapsed;

		Clock2HMSF(current_ts, stat_time, 255);
		Clock2HMSF(self->current_pcr / 27, stat_pcr, 255);
		Clock2HMSF(self->pcr_arrival_time, stat_pcr_arrival_time, 255);
		Clock2HMSF(stream_ts, stat_stream_time, 255);

		clockDiff = (self->current_pcr / 27 - (current_ts - self->start_ts)) / TIME_MILLISEC;
		snprintf (stat_message, 511, "<%s:xport: pCtx->pcr=%s  pcr_arriv=%s stream=%s t_bytes=%lld ts=%9d  crnt=%9d  avg=%9d",
				stat_time, stat_pcr, stat_pcr_arrival_time, stat_stream_time, totalbytes, (unsigned int)self->current_tsrate * 8, (int)rr, (int)average_bitrate);
		DBG_MSG("StrmId=%d %s\n", self->nInstanceId, stat_message);

		self->last_totalbytes = totalbytes;
		self->last_ts = current_ts;
	}
    return TRUE;
}



void Demux_DumpStat(MpegTsDemuxCtx *pCtx)
{
	char szTmp[128] = {0};
	DBG_MSG("\nDemux Statistics:\n");

	Clock2HMSF(pCtx->current_pcr / 27, szTmp,127);
	DBG_MSG("Current PCR=%s\n", szTmp);

	Clock2HMSF(pCtx->crnt_vid_pts, szTmp,127);
	DBG_MSG("Current Vid PTS=%s",  szTmp);
	Clock2HMSF(pCtx->crnt_aud_pts, szTmp,127);
	DBG_MSG("  Current Aud PTS=%s\n",  szTmp);

	DBG_MSG("Sent Pkts Vids=%d Aud=%d\n", pCtx->VidChunkCnt, pCtx->AudChunkCnt);
}


void demuxDelete(StrmCompIf *pComp)
{
	if(pComp->pCtx) {
		free(pComp->pCtx);
	}
	free(pComp);
	//return 0;
}

StrmCompIf *
demuxCreate()
{
	StrmCompIf *pComp = (StrmCompIf *)malloc(sizeof(StrmCompIf));
	pComp->pCtx = malloc(sizeof(MpegTsDemuxCtx));
	memset(pComp->pCtx, 0x00, sizeof(MpegTsDemuxCtx));

	pComp->Open=demuxOpen;
	pComp->SetOption = demuxSetOption;
	pComp->SetInputConn= demuxSetInputConn;
	pComp->SetOutputConn= demuxSetOutputConn;
	pComp->SetClkSrc = demuxSetClkSrc;
	pComp->GetClkSrc = demuxGetClkSrc;
	pComp->Start = demuxStart;
	pComp->Stop = demuxStop;
	pComp->Close = demuxClose;
	pComp->Delete = demuxDelete;

	return pComp;
}


#ifdef UNIT_TEST_DEMUX
//int gDbgLevel = 3;
int main(int argc, char **argv)
{
	FILE	*fp;
	unsigned char	buffer[16384];
	unsigned int	i, length;
	int			temp;
	char	videofilename[] = {"bits0001.mpv"};
	char	audiofilename[] = {"bits0001.mpa"};

	// Ram: added the following to tglle between program detection and use user supplied PIDs
	StrmCompIf *pComp = demuxCreate();
	MpegTsDemuxCtx *pCtx = (MpegTsDemuxCtx*)pComp->pCtx;
	void *pClk = demuxGetClkSrc(pComp);

	if (argc < 7) {
		fprintf(stderr, "xport Transport Stream Demuxer 1.01\n\n");
		fprintf(stderr, "Usage:\n");
		fprintf(stderr, "\txport <-pavtdszgher2> <infile> <detect_program_pids> <program number>/pcr_pid> <video stream number/vid_pid> <audio stream number/aud_pd> [<vid_stream_type> <aud_stream_type>]\n\n");
		fprintf(stderr, "Options:\n");
		fprintf(stderr, "\tp = parse only, do not demux to video and audio files\n");
		fprintf(stderr, "\ta = dump audio PTS\n");
		fprintf(stderr, "\tv = dump video PTS\n");
		fprintf(stderr, "\tt = GOP timecode mode, count repeated fields/frames\n");
		fprintf(stderr, "\td = dump all PID's (useful for debugging muxers, but tons of output)\n");
		fprintf(stderr, "\ts = suppress TS rate dumping (useful when piping output to a file)\n");
		fprintf(stderr, "\tz = demux to PES streams (instead of elementary streams)\n");
		fprintf(stderr, "\tg = dump ATSC PSIP information\n");
		fprintf(stderr, "\th = input file is in HDMV (AVCHD and Blu-ray) format (192 byte packets)\n");
		fprintf(stderr, "\te = dump HDMV arrival_time_stamp difference\n");
		fprintf(stderr, "\tr = dump PCR\n");
		fprintf(stderr, "\t2 = only extract 2 channels of HDMV LPCM audio from multi-channel tracks\n");
		exit(-1);
	}

		if(*argv[1] == OPTION_CHAR)  {
			for(i = 1; i < strlen(argv[1]); i++)  {
				switch (argv[1][i])  {
					case 'p':
					case 'P':
						pCtx->parse_only = TRUE;
						break;
					case 'a':
					case 'A':
						pCtx->dump_audio_pts = TRUE;
						break;
					case 'v':
					case 'V':
						pCtx->dump_video_pts = TRUE;
						break;
					case 't':
					case 'T':
						pCtx->timecode_mode = TRUE;
						break;
					case 'd':
					case 'D':
						pCtx->dump_pids = TRUE;
						break;
					case 's':
					case 'S':
						pCtx->suppress_tsrate = TRUE;
						break;
					case 'z':
					case 'Z':
						pCtx->pes_streams = TRUE;
						break;
					case 'g':
					case 'G':
						pCtx->dump_psip = TRUE;
						break;
					case 'h':
					case 'H':
						pCtx->hdmv_mode = TRUE;
						break;
					case 'e':
					case 'E':
						pCtx->dump_extra = TRUE;
						break;
					case 'r':
					case 'R':
						pCtx->dump_pcr = TRUE;
						break;
					case '2':
						pCtx->lpcm_mode = TRUE;
						break;
					default:
						fprintf(stderr, "Unsupported Option: %c\n", argv[1][i]);
				}
			}
		}
		else  {
			fprintf(stderr, "xport Transport Stream Demuxer 1.01\n\n");
			fprintf(stderr, "Usage:\n");
			fprintf(stderr, "\txport <-pavtdszgher2> <infile> <program number> <video stream number> <audio stream number>\n\n");
			fprintf(stderr, "Options:\n");
			fprintf(stderr, "\tp = parse only, do not demux to video and audio files\n");
			fprintf(stderr, "\ta = dump audio PTS\n");
			fprintf(stderr, "\tv = dump video PTS\n");
			fprintf(stderr, "\tt = GOP timecode mode, count repeated fields/frames\n");
			fprintf(stderr, "\td = dump all PID's (useful for debugging muxers, but tons of output)\n");
			fprintf(stderr, "\ts = suppress TS rate dumping (useful when piping output to a file)\n");
			fprintf(stderr, "\tz = demux to PES streams (instead of elementary streams)\n");
			fprintf(stderr, "\tg = dump ATSC PSIP information\n");
			fprintf(stderr, "\th = input file is in HDMV (AVCHD and Blu-ray) format (192 byte packets)\n");
			fprintf(stderr, "\te = dump HDMV arrival_time_stamp difference\n");
			fprintf(stderr, "\tr = dump PCR\n");
			fprintf(stderr, "\t2 = only extract 2 channels of HDMV LPCM audio from multi-channel tracks\n");
			exit(-1);
		}
		/*--- open binary file (for parsing) ---*/
		fp = fopen(argv[2], "rb");
		if (fp == 0) {
			fprintf(stderr, "Cannot open bitstream file <%s>\n", argv[2]);
			exit(-1);
		}


	if(pCtx->parse_only == FALSE)  {
		/*--- open binary file (for video output) ---*/
		pCtx->fpoutvideo = fopen(&videofilename[0], "wb");
		if (pCtx->fpoutvideo == 0) {
			fprintf(stderr, "Cannot open video output file <%s>\n", &videofilename[0]);
			exit(-1);
		}

		/*--- open binary file (for audio output) ---*/
		pCtx->fpoutaudio = fopen(&audiofilename[0], "wb");
		if (pCtx->fpoutaudio == 0) {
			fprintf(stderr, "Cannot open audio output file <%s>\n", &audiofilename[0]);
			exit(-1);
		}
	}

	{
		pCtx->detect_program_pids = atoi(argv[3]);
		if(pCtx->detect_program_pids) {
			pCtx->program = atoi(argv[4]);
			pCtx->audio_channel = atoi(argv[5]);
			pCtx->video_channel = atoi(argv[6]);
		} else {
			pCtx->pcr_pid = atoi(argv[4]);
			pCtx->audio_pid = atoi(argv[5]);
			pCtx->video_pid = atoi(argv[6]);
			if(argc >= 9) {
				pCtx->audio_stream_type = atoi(argv[7]);
				pCtx->video_stream_type = atoi(argv[8]);
			} else {
				pCtx->audio_stream_type = 0x0F; // AAC
				pCtx->video_stream_type = 0x1B; // H.264
			}
		}
	}
	DBG_MSG("xport Transport Stream Demuxer 1.01\n");
	DBG_MSG("program = %d, video channel = %d, audio channel = %d\n", pCtx->program, pCtx->video_channel, pCtx->audio_channel);
	demux_mpeg2_transport_init(pCtx);

	while(!feof(fp))  {
		length = fread(&buffer[0], 1, 16384, fp);
		demux_mpeg2_transport(pCtx, length, &buffer[0]);
	}
	DBG_MSG("\n");
	for(i = 0; i < 0x2000; i++)  {
		if(pCtx->pid_counter[i] != 0)  {
			DBG_MSG("packets for pCtx->pid %4d <0x%04x> = %d, first = %d, last = %d\n", i, i, pCtx->pid_counter[i], pCtx->pid_first_packet[i], pCtx->pid_last_packet[i]);
		}
	}
	if(pCtx->video_progressive == 0)  {
		DBG_MSG("coded pictures = %d, video fields = %d\n", pCtx->coded_frames, pCtx->video_fields);
	}
	else  {
		DBG_MSG("coded pictures = %d, video frames = %d\n", pCtx->coded_frames, pCtx->video_fields);
	}
	temp = (int)((pCtx->last_audio_pts + pCtx->last_audio_pts_diff) - (pCtx->last_video_pts + pCtx->last_video_pts_diff));
	DBG_MSG("Ending audio to video PTS difference = %d ticks, %f milliseconds\n", temp, (double)temp / 90.0);
	fclose(fp);
	if(pCtx->parse_only == FALSE)  {
		fclose(pCtx->fpoutvideo);
		fclose(pCtx->fpoutaudio);
	}

	demuxDelete(pComp);
	return 0;
}
#endif