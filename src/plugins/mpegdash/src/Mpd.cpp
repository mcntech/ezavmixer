#ifdef WIN32
//#include <Windows.h>
#include <winsock2.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#endif
#include <time.h>
#include <ctype.h>

#ifdef WIN32
#define snprintf _snprintf
#endif

#include <JdDbg.h>

static int  modDbgLevel = CJdDbg::LVL_ERR;

#include "Mpd.h"
#include "JdMpdDefs.h"

#define DEFAULT_UPDATE_PERIOD      (12 * 60 * 60 * 1000); // 12 Hours
#define DEFAULT_TIME_SHIFT_BUFFER  (12 * 1000);          // 12 seconds
#define DEFAULT_SEGMENT_DURATION   (4 * 1000)            // 4 seconds;
#define DEFAULT_MIN_BUFFER         (4 * 1000)

#define MAX_NAME_SIZE             256
#define MAX_TIME_STRING           256
#define MAX_SEGMENT_FILE_NAME     256

void FormatTime(int nDurationMs,char *szString, int nLen)
{
	int h, m, s, ms;
	h = nDurationMs / 3600000;
	nDurationMs = nDurationMs % 3600000;
	m = nDurationMs / 60000;
	nDurationMs = nDurationMs % 60000;
	s = nDurationMs / 1000;
	ms = nDurationMs % 1000;
	snprintf(szString, nLen, "PT%02dH%02dM%02d.%03dS", h, m, s, ms);
}

static unsigned long ParseDuration(char *pszDuration) 
{
	unsigned long i;
	if (pszDuration == NULL) return 0;
	i = 0;
	// Skip Spaces
	while (1) {
		if (pszDuration[i] == ' ') i++;
		else if (pszDuration[i] == 0) {
			return 0;
		} else {
			break;
		}
	}
	if (pszDuration[i] == 'P') {
		if (pszDuration[i+1] == 0) {
			return 0;
		} else if (pszDuration[i+1] != 'T') {
			return 0;
		} else {
			char *sep1, *sep2;
			unsigned long h, m;
			double s;
			h = m = 0;
			s = 0;
			if (NULL != (sep1 = strchr(pszDuration+i+2, 'H'))) {
				*sep1 = 0;
				h = atoi(pszDuration+i+2);
				*sep1 = 'H';
				sep1++;
			} else {
				sep1 = pszDuration+i+2;
			}
			if (NULL != (sep2 = strchr(sep1, 'M'))) {
				*sep2 = 0;
				m = atoi(sep1);
				*sep2 = 'M';
				sep2++;
			} else {
				sep2 = sep1;
			}
			if (NULL != (sep1 = strchr(sep2, 'S'))) {
				*sep1 = 0;
				s = atof(sep2);
				*sep1 = 'S';
			}
			return (unsigned long)((h*3600+m*60+s)*1000);
		}
	} else {
		return 0;
	}
}

CMpdSegmentBase::CMpdSegmentBase(CMpdRepresentation *pParent, TiXmlNode *pNode)
{
	m_pNode = NULL;
	m_nDuration = 0;
	//m_pSegmentTimeline = NULL;
	m_pszBitstreamSwitching = NULL;
	m_SpegmentInitializationUrl = NULL;
	m_pParent = pParent;
	if(pNode) {
		m_pNode = pNode;
	} else if (m_pParent && m_pParent->m_pNode) {
		m_pNode = new TiXmlElement(ELEMENT_SegmentTemplate);
		m_pParent->m_pNode->LinkEndChild(m_pNode);
	}
}

CMpdSegmentTemplate::CMpdSegmentTemplate(CMpdAdaptaionSet *pParent, TiXmlNode *pNode)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	m_pNode = NULL;
	m_nDuration = 0;
	m_nStartNumber = -1;
	//m_pSegmentTimeline = NULL;
	m_pszBitstreamSwitching = NULL;
	m_SpegmentInitializationUrl = NULL;
	m_pParent = pParent;
	if(pNode) {
		m_pNode = pNode;
	}else if ( m_pParent && m_pParent->m_pNode) {
		m_pNode = new TiXmlElement(ELEMENT_SegmentTemplate);
		m_pParent->m_pNode->LinkEndChild(m_pNode);
	}
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
}

CMpdSegmentTemplate::~CMpdSegmentTemplate()
{
}

void CMpdSegmentTemplate::Setup(int nStartNumber, int nDurationMs, const char *pszTemplate)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	if(m_pNode) {
		TiXmlElement *pElemUrl = m_pNode->ToElement();
		pElemUrl->SetAttribute(ATTRIB_NAME_SEGTMPLT_media, pszTemplate);
		pElemUrl->SetAttribute(ATTRIB_NAME_SEGTMPLT_duration, nDurationMs / 1000);
		pElemUrl->SetAttribute(ATTRIB_NAME_SEGTMPLT_startNumber, nStartNumber);
		m_nStartNumber = nStartNumber;
		m_nDuration = nDurationMs;
	}
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
}

void CMpdSegmentTemplate::SetInitializationSegment(std::string &InitializationUrl)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	if(InitializationUrl.size())	{
		std::string Url = InitializationUrl;
		CMpdInitializationUrl *pUrl = new CMpdInitializationUrl();
		TiXmlElement *pElemUrl = new TiXmlElement(ELEMENT_SegmentInitialization);
		pElemUrl->SetAttribute(ATTRIB_NAME_SEGMENT_INIT_sourceURL, InitializationUrl.c_str());
		m_pNode->LinkEndChild(pElemUrl);
	}
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
}

