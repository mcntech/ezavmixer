#ifndef __MPD_CONFIG_H__
#define __MPD_CONFIG_H__

#ifdef WIN32
//#include <Windows.h>
#include <winsock2.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include "pthread.h"
#endif
#include <string>
#include <vector>
#include <list>
//#include "strmconn.h"
#include "tinyxml.h"
#include "JdOsal.h"

#define MAX_NAME_SIZE	256

#define MAX_VID_FRAME_SIZE          (1920 * 1080)
#define MAX_AUD_FRAME_SIZE          (3 * 1024)


#define MPD_ID_SIZE 128
class CMpdRoot;
class CMpdPeriod;
class CMpdAdaptaionSet;
class CMpdRepresentation;

class CMpdSegmentBase;
class CMpdSegmentList;
class CMpdSegmentTemplate;
class CMpdSegmentUrl;

#define TS_SEGEMNT_FILE_EXT         "ts"
#define MP4_SEGEMNT_FILE_EXT        "m4s"
#define MPD_MUX_TYPE_TS             1
#define MPD_MUX_TYPE_VIDEO_MP4      2
#define MPD_MUX_TYPE_AUDIO_MP4      3
#define MPD_MUX_TYPE_M4S            4

typedef std::list<CMpdSegmentUrl *> SEGMETN_URL_LIST_T;

class CMpdSegmentUrl
{
public:
	CMpdSegmentUrl(){}

	~CMpdSegmentUrl(){	}

public:
	CMpdSegmentList *m_pParent;
	TiXmlNode       *m_pNode;
};

class CMpdInitializationUrl
{
public:
	CMpdInitializationUrl(){}

	~CMpdInitializationUrl(){	}

public:
	CMpdSegmentList *m_pParent;
	TiXmlNode       *m_pNode;
};



class CMpdSegmentBase
{
public:
	CMpdSegmentBase(CMpdRepresentation *pParent, TiXmlNode *pNode);
	~CMpdSegmentBase()
	{

	}
	void Setup(const char *pszUrl)
	{
	}

	void SetInitializationSegment(std::string &Url){}
public:

	CMpdInitializationUrl *m_SpegmentInitializationUrl;
	// Table 12 — Semantics of MultipleSegmentBaseInformation type
	int m_nDuration;
	
	CMpdRepresentation *m_pParent;
	TiXmlNode             *m_pNode;
	char                  *m_pszBitstreamSwitching;
};

class CMpdSegmentTemplate
{
public:
	CMpdSegmentTemplate(CMpdAdaptaionSet *pParent, TiXmlNode *pNode);
	~CMpdSegmentTemplate();
	void Setup(int nStartNumber, int nDurationMs, const char *pszTemplate);

	void SetInitializationSegment(std::string &Url);
public:

	CMpdInitializationUrl *m_SpegmentInitializationUrl;
	// Table 12 — Semantics of MultipleSegmentBaseInformation type
	int m_nDuration;
	int m_nStartNumber;
	
	// Segment Base Information
	//CSegmentTimeline *m_pSegmentTimeline;
	CMpdAdaptaionSet      *m_pParent;
	TiXmlNode             *m_pNode;
	char                  *m_pszBitstreamSwitching;
};

class CMpdSegmentList
{
public:
	CMpdSegmentList(CMpdRepresentation *pParent, TiXmlNode *pNode);
	~CMpdSegmentList();
	void AddSegmentUrl(CMpdSegmentUrl *pSegmentUrl)
	{
		m_listSegmentUrl.push_back(pSegmentUrl);
	}

	void SetStartNumber(int nStartNumber)
	{
		m_nStartNumber = nStartNumber;
	}

	void SetDuration(int nDuration)
	{
		m_nDuration = nDuration;
	}

	static bool deleteAll( CMpdSegmentUrl *theElement ) { delete theElement; return true; }

	void Clear()
	{
		m_listSegmentUrl.remove_if(deleteAll);
	}
	void Update(int nStartTime, int fXlink, std::list<std::string> *plistUrl, int nStatNumber);
	void SetInitializationSegment(std::string &Url);
	const char *GetInitializationSegment();
	const char *GetXlink();
	void SaveXlinkList(const char *szFileName);

public:
	std::list <CMpdSegmentUrl *> m_listSegmentUrl;
	CMpdInitializationUrl *m_SpegmentInitializationUrl;
	// Table 12 — Semantics of MultipleSegmentBaseInformation type
	int m_nDuration;
	int m_nStartNumber;
	
	// Segment Base Information
	//CSegmentTimeline *m_pSegmentTimeline;
	CMpdRepresentation *m_pParent;
	TiXmlNode             *m_pNode;
	TiXmlNode             *m_pXlinkNode;
	char                  *m_pszBitstreamSwitching;
};

class CMpdRepresentation
{
public:
	enum SEGMENT_TYPE {
		TYPE_SEGMENT_BASE,
		TYPE_SEGMENT_LIST,
		TYPE_SEGMENT_TEMPLATE,
	};
	CMpdRepresentation(CMpdAdaptaionSet *pParent);
	CMpdRepresentation(CMpdAdaptaionSet *pParent, std::string szId, TiXmlNode *pNode, int fSegmentTmplate);

