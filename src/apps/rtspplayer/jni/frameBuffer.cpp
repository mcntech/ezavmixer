
#include "frameBuffer.h"
#include "jEventHandler.h"

#define DBG_TRACE_HERE 	MIDSLOG("", "%s:%s:%d\n",__FILE__, __FUNCTION__, __LINE__);

static void DumpHex(char *pDat, int nSize)
{
	char szBbuff[64 * 8] = {0};
	char szTmp[8];
	for(int i=0; i < nSize && nSize < 62; i++) {
		sprintf(szTmp,"0x%02x ", pDat[i]);
		strcat(szBbuff, szTmp);
	}

	MIDSLOG("", "Dump:%s\n", szBbuff);
}
///////////////////////////////////////////////////////////////////////////////
// CBufferSrcManager

static int isframeStart(unsigned char *pData, int numBytes)
{
	if(numBytes) {
		// Simple check for start code
		if(pData[0] == 0x00 && pData[1] == 0x00 && pData[2] == 0x00)
			return 1;
	}
	return 0;
}

CBufferSrcManagerV2::CBufferSrcManagerV2(void){
    sourceBuffer = NULL;
    sourceBufferSize = 0;
    WriteOffset = 0;
    ReadOffset = 0;
    m_bytesRead = 0;
}

CBufferSrcManagerV2::~CBufferSrcManagerV2(void){
	Stop();
	if(sourceBuffer != NULL)
		free(sourceBuffer);
    pthread_cond_destroy(&m_dataAvailCond);
    pthread_mutex_destroy(&m_dataMutex);
    pthread_cond_destroy(&m_spaceAvailCond);
}

ESTATUS CBufferSrcManagerV2::init(int size){
	if(sourceBuffer != NULL){
		free(sourceBuffer);
		sourceBuffer = NULL;
		sourceBufferSize = 0;
	}
	sourceBuffer = (unsigned char *)malloc(size);
    if (sourceBuffer == NULL)
        return E_NOSPACE;

	sourceBufferSize = size;

    pthread_mutex_init(&m_dataMutex, 0);
    pthread_cond_init(&m_spaceAvailCond, 0);
    pthread_cond_init(&m_dataAvailCond, 0);

    return E_OK;
}

void CBufferSrcManagerV2::resetBufferOffset(){
	WriteOffset = ReadOffset; //thread safe
}

void CBufferSrcManagerV2::Start(){
	mRunning = true;
}

void CBufferSrcManagerV2::Stop(){

	mRunning = false;
	// Wake up threads
    pthread_mutex_lock(&m_dataMutex);
    pthread_cond_broadcast(&m_dataAvailCond);
    pthread_cond_broadcast(&m_spaceAvailCond);
    pthread_mutex_unlock(&m_dataMutex);
}


int CBufferSrcManagerV2::GetNextPacket(void * outputbuffer,int readsize)
{
	if (readsize > sourceBufferSize) {
		MIDSLOG("", "GetNextPacket:Error readsize(%d) > sourceBufferSize(readsize > sourceBufferSize) \n", readsize, sourceBufferSize);
        return - 1;
	}

	if (ReadOffset + readsize > sourceBufferSize){

		int overflow =  (ReadOffset + readsize) - sourceBufferSize;

		int AjustedReadSize  = readsize-overflow;
		memcpy(outputbuffer,sourceBuffer + ReadOffset,AjustedReadSize);

		memcpy((uint8_t*)outputbuffer + AjustedReadSize ,sourceBuffer,overflow);

		ReadOffset = overflow;
	}else{
		memcpy(outputbuffer,sourceBuffer + ReadOffset,readsize);
		ReadOffset = ReadOffset + readsize;
	}
	m_bytesRead+= readsize;

	return readsize;
}


