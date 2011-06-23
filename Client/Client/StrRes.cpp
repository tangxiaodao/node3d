#include "StrRes.h"
#include "CSVFile.h"
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

CStrRes::CStrRes()
{
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

CStrRes::~CStrRes()
{
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void CStrRes::load(const char* szFilename)
{
	m_mapStr.clear();
	// ----
	CCsvFile csvStrRes;
	// ----
	if (csvStrRes.open(szFilename))
	{
		// ----
		while (csvStrRes.seekNextLine())
		{
			const char* szSign		= csvStrRes.getStr("index","NULL");
			const char* szText		= csvStrRes.getStr("text","NULL");
			// ----
			m_mapStr[szSign] = szText;
		}
		// ----
		csvStrRes.close();
	}
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

const std::string& CStrRes::getStr(const char* szIndex)const
{
	static const std::string s_strStrResError = "ERROR";
	// ----
	std::map<std::string, std::string>::const_iterator it = m_mapStr.find(szIndex);
	// ----
	if (it!=m_mapStr.end())
		return it->second;
	// ----
	return s_strStrResError;
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------