CMpdSegmentList::CMpdSegmentList(CMpdRepresentation *pParent, TiXmlNode *pNode)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	m_pNode = NULL;
	m_pXlinkNode = NULL;
	m_nDuration = 0;
	m_nStartNumber = -1;
	//m_pSegmentTimeline = NULL;
	m_pszBitstreamSwitching = NULL;
	m_SpegmentInitializationUrl = NULL;
	m_pParent = pParent;

	if(pNode) {
		m_pNode = pNode;
	} else 	if (m_pParent && m_pParent->m_pNode) {
		m_pNode = new TiXmlElement(ELEMENT_SegmentList);
		m_pParent->m_pNode->LinkEndChild(m_pNode);
	}
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
}

void CMpdSegmentList::Update(int nCrntTIme, int fXlink, std::list<std::string> *plistUrl, int nStatNumber)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	// Clear current list
	TiXmlNode  *pElemNode = NULL;
	if(fXlink) {
		if(m_pXlinkNode == NULL)
			m_pXlinkNode = new TiXmlElement(ELEMENT_SegmentList);
		pElemNode = m_pXlinkNode;
	} else {
		pElemNode = m_pNode;
	}
	if(pElemNode) {
		for(SEGMETN_URL_LIST_T::iterator it = m_listSegmentUrl.begin(); it != m_listSegmentUrl.end(); it++){
			CMpdSegmentUrl *pSegmentUrl = *it;
			TiXmlNode *pNodeUrl = pSegmentUrl->m_pNode;
			pElemNode->RemoveChild(pNodeUrl);
			delete pSegmentUrl;
		}
		m_listSegmentUrl.clear();
		for(std::list<std::string>::iterator it = plistUrl->begin(); it != plistUrl->end(); it++){
			std::string Url = *it;
			CMpdSegmentUrl *pSegmentUrl = new CMpdSegmentUrl();
			TiXmlElement *pElemUrl = new TiXmlElement(ELEMENT_SegmentURL);
			pElemUrl->SetAttribute(ATTRIB_NAME_SEGMENTURL_media ,Url.c_str());
			pSegmentUrl->m_pNode = pElemUrl;

			pElemNode->LinkEndChild(pElemUrl);
			m_listSegmentUrl.push_back(pSegmentUrl);
		}
		TiXmlElement *pElemUrl = pElemNode->ToElement();
		//pElemUrl->SetAttribute(ATTRIB_NAME_SEGTMPLT_startNumber, nStatNumber);
		pElemUrl->SetAttribute(ATTRIB_NAME_SEGMENTLIST_presentationTimeOffset, nCrntTIme);
		pElemUrl->SetAttribute(ATTRIB_NAME_SEGMENTLIST_timescale, 1000);
	}
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
}

void CMpdSegmentList::SaveXlinkList(const char *szFileName)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	if(GetXlink() != NULL && m_pXlinkNode != NULL) {
		FILE *fdSegmentList = fopen(szFileName, "w");
		if(fdSegmentList) {
			m_pXlinkNode->Print(fdSegmentList, 2);
			fclose(fdSegmentList);
		}
	}
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
}

void CMpdSegmentList::SetInitializationSegment(std::string &InitializationUrl)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	if(InitializationUrl.size())	{
		std::string Url = InitializationUrl;
		CMpdInitializationUrl *pUrl = new CMpdInitializationUrl();
		TiXmlElement *pElemUrl = new TiXmlElement(ELEMENT_SegmentInitialization);
		pElemUrl->SetAttribute(ATTRIB_NAME_SEGMENT_INIT_sourceURL, InitializationUrl.c_str());
		m_pNode->LinkEndChild(pElemUrl);
	}
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
}

const char *CMpdSegmentList::GetInitializationSegment()
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	const char *pszUrl = NULL;
	TiXmlNode *pNode = m_pNode->FirstChild(ELEMENT_SegmentInitialization);
	if(pNode) {
		TiXmlElement *pElemUrl = pNode->ToElement();
		pszUrl = pElemUrl->Attribute(ATTRIB_NAME_SEGMENT_INIT_sourceURL);
	}
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return pszUrl;
}

const char *CMpdSegmentList::GetXlink()
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	const char *pszXlinkUrl = NULL;
	TiXmlElement *pElem = m_pNode->ToElement();
	pszXlinkUrl = pElem->Attribute(ATTRIB_NAME_XLINK_HREF);
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return pszXlinkUrl;
}

CMpdSegmentList::~CMpdSegmentList()
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	for(SEGMETN_URL_LIST_T::iterator it = m_listSegmentUrl.begin(); it != m_listSegmentUrl.end(); it++){
		CMpdSegmentUrl *pUrl = *it;
		// TODO delete
	}
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
}

CMpdRepresentation::CMpdRepresentation(CMpdAdaptaionSet *pParent)
{
	m_pParent = pParent;
}