ESTATUS CBufferSrcManagerV2::Render(unsigned char * samples,int size){
	if(sourceBuffer == NULL)
		return E_NOTINITIALIZED;

	int overflow = 0;

	if ((WriteOffset + size) > sourceBufferSize){
		overflow =  (WriteOffset + size) - sourceBufferSize;
		int adjustedsize = size - overflow; //adjust to fit the buffer
		memcpy(sourceBuffer + WriteOffset,samples,adjustedsize);

		//wrap to start of buffer
		memcpy(sourceBuffer,samples + adjustedsize,overflow);//get remaining overflow
		WriteOffset = overflow;
	}else{
		memcpy(sourceBuffer + WriteOffset,samples,size);
		WriteOffset = WriteOffset + size;
	}
	return E_OK;
}

int CAvBufferSrcManager::WriteFrame(unsigned char * samples,int size, long lPts, int nFlags, int nCodec)
{
    pthread_mutex_lock(&m_dataMutex);

	while((freeSpace() < size || (m_nWr + 1) % m_nMax == m_nRd) && mRunning){
		 pthread_cond_wait(&m_spaceAvailCond, &m_dataMutex);
	}

	if(mRunning) {
		//if(!isframeStart(samples, size)){
		//	MIDSLOG("", "CAvBufferSrcManager::WriteFrame Corrupted Frame!!!\n");
		//}

		//DumpHex((char *)samples, 16);
		Render(samples,size);
		AVCAP_FRAME_T *avFrame = &m_avcapFrameList[m_nWr];
		// avFrame->data =
		avFrame->size = size;
		avFrame->pts = lPts;
		avFrame->codec = nCodec;
		m_nWr = (m_nWr + 1) % m_nMax;
		pthread_cond_signal(&m_dataAvailCond);
	} else {
		MIDSLOG("", "WriteFrame:skip frame copy due to mRunning==0\n");
	}
	//MIDSLOG("", "WriteFrame:CrntFrame size =%d bufferdet m_nRd=%d m_nWr=%d freeSpace=%d usedSpace=%d\n", size, m_nRd, m_nWr, freeSpace(), usedSpace());
	pthread_mutex_unlock(&m_dataMutex);

	return 0;
}

int CAvBufferSrcManager::WriteFrameBegin(unsigned char * samples,int size, int64_t llPts, int nFlags, int nCodec)
{
    pthread_mutex_lock(&m_dataMutex);

	while((freeSpace() < size || (m_nWr + 1) % m_nMax == m_nRd) && mRunning){
		 pthread_cond_wait(&m_spaceAvailCond, &m_dataMutex);
	}
	if(mRunning) {
		//if(!isframeStart(samples, size)){
		//	MIDSLOG("", "CAvBufferSrcManager::WriteFrameBegin Corrupted Frame!!!\n");
		//} else {
		//	//MIDSLOG("", "CAvBufferSrcManager::WriteFrameBegin Good Frame!!!\n");
		//}
		m_crntAvFrame = &m_avcapFrameList[m_nWr];
		m_crntAvFrame->size = 0;
		m_crntAvFrame->pts = llPts;
		m_crntAvFrame->codec = nCodec;

		if(samples && size > 0) {
			//DumpHex((char *)samples, 16);
			Render(samples,size);
			m_crntAvFrame->size = size;
		}
	} else {
		MIDSLOG("", "WriteFrameBegin:Codec=%d skip frame copy due to mRunning==0\n", nCodec);
	}
	pthread_mutex_unlock(&m_dataMutex);

	//MIDSLOG("", "WriteFrameBegin: accum_size =%d size=%d bufferdet m_nRd=%d m_nWr=%d freeSpace=%d usedSpace=%d\n", m_crntAvFrame->size, size, m_nRd, m_nWr, freeSpace(), usedSpace());
	return 0;

}

int CAvBufferSrcManager::WriteFrameMiddle(unsigned char * samples,int size, int nCodec)
{
	const int nWaitTimeUs = 10 * 1000;
	if (m_crntAvFrame == NULL) {
		MIDSLOG("", "WriteFrameMiddle:Error:Codec=%dIgnoring the request\n", nCodec);
		return 0;
	}
    pthread_mutex_lock(&m_dataMutex);

	while((freeSpace() < size || (m_nWr + 1) % m_nMax == m_nRd) && mRunning){
		 pthread_cond_wait(&m_spaceAvailCond, &m_dataMutex);
	}
	if(mRunning) {
		if(samples && size > 0) {
			//DumpHex((char *)samples, 16);
			Render(samples,size);
			m_crntAvFrame->size += size;
		}
	} else {
		MIDSLOG("", "WriteFrameMiddle:skip frame copy due to mRunning==0\n");
	}
	pthread_mutex_unlock(&m_dataMutex);
	//MIDSLOG("", "WriteFrameMiddle: accum_size =%d size=%d bufferdet m_nRd=%d m_nWr=%d freeSpace=%d usedSpace=%d\n", m_crntAvFrame->size, size, m_nRd, m_nWr, freeSpace(), usedSpace());
	return 0;

}

