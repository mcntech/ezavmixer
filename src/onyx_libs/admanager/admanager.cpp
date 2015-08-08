#include "admanager.h"
#include "JdHttpClnt.h"
#include "JdNetUtil.h"
#include "JdMpdDefs.h"
#include "Mpd.h"

//http://www.onyxvideo.com/revive/www/delivery/fc.php?script=bannerTypeHtml:vastInlineBannerTypeHtml:vastInlineHtml&zones=pre-roll:0.0-0%3D1&nz=1&source=&r=R0.8214839450083673&block=0&format=vast&charset=UTF-8

#define ELEM_VIDDEO_AD_SERVING_TEMPLATE   "VideoAdServingTemplate"
#define ELEM_AD                            "Ad"
#define ELEM_IN_LINE                       "InLine"
#define ELEM_VIDEO                         "Video"
#define ELEM_DURAITON                      "Duration"
#define ELEM_MEDIA_FILES                   "MediaFiles"
#define ELEM_MEDIA_FILE                    "MediaFile"
#define ELEM_URL                           "URL"

#define ELEM_TRACKING_EVENTS               "TrackingEvents"
#define ELEM_TRACKING_EVENT                "TrackingEvent"

#define ATTRIB_MEDIA_FILE_TYPE             "type"
#define ATTRIB_MEDIA_FILE_BITRATE          "bitrate"
#define ATTRIB_MEDIA_FILE_WIDTH            "width"
#define ATTRIB_MEDIA_FILE_HEIGHT           "height"

#define ATTRIB_VAL_MEDIA_FILE_TYPE_MP4     "video/x-mp4"

#define ATTRIB_TRACKING_EVENT              "event"


#define ATTRIB_VAL_TRACKING_EVENT_START    "start"
#define ATTRIB_VAL_TRACKING_EVENT_MID      "midpoint"
#define ATTRIB_VAL_TRACKING_EVENT_MIDPOINT "complete"
#define ATTRIB_VAL_TRACKING_EVENT_MUTE      "mute"
#define ATTRIB_VAL_TRACKING_EVENT_PAUSE     "pause"
#define ATTRIB_VAL_TRACKING_EVENT_PAUSE     "replay"
#define ATTRIB_VAL_TRACKING_EVENT_FULLSCREEN "fullscreen"
#define ATTRIB_VAL_TRACKING_EVENT_STOP      "stop"
#define ATTRIB_VAL_TRACKING_EVENT_UNMUTE    "unmute"
#define ATTRIB_VAL_TRACKING_EVENT_RESUME    "resume"


CAdManager::CAdManager(const char *szVastTag)
{
	m_szVastTag = strdup(szVastTag);
}

CAdManager::~CAdManager()
{
	if(m_szVastTag)
		free(m_szVastTag);

}

void ConvertTimeVastToMpd(const char *pszDuration, char *szFormatted, int nMaxLen)
{
	char *szTime = strdup(pszDuration);
	int h = 0,m = 0,s = 0;
	int nDuration = 0;
	// Hour
	char *pStart = szTime;
	char *pEnd = szTime + strlen(szTime);
	char *pIdx = strchr(szTime,':');
	if(pIdx && pIdx < pEnd) {
		*pIdx = 0;
		h = atoi(pStart);
		// Minute
		pStart = pIdx + 1;
		pIdx = strchr(pStart,':');
		if(pIdx && pIdx < pEnd) {
			m = atoi(pStart);
			// Second
			pStart = pIdx + 1;
			if(pIdx && pIdx < pEnd)
				s = atoi(pStart);
		}
	}
	nDuration = h * 60 * 60 + m * 60 + s;
	FormatTime(nDuration * 1000, szFormatted, nMaxLen);
}