CMpdRepresentation::CMpdRepresentation(CMpdAdaptaionSet *pParent, std::string szId, TiXmlNode *pNode, int fSegmentTmplate)
{
	m_pParent = pParent;
	m_szId = szId;
	m_pNode = pNode;
	TiXmlElement *pElem = m_pNode->ToElement();
	m_MimeType = MIME_MP4; // TODO get it from arg
	m_VidCodecType = VID_CODEC_42E01E; // TODO: get it
	m_nWidth = 1280;
	m_nHeight = 720;
	m_nBandwidth = 1000000;

	if(!fSegmentTmplate) {
		TiXmlElement *pSegElem = new TiXmlElement( ELEMENT_SegmentList );
		m_SegmentType = CMpdRepresentation::TYPE_SEGMENT_LIST;

		m_pNode->InsertEndChild(*pSegElem);
		TiXmlNode *pSegNode = m_pNode->FirstChild();
		m_pSegmentList = new CMpdSegmentList(this, pSegNode);
	} else {
		// todo pRepresentation->m_SegmentType = CMpdRepresentation::TYPE_SEGMENT_TEMPLATE;
	}
	if(m_MimeType == MIME_MP4) {
		pElem->SetAttribute(ATTRIB_NAME_ADAPTSET_mimeType, ATTRIB_VAL_ADAPTSET_MIMETYPE_MP4);
	} else  {
		pElem->SetAttribute(ATTRIB_NAME_ADAPTSET_mimeType, ATTRIB_VAL_ADAPTSET_MIMETYPE_MP2T);
	}
	if(m_VidCodecType == VID_CODEC_42E01E) {
		pElem->SetAttribute(ATTRIB_NAME_ADAPTSET_codecs, ATTRIB_VAL_ADAPTSET_codecs_AVC1_42E01E);
	}
	char szTmp[32] = {0};
	sprintf(szTmp,"%d", m_nWidth);
	pElem->SetAttribute(ATTRIB_NAME_REPRESENTATION_width, szTmp);
	sprintf(szTmp,"%d", m_nHeight);
	pElem->SetAttribute(ATTRIB_NAME_REPRESENTATION_height, szTmp);
	sprintf(szTmp,"%d", m_nBandwidth);
	pElem->SetAttribute(ATTRIB_NAME_REPRESENTATION_bandwidth, szTmp);

}

const char *CMpdRepresentation::GetId()
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	const char *pszId;
	TiXmlElement *pElem = m_pNode->ToElement();
	pszId = pElem->Attribute(ATTRIB_NAME_REPRESENTATION_id);
	if(pszId == NULL) {
		// TODO: Generate ID
	}
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return pszId;
}


CMpdRepresentation::MIME_TYPE CMpdRepresentation::GetMimeType()
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	MIME_TYPE nMimeType = MIME_MP2T;
	TiXmlElement *pElem = m_pNode->ToElement();
	const char *pszMimetype = pElem->Attribute(ATTRIB_NAME_REPRESENTATION_mimetype);
	if(pszMimetype) {
		if(strcmp(pszMimetype,ATTRIB_VAL_REPRESENTATION_MIMETYPE_video_mp4) == 0) {
			nMimeType = MIME_MP4;
		} else {
			nMimeType = MIME_MP2T;
		}
	}
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return nMimeType;
}

#define DEFUALT_MOOF_LEN  500
#define MAX_MOOF_LEN      20000
#define MIN_MOOF_LEN      500
int CMpdRepresentation::GetCutomAttribMoofLength()
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	int nMoofLen = DEFUALT_MOOF_LEN;
	TiXmlElement *pElem = m_pNode->ToElement();
	const char *pszVal = pElem->Attribute(ATTRIB_NAME_MCN_REPRESENTATION_moofLength);
	if(pszVal) {
		nMoofLen = atoi(pszVal);
		if(nMoofLen > MAX_MOOF_LEN)
			nMoofLen = MAX_MOOF_LEN;
		else if (nMoofLen < MIN_MOOF_LEN)
			nMoofLen = MIN_MOOF_LEN;
	}
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return nMoofLen;
}

void CMpdRepresentation::UpdateSegmentList(int nStartTime, int nSegmentDuration, int nStartNum, std::list<std::string> *plistUrl)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	const char *pszXlinkListFile = m_pSegmentList->GetXlink();

	if(pszXlinkListFile != NULL) {
		char szSegmentListFileName[MAX_SEGMENT_FILE_NAME];
		CMpdRoot *pMpdRoot = GetMpd();
		const char *pszFolder = pMpdRoot->GetCutomCfgFolder();
		const char *szMpdLive = m_pParent->GetBaseURL();
		sprintf(szSegmentListFileName, "%s%s%s", pszFolder, szMpdLive, pszXlinkListFile);
		m_pSegmentList->Update(nStartTime, 1, plistUrl, nStartNum);
		m_pSegmentList->SaveXlinkList(szSegmentListFileName);
	} else {
		m_pSegmentList->Update(nStartTime, 0, plistUrl, nStartNum);
		m_pParent->CallbackChildUpdate(this);
	}
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
}

void CMpdRepresentation::SetInitializationSegment(std::string &Url)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	m_pSegmentList->SetInitializationSegment(Url);
	m_pParent->CallbackChildUpdate(this);
}

const char *CMpdRepresentation::GetInitializationSegment()
{
	return m_pSegmentList->GetInitializationSegment();
}


void CMpdRepresentation::SetStreamParams(int nWith, int nHeight, int nFrameRate, int nBandwidth)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	TiXmlElement *pElem = m_pNode->ToElement();
	pElem->SetAttribute(ATTRIB_NAME_REPRESENTATION_width, nWith);
	pElem->SetAttribute(ATTRIB_NAME_REPRESENTATION_height, nHeight);
	pElem->SetAttribute(ATTRIB_NAME_REPRESENTATION_rameRate, nFrameRate);
	pElem->SetAttribute(ATTRIB_NAME_REPRESENTATION_bandwidth, nBandwidth);
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
}