int CAvBufferSrcManager::WriteFrameEnd(unsigned char * samples,int size, int nCodec)
{
	if (m_crntAvFrame == NULL) {
		MIDSLOG("", "WriteFrameEnd:Codec=%d Ignoring  the request\n", nCodec);
		return 0;
	}
    pthread_mutex_lock(&m_dataMutex);

	while((freeSpace() < size || (m_nWr + 1) % m_nMax == m_nRd) && mRunning){
		 pthread_cond_wait(&m_spaceAvailCond, &m_dataMutex);
	}
	if(mRunning) {
		if(samples && size > 0) {
			//DumpHex((char *)samples, 16);
			Render(samples,size);
			m_crntAvFrame->size += size;
		}

		// Advance the codec
		m_nWr = (m_nWr + 1) % m_nMax;
		//MIDSLOG("", "WriteFrameEnd: size =%d pts=%lld bufferdet m_nRd=%d m_nWr=%d freeSpace=%d usedSpace=%d\n", m_crntAvFrame->size, m_crntAvFrame->pts, m_nRd, m_nWr, freeSpace(), usedSpace());
		m_crntAvFrame = NULL;
		pthread_cond_signal(&m_dataAvailCond);
	} else {
		MIDSLOG("", "WriteFrameEnd:skip frame copy due to mRunning==0\n");
	}

	pthread_mutex_unlock(&m_dataMutex);
	return 0;

}

int CAvBufferSrcManager::GetFrame(AVCAP_FRAME_T *avFrame)
{
	int nBytesRead = 0;
	avFrame->size = 0;

    pthread_mutex_lock(&m_dataMutex);

	while(m_nRd == m_nWr && mRunning){
		 pthread_cond_wait(&m_dataAvailCond, &m_dataMutex);
	}
	if(mRunning){
		AVCAP_FRAME_T *tmpavFrame = &m_avcapFrameList[m_nRd];
		//MIDSLOG("", "GetFrame:CrntFrame size =%d pts=%lld bufferdet m_nRd=%d m_nWr=%d freeSpace=%d usedSpace=%d\n", tmpavFrame->size, tmpavFrame->pts, m_nRd, m_nWr, freeSpace(), usedSpace());
		if(GetNextPacket(avFrame->data, tmpavFrame->size) > 0) {
			nBytesRead = avFrame->size = tmpavFrame->size;
			avFrame->pts = tmpavFrame->pts;
			avFrame->codec = tmpavFrame->codec;
			//assert(avFrame->size = size == nBytesRead);
			m_nRd = (m_nRd + 1) % m_nMax;
			//if(!isframeStart(avFrame->data, avFrame->size)){
			//	MIDSLOG("", "CAvBufferSrcManager::GetFrame Corrupted Frame!!!\n");
			//}  else {
			//	//MIDSLOG("", "CAvBufferSrcManager::GetFrame Good Frame!!!\n");
			//}
			//DumpHex((char *)avFrame->data, 16);
		} else {
			MIDSLOG("", "GetAvFrame:Error  bufferdet m_nRd=%d m_nWr=%d freeSpace=%d usedSpace=%d frame_size=%d\n",  m_nRd, m_nWr, freeSpace(), usedSpace(), tmpavFrame->size);
			//usleep(100 * 1000);
		}
		pthread_cond_signal(&m_spaceAvailCond);
	} else {
		MIDSLOG("", "GetFrame:skip frame copy due to mRunning==0\n");
	}

	pthread_mutex_unlock(&m_dataMutex);
	return nBytesRead;
}