TiXmlNode *CAdManager::GetMpdPeriod(int nTime)
{
	TiXmlDocument *pDoc = new TiXmlDocument;
	TiXmlDocument *pMpdDoc = new TiXmlDocument;
	
	TiXmlElement  *pMpd = new TiXmlElement(ELEMENT_MPD);
	pMpdDoc->LinkEndChild(pMpd);

	int nMaxLen = (8 * 1024);
	char *pData = (char *)malloc(nMaxLen);
	int nLen = HttpGetResource(m_szVastTag, pData, nMaxLen);
	if(nLen > 0) {
		pDoc->LoadFromBuffer(pData, nLen);
		pDoc->Print();

		TiXmlNode *pNode = pDoc->FirstChild(ELEM_VIDDEO_AD_SERVING_TEMPLATE);
		if(pNode) {
			TiXmlNode *pAdNode = pNode->FirstChild(ELEM_AD);
			if(pAdNode) {
				TiXmlNode *pInLineNode = pAdNode->FirstChild(ELEM_IN_LINE);
				if(pInLineNode) {
					TiXmlNode  *pVidNode = pInLineNode->FirstChild(ELEM_VIDEO);
					if(pVidNode) {
						TiXmlElement *pPeriod = new TiXmlElement(ELEMENT_Period);
						TiXmlElement *pAdaptationSet = new TiXmlElement(ELEMENT_AdaptationSet);
						TiXmlElement *pRepresentation = new TiXmlElement(ELEMENT_Representation);
						pMpd->LinkEndChild(pPeriod);
						pPeriod->LinkEndChild(pAdaptationSet);
						pAdaptationSet->LinkEndChild(pRepresentation);
						TiXmlNode  *pMediaFilesNode = NULL;
						TiXmlNode  *pDurationNode = pVidNode->FirstChild(ELEM_DURAITON);
						if(pDurationNode) {
#define MAX_TIME_SIZE 32
							char szFormatted[MAX_TIME_SIZE];
							TiXmlElement  *pTmpElem = pDurationNode->ToElement();
							const char *pszDuration = pTmpElem->GetText();
							ConvertTimeVastToMpd(pszDuration, szFormatted, MAX_TIME_SIZE);
							pPeriod->SetAttribute(ATTRIB_NAME_PERIOD_duration, szFormatted);
						}
						pMediaFilesNode = pVidNode->FirstChild(ELEM_MEDIA_FILES);
						if(pMediaFilesNode) {
							TiXmlNode  *pMediaFileNode = pMediaFilesNode->FirstChild(ELEM_MEDIA_FILE);
							if(pMediaFileNode) {
								TiXmlElement  *pMediaFileElem = pMediaFileNode->ToElement();
								TiXmlNode  *pUrlNode = pMediaFileNode->FirstChild(ELEM_URL);
								if(pUrlNode) {
									TiXmlElement  *pUrlElem = pUrlNode->ToElement();
									const char *pszUrl = pUrlElem->GetText();
									if(pszUrl) {
										TiXmlElement  *pRepresentationUrlElem = new TiXmlElement(ELEMENT_BaseURL);
										TiXmlText  *pRepresentationUrlText = new TiXmlText(pszUrl);
										pRepresentationUrlElem->LinkEndChild(pRepresentationUrlText);
										pRepresentation->LinkEndChild(pRepresentationUrlElem);
									}
								}
								if(pMediaFileElem) {
									const char *szValue = pMediaFileElem->Attribute(ATTRIB_MEDIA_FILE_TYPE);
									if(szValue) {
										if(strcmp(szValue,ATTRIB_VAL_MEDIA_FILE_TYPE_MP4) == 0)
											pRepresentation->SetAttribute(ATTRIB_NAME_REPRESENTATION_mimetype, ATTRIB_VAL_REPRESENTATION_MIMETYPE_video_mp4);
									}
									szValue = pMediaFileElem->Attribute(ATTRIB_MEDIA_FILE_WIDTH);
									if(szValue)
										pRepresentation->SetAttribute(ATTRIB_NAME_REPRESENTATION_width,szValue);
									szValue = pMediaFileElem->Attribute(ATTRIB_MEDIA_FILE_HEIGHT);
									if(szValue)
										pRepresentation->SetAttribute(ATTRIB_NAME_REPRESENTATION_height,szValue);
									szValue = pMediaFileElem->Attribute(ATTRIB_MEDIA_FILE_BITRATE);
									if(szValue)
										pRepresentation->SetAttribute(ATTRIB_NAME_REPRESENTATION_bandwidth,szValue);
								}
							}
						}
					}

				}
			}
		}
	}
	pMpdDoc->Print();
	if(pDoc)
		delete pDoc;
	if(pData)
		free(pData);
	return pMpdDoc;
}

int CAdManager::GetAdType()
{
	return AD_TYPE_STATIC;	
};