CMpdRoot *CMpdRepresentation::GetMpd()
{
	return m_pParent->GetMpd();
}

CMpdAdaptaionSet::CMpdAdaptaionSet(CMpdPeriod *pParent, TiXmlNode* pNode)
{
	m_pParent = pParent;
	m_pNode = pNode;
	m_pSegmentTemplate = NULL;

	TiXmlElement *pElem = m_pNode->ToElement();
	pElem->SetAttribute(ATTRIB_NAME_ADAPTSET_mimeType, ATTRIB_VAL_ADAPTSET_MIMETYPE_MP4);
	pElem->SetAttribute(ATTRIB_NAME_ADAPTSET_codecs, ATTRIB_VAL_ADAPTSET_codecs_AVC1_42E01E);
	pElem->SetAttribute(ATTRIB_NAME_ADAPTSET_segmentAlignment, "true");
	pElem->SetAttribute(ATTRIB_NAME_ADAPTSET_subsegmentAlignment, "true");
	pElem->SetAttribute(ATTRIB_NAME_ADAPTSET_bitstreamSwitching, "false");
	pElem->SetAttribute(ATTRIB_NAME_ADAPTSET_startWithSAP, "2");
	pElem->SetAttribute(ATTRIB_NAME_ADAPTSET_subsegmentStartsWithSAP, "2");
}

int CMpdAdaptaionSet::CallbackChildUpdate(CMpdRepresentation *pChild)
{
	return m_pParent->CallbackChildUpdate(this);
}

int CMpdAdaptaionSet::GetMimeType()
{
	int nMimeType = MPD_MUX_TYPE_VIDEO_MP4;
	TiXmlElement *pElem = m_pNode->ToElement();
	const char *pszMimetype = pElem->Attribute(ATTRIB_NAME_ADAPTSET_mimeType);
	if(pszMimetype) {
		if(pszMimetype && strcmp(pszMimetype,ATTRIB_VAL_REPRESENTATION_MIMETYPE_video_mp4) == 0) {
			nMimeType = MPD_MUX_TYPE_VIDEO_MP4;
		} else {
			nMimeType = MPD_MUX_TYPE_TS;
		}
	}
	return nMimeType;

}

CMpdAdaptaionSet::~CMpdAdaptaionSet()
{
	for (std::vector<CMpdRepresentation *>::iterator it = m_listRepresentations.begin(); it !=  m_listRepresentations.end(); it++) {
		delete *it;
	}
	m_listRepresentations.clear();
}

void CMpdAdaptaionSet::SetupTemplate(int nStartNum, int nSegmentDuration, const char *pszTemplate)
{
	if(m_pSegmentTemplate) {
		m_pSegmentTemplate->Setup(nStartNum, nSegmentDuration, pszTemplate);
	}
}

void CMpdAdaptaionSet::GetTemplate(int *pnStartNum, int *pnSegmentDuration, const char *pszTemplate)
{

}

int CMpdAdaptaionSet::IsSegmentTemplate()
{
	return (m_pSegmentTemplate != NULL);
}

const char *CMpdAdaptaionSet::GetBaseURL()
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	const char *pszBaseURL = NULL;
	TiXmlElement *pElem = m_pNode->ToElement();
	TiXmlNode* pNod = pElem->FirstChild(ELEMENT_BaseURL);
	if(pNod) {
		TiXmlElement *pElem = pNod->ToElement();
		pszBaseURL = pElem->GetText();
	}
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return pszBaseURL;
}

const char *CMpdAdaptaionSet::GetInitializationSegmentUrl()
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	const char *pszURL = NULL;
	TiXmlElement *pElem = m_pNode->ToElement();
	TiXmlNode* pNod = pElem->FirstChild(ELEMENT_SegmentTemplate);
	if(pNod) {
		TiXmlElement *pElem = pNod->ToElement();
		pszURL = pElem->Attribute(ATTRIB_NAME_SEGTMPLT_initialization);
	}
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return pszURL;
}

CMpdRoot *CMpdAdaptaionSet::GetMpd() 	
{
	return m_pParent->GetMpd();
}

int CMpdAdaptaionSet::CreateRepresentation(std::string szId, int fSegmentTmplate)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	TiXmlNode   *pRepNode;
	TiXmlElement *pRepElem = new TiXmlElement( ELEMENT_Representation );

	//pRepresentation->m_inputSwitch = szSwitchId;

	m_pNode->InsertEndChild(*pRepElem);
	pRepNode = m_pNode->FirstChild(); // TODO add support for multiple representation

	CMpdRepresentation *pRepresentation = new  CMpdRepresentation(this, szId, pRepNode, fSegmentTmplate);
	m_listRepresentations.push_back(pRepresentation);

	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return 0;
}

CMpdRepresentation *CMpdAdaptaionSet::FindRepresentation(std::string szId)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	CMpdRepresentation *pRep = NULL;
	for (std::vector<CMpdRepresentation *>::iterator it = m_listRepresentations.begin(); it !=  m_listRepresentations.end(); it++) {
		pRep = *it;
		if(pRep->m_szId == szId){
			return pRep;
		}
	}
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return NULL;
}

CMpdPeriod::CMpdPeriod(CMpdRoot *pParent)
{
	m_pParent = pParent;
}

CMpdPeriod::CMpdPeriod(CMpdRoot *pParent, std::string strId, TiXmlNode *pNode, int nStartTime)
{
	m_pParent = pParent;
	m_pNode = pNode;
	m_szId = strId;
	TiXmlElement *pElem = m_pNode->ToElement();
	pElem->SetAttribute(ATTRIB_NAME_PERIOD_start, nStartTime);
}