	const char *GetId();
	int GetMimeType();
	int GetCutomAttribMoofLength();
	void UpdateSegmentList(int nStartTime, int nSegmentDuration, int nStartNum, std::list<std::string> *plistUrl);
	void SetInitializationSegment(std::string &Url);
	const char *GetInitializationSegment();
	void SetStreamParams(int nWith, int nHeight, int nFrameRate, int nBandwidth);
	int  IsLive(){ return 1;}
	CMpdRoot *GetMpd();
	CMpdAdaptaionSet *GetAdaptationSet(){return m_pParent;}

public:
	int            m_nBandwidth;
	int            m_nQualityRanking;
	char           m_nDependencyId[MPD_ID_SIZE];
	char           m_nMediaStreamStructureId[MPD_ID_SIZE];
	char           m_nBaseURL[MPD_ID_SIZE];

	int            m_fPrimary;
	std::string    m_inputSwitch;

	std::string          m_szId;
	CMpdAdaptaionSet     *m_pParent;
	TiXmlNode            *m_pNode;
	CMpdSegmentBase      *m_pSegmentBase;
	CMpdSegmentList      *m_pSegmentList;
	SEGMENT_TYPE         m_SegmentType;
};

class CMpdAdaptaionSet
{
public:
	CMpdAdaptaionSet(CMpdPeriod *pParent, TiXmlNode* pNode);
	~CMpdAdaptaionSet();
	int CallbackChildUpdate(CMpdRepresentation *pChild);
	void SetupTemplate(int nStartNum, int nSegmentDuration, const char *pszTemplate);
	void GetTemplate(int *pnStartNum, int *pnSegmentDuration, const char *pszTemplate);
	int IsSegmentTemplate();
	const char *GetInitializationSegmentUrl();
	const char *GetBaseURL();
	int GetMimeType();
	CMpdRoot *GetMpd();
	int CreateRepresentation(std::string szId, int fSegmentTmplate);
	CMpdRepresentation *FindRepresentation(std::string szId);

public:
	char           m_szGroup[128];
	char           m_szLang[128];
	char           m_szContentType[128];
	char           m_szPar[128];
	int            m_nMinBandWidth;
	int            m_nMaxBandWidth;
	int            m_nMinWidth;
	int            m_nMaxWidth;
	int            m_nMinHeight;
	int            m_nMaxHeight;
	int            m_nMinFrameRate;
	int            m_nMaxFrameRate;
	int            m_nSegmentAlignment;
	int            m_nBitStreamSwitching;
	char           m_szBaseURL[128];
	std::vector<CMpdRepresentation *> m_listRepresentations;
	std::string    m_szId;
	CMpdPeriod     *m_pParent;
	TiXmlNode      *m_pNode;
	CMpdSegmentTemplate  *m_pSegmentTemplate;
};

class CMpdPeriod
{
public:
	CMpdPeriod(CMpdRoot *pParent);
	CMpdPeriod(CMpdRoot *pParent, std::string strId, TiXmlNode *pNode, int nStartTime);

	~CMpdPeriod();
	CMpdAdaptaionSet *CreateAdaptationSet(std::string szId);
	CMpdAdaptaionSet *FindAdaptationSet(std::string szAdapt);
	int CallbackChildUpdate(CMpdAdaptaionSet *pChild);
	CMpdRoot *GetMpd();

public:
	std::string  m_szId;
	CMpdRoot    *m_pParent;
	TiXmlNode   *m_pNode;
	std::vector<CMpdAdaptaionSet *> m_listAdaptionSets;
};

class CMpdRoot
{
public:
	CMpdRoot(const char *pszConfFile);
	CMpdRoot( int fDynamic, int nSegmentDurationMs);
	CMpdRoot(const char *szSwitchId[], int numSwitches);
	~CMpdRoot();
	
	CMpdPeriod *FindPeriod(std::string szPeriod);
	int CreateRepresentation(std::string szPeriod, std::string szAdapt, std::string szRep, int fTmplate);
	CMpdRepresentation *FindRepresentation(std::string szPeriod, std::string szAdapt, std::string szRep);

	CMpdPeriod *CreatePeriod(std::string szId);
	void SetSaveFileName(const char *pszFileName);
	int CallbackChildUpdate(CMpdPeriod *pChild);
	int SaveFile();
	std::string GetAsXmlText();
	const char *GetBaseURL();
	const char *GetCutomCfgFolder();
	const char *GetAvailabilityStartTime();
	int GetMaxSegmentDuration();
	int GetTimeShiftBuffer();
	int GetCustomAvailabilityDelay();
	int IsUpdateRequired();
	int IsDynamic();
public:
	std::vector<CMpdPeriod *> m_listPeriods;
	TiXmlNode* m_pNode;
	TiXmlDocument *m_pDoc;
	
	int           m_fIsLive;
	int           m_nSegmentDurationMs;
	std::string   m_strSaveFile;
	unsigned int  m_nUpdateTime;
	unsigned int  m_nUpdateInterval;
	COsalMutex    m_Mutex;
};

void FormatTime(int nDurationMs, char *szString, int nLen);
static unsigned long ParseDuration(char *pszDuration);

#endif
