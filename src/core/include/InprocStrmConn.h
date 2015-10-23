/*
 * StrmInputInproc.h
 *
 *  Created on: Oct 20, 2015
 *      Author: Ram
 */

#ifndef INCLUDE_INPROCSTRMCONN_H_
#define INCLUDE_INPROCSTRMCONN_H_
#include "strmconn.h"
#include <string>
#include <map>

class CInprocStrmConn
{
public:
	CInprocStrmConn(ConnCtxT *pVidCon, ConnCtxT *pAudCon)
	{
		m_pVidCon = pVidCon;
		m_pAudCon = pAudCon;
	}
	CInprocStrmConn(CInprocStrmConn *pInputproc)
	{
		m_pVidCon = pInputproc->m_pVidCon;
		m_pAudCon = pInputproc->m_pAudCon;
	}

	ConnCtxT *m_pVidCon;
	ConnCtxT *m_pAudCon;
};

typedef std::map<std::string, CInprocStrmConn *> StremInputInprocMap;

class CInprocStrmConnRegistry
{
public:
	void setEntry(std::string key, CInprocStrmConn *pVal)
	{
		if(pVal == NULL){
			StremInputInprocMap::iterator it = m_StrmInputs.find(key);
			if(it != m_StrmInputs.end()) {
				CInprocStrmConn *pEntry = it->second;
				if(pEntry)
					delete pEntry;
				m_StrmInputs.erase(it);
			}

		} else {
			m_StrmInputs[key] = new CInprocStrmConn(pVal);
		}
	}
	CInprocStrmConn *getEntry(std::string key)
	{
		CInprocStrmConn *pVal = NULL;
		StremInputInprocMap::iterator it = m_StrmInputs.find(key);
		if(it != m_StrmInputs.end()) {
			CInprocStrmConn *pVal = it->second;
		}
		return pVal;
	}

	static CInprocStrmConnRegistry *getRegistry()
	{
		if(m_pInprocStrmConnRegistry == NULL)
			m_pInprocStrmConnRegistry = new CInprocStrmConnRegistry;
		return m_pInprocStrmConnRegistry;
	}
private:
	StremInputInprocMap m_StrmInputs;
	static  CInprocStrmConnRegistry *m_pInprocStrmConnRegistry;
};

#endif /* INCLUDE_INPROCSTRMCONN_H_ */