CMpdPeriod::~CMpdPeriod()
{
	for (std::vector<CMpdAdaptaionSet *>::iterator it = m_listAdaptionSets.begin(); it !=  m_listAdaptionSets.end(); it++) {
		delete *it;
	}
	m_listAdaptionSets.clear();
}

int CMpdPeriod::CallbackChildUpdate(CMpdAdaptaionSet *pChild)
{
	return m_pParent->CallbackChildUpdate(this);
}

CMpdRoot *CMpdPeriod::GetMpd()
{
	return m_pParent;
}

CMpdAdaptaionSet *CMpdPeriod::CreateAdaptationSet(std::string szId)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	TiXmlElement *pAdaptElem;
	TiXmlNode* pAdaptNode = NULL;

	pAdaptElem = new TiXmlElement( ELEMENT_AdaptationSet );

	//			pSegNode = pAdaptElem->FirstChild(ELEMENT_SegmentTemplate);
	//			if(pSegNode) {
	//				CMpdSegmentTemplate *pSegmentTemplate =  new CMpdSegmentTemplate(pAdaptaionSet, pSegNode);
	//				fSegmentTmplate = 1;
	//				pAdaptaionSet->m_pSegmentTemplate = pSegmentTemplate;
	//			}

	//while(pRepNode = pAdaptNode->IterateChildren(ELEMENT_Representation, pRepNode))

	m_pNode->InsertEndChild(*pAdaptElem);
	pAdaptNode = m_pNode->FirstChild(); // TODO: support for multiple adapt

	CMpdAdaptaionSet *pAdaptaionSet = new  CMpdAdaptaionSet(this, pAdaptNode);
	pAdaptaionSet->m_szId = szId;
	m_listAdaptionSets.push_back(pAdaptaionSet);

	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
}

CMpdAdaptaionSet *CMpdPeriod::FindAdaptationSet(std::string szId)
{
	CMpdAdaptaionSet *pAdapt;
	for (std::vector<CMpdAdaptaionSet *>::iterator it = m_listAdaptionSets.begin(); it !=  m_listAdaptionSets.end(); it++) {
		pAdapt = *it;
		if(pAdapt->m_szId == szId)
			return pAdapt;
	}
	return NULL;
}

unsigned int osalGetSystemTime()
{
#ifdef WIN32
	return GetTickCount();
#else
	struct timeval   tv;
	gettimeofday(&tv,NULL);
	int Timestamp =  tv.tv_sec * 1000 + tv.tv_usec / 1000;
	return (Timestamp);
#endif
}


CMpdPeriod *CMpdRoot::CreatePeriod(std::string szId)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	TiXmlElement *pPeriodElem;

	pPeriodElem = new TiXmlElement( ELEMENT_Period );
	m_pNode->InsertEndChild(*pPeriodElem);
	TiXmlNode   *periodNode = m_pNode->FirstChild();

	CMpdPeriod *pPeriod = new CMpdPeriod(this, szId, periodNode, 0);
	m_listPeriods.push_back(pPeriod);

	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return pPeriod;
}


