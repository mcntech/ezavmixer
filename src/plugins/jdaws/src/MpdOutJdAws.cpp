#include "MpdOutJdAws.h"

CMpdOutJdS3::CMpdOutJdS3(const char *pszCfgFile)
{
}
CMpdOutJdS3::CMpdOutJdS3( const char *pszBucket, const char *pszHost, const char *pszAccessId, const char *pszSecKey)
{
}

int CMpdOutJdS3::Start(
			const char *pszParent, 
			const char *pszFile, 
			int        nTotalLen,	/* Totoal length of content including subsequent continue */
			char       *pData,    
			int        nLen,		/* Length of data of this call */
			const char *pContentType
			)
{
    return 0;
}

int CMpdOutJdS3::Continue(char *pData, int nLen)
{
    return 0;
}

int CMpdOutJdS3::End(char *pData, int nLen)
{
    return 0;
}

int CMpdOutJdS3::Send(const char *pszParent, const char *pszFile, char *pData, int nLen, const char *pContentType, int nTimeOut)
{
    return 0;
}

int CMpdOutJdS3::Delete(const char *pszParent, const char *pszFile)
{
    return 0;
}