CMpdRoot::CMpdRoot(const char *pszConfFile)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	char szDaration[MAX_TIME_STRING];
	const char *pszCfgVal = NULL;
	m_pDoc = new TiXmlDocument( pszConfFile );
	bool loadOkay = m_pDoc->LoadFile();

	TiXmlNode* pPeriodNode = NULL;
	TiXmlNode* pAdaptNode = NULL;
	TiXmlNode* pRepNode = NULL;
	TiXmlNode* pSegNode = NULL;
	TiXmlElement *pPeriodElem;
	TiXmlElement *pElem;
	TiXmlElement *pRepElem;
	TiXmlElement *pSegElem;
	TiXmlElement *pAdaptElem;

	m_pNode = m_pDoc->FirstChild( ELEMENT_MPD );
	if(m_pNode == NULL)
		goto Exit;

	pElem = m_pNode->ToElement();
	pElem->SetAttribute(ATTRIB_NAME_MPD_profiles, ATTRIB_VAL_MPD_PROFILE_isoff_live);
	pElem->SetAttribute(ATTRIB_NAME_MPD_XMLNS_XSI, ATTRIB_VAL_MPD_XMLNS_XSI);
	pElem->SetAttribute(ATTRIB_NAME_MPD_XMLNS, ATTRIB_NAME_MPD_XMLNS);
	pElem->SetAttribute(ATTRIB_NAME_MPD_XSI_SCHEMA_LOCN, ATTRIB_VAL_MPD_XSI_SCHEMA_LOCN);


	if(1/*TODO: Check if dynamic*/)	{
		struct tm ast_time;
		time_t    time_now;
		char availability_start_time[MAX_TIME_STRING];
		time ( &time_now );
		time_now += GetCustomAvailabilityDelay() / 1000;
		ast_time = *gmtime(&time_now);
		strftime(availability_start_time, 64, "%Y-%m-%dT%H:%M:%S", &ast_time);
		pElem->SetAttribute(ATTRIB_NAME_MPD_availabilityStartTime, availability_start_time);
	} else {
		// TODO: Make v_dur, a_dur externally set
		int v_dur = 0;
		int a_dur = 0;
		int dur = v_dur > a_dur ? v_dur : a_dur;
		FormatTime(dur, szDaration, MAX_TIME_STRING);
		pElem->SetAttribute(ATTRIB_NAME_MPD_mediaPresentationDuration, szDaration);
	}
	
	pszCfgVal = pElem->Attribute(ATTRIB_NAME_MPD_minimumUpdatePeriod);
	if(pszCfgVal == NULL){
		int nMinimumUpdatePeriod = DEFAULT_UPDATE_PERIOD;//3600 * 1000; // 1 Hour
		FormatTime(nMinimumUpdatePeriod, szDaration, MAX_TIME_STRING);
		pElem->SetAttribute(ATTRIB_NAME_MPD_minimumUpdatePeriod, szDaration);
	} else {
		// TODO Get the value
	}

	pszCfgVal = pElem->Attribute(ATTRIB_NAME_MPD_minBufferTime);
	if(pszCfgVal == NULL){
		int nTimeShiftBufferDepth = DEFAULT_MIN_BUFFER;
		FormatTime(nTimeShiftBufferDepth, szDaration, MAX_TIME_STRING);
		pElem->SetAttribute(ATTRIB_NAME_MPD_minBufferTime, szDaration);
	}

	pszCfgVal = pElem->Attribute(ATTRIB_NAME_MPD_timeShiftBufferDepth);
	if(pszCfgVal == NULL){
		int nTimeShiftBufferDepth = DEFAULT_TIME_SHIFT_BUFFER; //12 * 1000;
		FormatTime(nTimeShiftBufferDepth, szDaration, MAX_TIME_STRING);
		pElem->SetAttribute(ATTRIB_NAME_MPD_timeShiftBufferDepth, szDaration);
	}

	pszCfgVal = pElem->Attribute(ATTRIB_NAME_MPD_suggestedPresentationDelay);
	if(pszCfgVal == NULL) {
		int nSuggestedPresentationDelay = 4000;
		FormatTime(nSuggestedPresentationDelay, szDaration, MAX_TIME_STRING);
		pElem->SetAttribute(ATTRIB_NAME_MPD_suggestedPresentationDelay, szDaration);
	}

	pszCfgVal = pElem->Attribute(ATTRIB_MPD_maxSegmentDuration);
	if(pszCfgVal == NULL) {
		int nMaxSegmentDuration = DEFAULT_SEGMENT_DURATION; //4000;
		FormatTime(nMaxSegmentDuration, szDaration, MAX_TIME_STRING);
		pElem->SetAttribute(ATTRIB_MPD_maxSegmentDuration, szDaration);
	}

	if(0/*maxSubsegmentDuration*/) {
		int nMaxSubsegmentDuration = 4000;
		FormatTime(nMaxSubsegmentDuration, szDaration, MAX_TIME_STRING);
		pElem->SetAttribute(ATTRIB_MPD_maxSubsegmentDuration, szDaration);
	}
	while (pPeriodNode = m_pNode->IterateChildren(ELEMENT_Period, pPeriodNode)) {

		pPeriodElem = pPeriodNode->ToElement();
		CMpdPeriod *pPeriod = new CMpdPeriod(this);
		pPeriodElem->SetAttribute(ATTRIB_NAME_PERIOD_start, 0);

		while(pAdaptNode = pPeriodNode->IterateChildren(ELEMENT_AdaptationSet, pAdaptNode)) {
			int fSegmentTmplate = 0;			
			pAdaptElem = pAdaptNode->ToElement();
			
			CMpdAdaptaionSet *pAdaptaionSet = new  CMpdAdaptaionSet(pPeriod, pAdaptNode);

			pSegNode = pAdaptElem->FirstChild(ELEMENT_SegmentTemplate);
			if(pSegNode) {
				CMpdSegmentTemplate *pSegmentTemplate =  new CMpdSegmentTemplate(pAdaptaionSet, pSegNode);
				fSegmentTmplate = 1;
				pAdaptaionSet->m_pSegmentTemplate = pSegmentTemplate;
			}

			while(pRepNode = pAdaptNode->IterateChildren(ELEMENT_Representation, pRepNode)) {

				pRepElem = pRepNode->ToElement();
				
				CMpdRepresentation *pRepresentation = new  CMpdRepresentation(pAdaptaionSet);
				pRepresentation->m_pNode = pRepNode;
				if(!fSegmentTmplate) {
					pSegNode = pRepNode->FirstChild(ELEMENT_SegmentList);
					if(pSegNode) {
						pRepresentation->m_SegmentType = CMpdRepresentation::TYPE_SEGMENT_LIST;
						pRepresentation->m_pSegmentList = new CMpdSegmentList(pRepresentation, pSegNode);
						pRepresentation->m_pSegmentList->m_pNode = pSegNode;
					} else {
#if 0 // TODO add support for segment base
						pSegNode = pRepNode->FirstChild(ELEMENT_SegmentBase);
						if(pSegNode) {
							pRepresentation->m_SegmentType = CMpdRepresentation::TYPE_SEGMENT_BASE;
							pRepresentation->m_pSegmentBase = new CMpdSegmentBase(pRepresentation, NULL);
						}
#endif
					}
				} else {
					pRepresentation->m_SegmentType = CMpdRepresentation::TYPE_SEGMENT_TEMPLATE;
				}
				const char *pszVal = pRepElem->Attribute(ATTRIB_NAME_MCN_REPRESENTATION_source);
				pRepresentation->m_inputSwitch = pszVal;
				pAdaptaionSet->m_listRepresentations.push_back(pRepresentation);

				//Test
				if (0)
				{
					SetSaveFileName("C:\\TestStreams\\test.mpd");
					std::list<std::string> listUrl;
					std::string Url("www.test.com/xx1.ts");
					listUrl.push_back(Url);
					std::string  Url1("www.test.com/xx2.ts");
					listUrl.push_back(Url1);

					pRepresentation->UpdateSegmentList(0, 0,0, &listUrl);
				}
			}
			pPeriod->m_listAdaptionSets.push_back(pAdaptaionSet);
		}

		pPeriod->m_pNode = pPeriodNode;
		pPeriod->m_pParent = this;
		m_listPeriods.push_back(pPeriod);
	}

	m_nUpdateTime = 0;
    m_nUpdateInterval = 1000; // default 1 Sec
Exit:
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return;
}

CMpdRoot::~CMpdRoot()
{
	for (std::vector<CMpdPeriod *>::iterator it = m_listPeriods.begin(); it !=  m_listPeriods.end(); it++) {
		delete *it;
	}
	m_listPeriods.clear();
}

void CMpdRoot::SetSaveFileName(const char *pszFileName)
{
	m_strSaveFile = pszFileName;
}

int CMpdRoot::IsUpdateRequired()
{
	if(osalGetSystemTime() - m_nUpdateTime > m_nUpdateInterval)
		return 1;
	else
		return 0;
}

int CMpdRoot::CallbackChildUpdate(CMpdPeriod *pChild)
{
	if(IsUpdateRequired()) {
		SaveFile();
		m_nUpdateTime = osalGetSystemTime();
	}
	return 0;
}

int CMpdRoot::SaveFile()
{
	m_Mutex.Acquire();
	if(m_pDoc && !m_strSaveFile.empty()) {
		m_pDoc->SaveFile(m_strSaveFile.c_str());
		m_nUpdateTime = osalGetSystemTime();
	}
	m_Mutex.Release();
	return 0;
}

std::string CMpdRoot::GetAsXmlText()
{
	TiXmlPrinter printer;
	printer.SetIndent( "    " );

	m_pDoc->Accept( &printer );
	int len = printer.Size();
	const char *pData = printer.CStr();
	std::string xmltext = printer.CStr();
	return xmltext;
}

const char *CMpdRoot::GetBaseURL()
{
	const char *pszBaseURL = NULL;
	TiXmlElement *pElem = m_pNode->ToElement();
	TiXmlNode* pNod = pElem->FirstChild(ELEMENT_BaseURL);
	if(pNod) {
		TiXmlElement *pElem = pNod->ToElement();
		pszBaseURL = pElem->GetText();
	}
	return pszBaseURL;
}

const char *CMpdRoot::GetCutomCfgFolder()
{
	const char *pszBaseURL = NULL;
	TiXmlElement *pElem = m_pNode->ToElement();
	TiXmlNode* pNod = pElem->FirstChild(ATTRIB_NAME_MCN_MPD_Folder);
	if(pNod) {
		TiXmlElement *pElem = pNod->ToElement();
		pszBaseURL = pElem->GetText();
	}
	return pszBaseURL;
}

const char *CMpdRoot::GetAvailabilityStartTime()
{
	const char *pszTime = NULL;
	char *szTmp;
	int nTime = 0;
	TiXmlElement *pElem = m_pNode->ToElement();
	pszTime = pElem->Attribute(ATTRIB_NAME_MPD_availabilityStartTime);
	return pszTime;
}

int CMpdRoot::GetMaxSegmentDuration()
{
	const char *pszTime = NULL;
	char *szTmp;
	int nTime = 0;
	TiXmlElement *pElem = m_pNode->ToElement();
	pszTime = pElem->Attribute(ATTRIB_MPD_maxSegmentDuration);
	szTmp = strdup(pszTime);
	if(szTmp) {
		nTime = ParseDuration(szTmp);
	}
	return nTime;
}

int CMpdRoot::GetTimeShiftBuffer()
{
	const char *pszTime = NULL;
	char *szTmp;
	int nTime = 0;
	TiXmlElement *pElem = m_pNode->ToElement();
	pszTime = pElem->Attribute(ATTRIB_NAME_MPD_timeShiftBufferDepth);
	if(pszTime) {
		szTmp = strdup(pszTime);
		nTime = ParseDuration(szTmp);
	}
	return nTime;
}

int CMpdRoot::GetCustomAvailabilityDelay()
{
	const char *pszTime = NULL;
	char *szTmp;
	int nTime = 0;
	TiXmlElement *pElem = m_pNode->ToElement();
	pszTime = pElem->Attribute(ATTRIB_NAME_MPD_customAvailabilityDelay);
	if(pszTime) {
		szTmp = strdup(pszTime);
		nTime = ParseDuration(szTmp);
	}
	return nTime;
}

int CMpdRoot::IsDynamic()
{
	int res = 0;
	const char *pszVal = NULL;
	TiXmlElement *pElem = m_pNode->ToElement();
	pszVal = pElem->Attribute(ATTRIB_NAME_MPD_TYPE);
	if(pszVal) {
		if(strcmp(pszVal,ATTRIB_VAL_MPD_TYPE_DYNAMIC) == 0) {
			res = 1;
		}
	}
	return res;
}

CMpdRoot::CMpdRoot(int fDynamic, int nSegmentDurationMs)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	m_pDoc = new TiXmlDocument( );

	char szDaration[128];
	TiXmlElement *pElem  = new TiXmlElement( ELEMENT_MPD );


	pElem->SetAttribute(ATTRIB_NAME_MPD_XMLNS_XSI, ATTRIB_VAL_MPD_XMLNS_XSI);
	pElem->SetAttribute(ATTRIB_NAME_MPD_XMLNS, ATTRIB_NAME_MPD_XMLNS);
	pElem->SetAttribute(ATTRIB_NAME_MPD_XSI_SCHEMA_LOCN, ATTRIB_VAL_MPD_XSI_SCHEMA_LOCN);

	m_fIsLive = fDynamic;
	m_nSegmentDurationMs = nSegmentDurationMs;
	if(fDynamic)	{
		struct tm ast_time;
		time_t    time_now;
		char availability_start_time[MAX_TIME_STRING];
		time ( &time_now );
		time_now += 0;//GetCustomAvailabilityDelay() / 1000;
		ast_time = *gmtime(&time_now);
		strftime(availability_start_time, 64, "%Y-%m-%dT%H:%M:%S", &ast_time);
		pElem->SetAttribute(ATTRIB_NAME_MPD_profiles, ATTRIB_VAL_MPD_PROFILE_isoff_live);
		pElem->SetAttribute(ATTRIB_NAME_MPD_TYPE, ATTRIB_VAL_MPD_TYPE_DYNAMIC);
		pElem->SetAttribute(ATTRIB_NAME_MPD_availabilityStartTime, availability_start_time);
	} else {
		pElem->SetAttribute(ATTRIB_NAME_MPD_profiles, ATTRIB_VAL_MPD_PROFILE_isoff_on_demand);
		pElem->SetAttribute(ATTRIB_NAME_MPD_TYPE, ATTRIB_VAL_MPD_TYPE_STATIC);
	}

	FormatTime(nSegmentDurationMs / 1000, szDaration, MAX_TIME_STRING);
	pElem->SetAttribute(ATTRIB_NAME_MPD_mediaPresentationDuration, szDaration);

	int nMinimumUpdatePeriod = DEFAULT_UPDATE_PERIOD;//3600 * 1000; // 1 Hour
	FormatTime(nMinimumUpdatePeriod, szDaration, MAX_TIME_STRING);
	pElem->SetAttribute(ATTRIB_NAME_MPD_minimumUpdatePeriod, szDaration);

	int nTimeShiftBufferDepth = DEFAULT_MIN_BUFFER;
	FormatTime(nTimeShiftBufferDepth, szDaration, MAX_TIME_STRING);
	pElem->SetAttribute(ATTRIB_NAME_MPD_minBufferTime, szDaration);

	nTimeShiftBufferDepth = DEFAULT_TIME_SHIFT_BUFFER; //12 * 1000;
	FormatTime(nTimeShiftBufferDepth, szDaration, MAX_TIME_STRING);
	pElem->SetAttribute(ATTRIB_NAME_MPD_timeShiftBufferDepth, szDaration);

	int nSuggestedPresentationDelay = 4000;
	FormatTime(nSuggestedPresentationDelay, szDaration, MAX_TIME_STRING);
	pElem->SetAttribute(ATTRIB_NAME_MPD_suggestedPresentationDelay, szDaration);

	int nMaxSegmentDuration = DEFAULT_SEGMENT_DURATION; //4000;
	FormatTime(nMaxSegmentDuration, szDaration, MAX_TIME_STRING);
	pElem->SetAttribute(ATTRIB_MPD_maxSegmentDuration, szDaration);

	int nMaxSubsegmentDuration = 4000;
	FormatTime(nMaxSubsegmentDuration, szDaration, MAX_TIME_STRING);
	pElem->SetAttribute(ATTRIB_MPD_maxSubsegmentDuration, szDaration);

	m_nUpdateTime = 0;
    m_nUpdateInterval = 1000; // default 1 Sec

    m_pDoc->InsertEndChild(*pElem);
    m_pNode = m_pDoc->FirstChild();
    JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
}

CMpdPeriod *CMpdRoot::FindPeriod(std::string szPeriod)
{
	CMpdPeriod *pPeriod;
	for (std::vector<CMpdPeriod *>::iterator it = m_listPeriods.begin(); it !=  m_listPeriods.end(); it++) {
		pPeriod = *it;
		if(pPeriod->m_szId == szPeriod)
			return pPeriod;
	}
	return NULL;
}

int CMpdRoot::CreateRepresentation(std::string szPeriod, std::string szAdapt, std::string szRep, int fTmplate)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	char szDaration[MAX_TIME_STRING];
	int numAdaptations = 1;
	int numPeriods = 1;

	//CMpdRoot(1);

	CMpdPeriod *pPeriod = FindPeriod(szPeriod);
	if(pPeriod) {
		int fSegmentTmplate = 0;
		CMpdAdaptaionSet *pAdaptaionSet = pPeriod->FindAdaptationSet(szAdapt);
		if(pAdaptaionSet) {
			pAdaptaionSet->CreateRepresentation(szRep, fTmplate);
		}
	}
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return 0;
}

CMpdRepresentation *CMpdRoot::FindRepresentation(std::string szPeriod, std::string szAdapt, std::string szRep)
{
	char szDaration[MAX_TIME_STRING];
	int numAdaptations = 1;
	int numPeriods = 1;

	CMpdPeriod *pPeriod = FindPeriod(szPeriod);
	if(pPeriod) {
		int fSegmentTmplate = 0;
			CMpdAdaptaionSet *pAdaptaionSet = pPeriod->FindAdaptationSet(szAdapt);
			if(pAdaptaionSet) {
				return pAdaptaionSet->FindRepresentation(szRep);
			}
	}
	return 0;
}
