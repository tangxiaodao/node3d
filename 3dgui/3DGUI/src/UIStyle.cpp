#include "UIStyle.h"
#include "TextRender.h"
#include "tinyxml.h"
#include "UIGraph.h"

CUIStyleMgr& GetStyleMgr()
{
	static CUIStyleMgr g_UIStyleMgr;
	return g_UIStyleMgr;
}

inline unsigned int StrToTextFormat(const char* szFormat)
{
	unsigned int uFormat = 0;
	char szFormatList[256]={'\0'};
	strcpy(szFormatList,szFormat);
	const char* pszFormat=strtok(szFormatList,"|");
	while(pszFormat)
	{
		if (strcmp("VCENTER",pszFormat)==0)
		{
			uFormat |= ALIGN_TYPE_VCENTER;
		}
		else if (strcmp("BOTTOM",pszFormat)==0)
		{
			uFormat |= ALIGN_TYPE_BOTTOM;
		}

		if (strcmp("UCENTER",pszFormat)==0)
		{

			uFormat|=ALIGN_TYPE_CENTER;
		}
		else if (strcmp("RIGHT",pszFormat)==0)
		{
			uFormat|=ALIGN_TYPE_RIGHT;
		}
		pszFormat=strtok(NULL,"|");
	}
	return uFormat;
}

StyleElement::StyleElement()
{
	for (size_t i=0;i< CONTROL_STATE_MAX;++i)
	{
		setColor[i]=0xFFFFFFFF;
	}
	SetRect(&rcOffset,0,0,0,0);
	SetRect(&rcScale,0,0,1,1);
	setColor[CONTROL_STATE_HIDDEN].a=0;
	setColor[CONTROL_STATE_DISABLED].a=0xCCCCCCCC;
	color = setColor[CONTROL_STATE_HIDDEN];

}

RECT StyleElement::updateRect(RECT rect)
{
	RECT rc;
	int nWidth = rect.right - rect.left;
	int nHeight = rect.bottom - rect.top;
	rc.left		= rect.left	+ nWidth	* rcScale.left		+ rcOffset.left;
	rc.right	= rect.left	+ nWidth	* rcScale.right		+ rcOffset.right;
	rc.top		= rect.top	+ nHeight	* rcScale.top		+ rcOffset.top;
	rc.bottom	= rect.top	+ nHeight	* rcScale.bottom	+ rcOffset.bottom;
	return rc;
}

void StyleElement::XMLParse(const TiXmlElement& element)
{
	const TiXmlElement *pElement = element.FirstChildElement("color");
	if (pElement)
	{
		for (int i = 0; i < CONTROL_STATE_MAX; ++i)
		{
			const char* pszText =  pElement->Attribute(szControlState[i]);
			if (pszText)
			{
				setColor[i] = pszText;
			}
		}
	}
	//
	const char* pszText = element.Attribute("offset");
	if(pszText)
	{
		sscanf_s(pszText, "%d,%d,%d,%d", &rcOffset.left, &rcOffset.top, &rcOffset.right, &rcOffset.bottom);
	}
	// 
	pszText = element.Attribute("scale");
	if(pszText)
	{
		sscanf_s(pszText, "%d,%d,%d,%d", &rcScale.left, &rcScale.top, &rcScale.right, &rcScale.bottom);
	}
}

void StyleSprite::draw(const RECT& rc)
{
	RECT rcDest = updateRect(rc);
	UIGraph::getInstance().setShader(m_strShader.c_str());
	switch(m_nLayoutType)
	{
	case SPRITE_LAYOUT_WRAP:
		UIGraph::getInstance().drawSprite(rcDest,m_pTexture,color,&rcDest);
		break;
	case SPRITE_LAYOUT_SIMPLE:
		UIGraph::getInstance().drawSprite(rcDest,m_pTexture,color,&m_rcBorder);
		break;
	case SPRITE_LAYOUT_3X3GRID:
		UIGraph::getInstance().drawSprite(rcDest,m_pTexture,color,&m_rcBorder,&m_rcCenter);
		break;
	default:
		break;
	}
}

void StyleSprite::XMLParse(const TiXmlElement& element)
{
	//
	StyleElement::XMLParse(element);
	if (element.Attribute("filename"))
	{
		std::string strTexture = element.Attribute("filename");
		m_pTexture = UIGraph::getInstance().createTexture(strTexture.c_str());
	}
	//
	if (element.Attribute("shader"))
	{
		m_strShader = element.Attribute("shader");
	}
	
	if (element.Attribute("rect"))
	{
		const char* strRect = element.Attribute("rect");
		sscanf_s(strRect, "%d,%d,%d,%d", &m_rcBorder.left, &m_rcBorder.top, &m_rcBorder.right, &m_rcBorder.bottom);
		m_rcBorder.right	+= m_rcBorder.left;
		m_rcBorder.bottom	+= m_rcBorder.top;
		if (element.Attribute("center_rect"))
		{
			m_nLayoutType = SPRITE_LAYOUT_3X3GRID;
			const char* strCenterRect = element.Attribute("center_rect");
			sscanf_s(strCenterRect, "%d,%d,%d,%d", &m_rcCenter.left, &m_rcCenter.top, &m_rcCenter.right, &m_rcCenter.bottom);
			m_rcCenter.left		+= m_rcBorder.left;
			m_rcCenter.top		+= m_rcBorder.top;
			m_rcCenter.right	+= m_rcCenter.left;
			m_rcCenter.bottom	+= m_rcCenter.top;
		}
		else
		{
			m_nLayoutType = SPRITE_LAYOUT_SIMPLE;
		}
	}
	else
	{
		m_nLayoutType = SPRITE_LAYOUT_WRAP;
	}
}

void StyleFont::XMLParse(const TiXmlElement& element)
{
	StyleElement::XMLParse(element);
	uFormat = 0;
	if (element.Attribute("format"))
	{
		uFormat = StrToTextFormat(element.Attribute("format"));
	}
}

void CUIStyle::Blend(UINT iState, float fElapsedTime)
{
	if (m_uState!=iState)
	{
		m_uState = iState;
		m_fRate = 0.0f;
		if (CONTROL_STATE_HIDDEN!=iState)
		{
			m_nVisible=true;
		}
	}
	// ----
	if (m_nVisible)
	{
 		if (m_fRate<1.0f)
 		{
			m_fRate += m_setBlendRate[iState]*fElapsedTime;
			m_fRate = min(1.0f,m_fRate);
		}
		else if (CONTROL_STATE_HIDDEN==iState)
		{
			m_nVisible = false;
			return;
		}
		for (size_t i=0; i<m_StyleSprites.size(); ++i)
		{
			m_StyleSprites[i].color = interpolate(m_fRate, m_StyleSprites[i].color, m_StyleSprites[i].setColor[iState]);
		}
		m_FontStyle.color = interpolate(m_fRate, m_FontStyle.color, m_FontStyle.setColor[iState]);
	}
}

void CUIStyle::setStyle(const std::string& strName)
{
	m_strName = strName;
	const CUIStyle& style = GetStyleMgr().getStyleData(strName);
	*this = style;
	Blend(CONTROL_STATE_HIDDEN,100);
}

void CUIStyle::draw(const RECT& rc, const wchar_t* wcsText)
{
	for (size_t i=0; i<m_StyleSprites.size(); ++i)
	{
		m_StyleSprites[i].draw(rc);
	}
	// ----
	if(m_FontStyle.color.a!=0 && wcsText!=NULL)
	{
		RECT rcDest = m_FontStyle.updateRect(rc);
		UIGraph::getInstance().drawText(wcsText,-1,rcDest,m_FontStyle.uFormat,m_FontStyle.color.c);
	}
}

void CUIStyle::draw(const RECT& rc, const wchar_t* wcsText, CONTROL_STATE state, float fElapsedTime)
{
	Blend(state, fElapsedTime);
	draw(rc,wcsText);
}

void CUIStyle::playSound()
{
	UIGraph::getInstance().playSound(m_strSound.c_str());
}

bool CUIStyle::isVisible()
{
	return m_nVisible!=0;
}

CUIStyle::CUIStyle()
	:m_nVisible(true)
{
	for (size_t i=0;i< CONTROL_STATE_MAX;++i)
	{
		m_setBlendRate[i]=0.8f;
	}
	m_fRate = 0.0f;
	m_uState = CONTROL_STATE_HIDDEN;
}

CUIStyle::~CUIStyle()
{
	clear();
}

void CUIStyle::clear()
{
	m_StyleSprites.clear();
}

void CUIStyle::Refresh()
{
	//m_SpriteColor.Current = m_TextureColor.States[ CONTROL_STATE_HIDDEN ];
	//m_FontColor.Current = m_FontColor.States[ CONTROL_STATE_HIDDEN ];
}

void CUIStyle::XMLParse(const TiXmlElement& xml)
{
	const TiXmlElement* pElement = xml.FirstChildElement("texture");
	while (pElement)
	{
		StyleSprite newStyleElement;
		newStyleElement.XMLParse(*pElement);
		m_StyleSprites.push_back(newStyleElement);
		pElement = pElement->NextSiblingElement("texture");
	}
	// ----
	pElement = xml.FirstChildElement("font");
	if (pElement)
	{
		m_FontStyle.XMLParse(*pElement);
	}
	// ----
	if (xml.Attribute("sound"))
	{
		m_strSound = GetStyleMgr().getDir()+xml.Attribute("sound");
	}
	//
	pElement = xml.FirstChildElement("blend");
	if (pElement)
	{
		const char* pszText = pElement->GetText();
		if(pszText)
		{
			float fBlend = (float)atof(pszText);
			for (size_t i=0;i< CONTROL_STATE_MAX;++i)
			{
				m_setBlendRate[i] = fBlend;
			}
		}
		for (size_t i=0;i< CONTROL_STATE_MAX;++i)
		{
			pszText =  pElement->Attribute(szControlState[i]);
			if (pszText)
			{
				m_setBlendRate[i] = (float)atof(pszText);
			}
		}
	}
	//

	//
	//{
	//	for (size_t i=0;i< CONTROL_STATE_MAX;++i)
	//	{
	//		setOffset[i].set(0.0f,0.0f,0.0f,0.0f);
	//	}
	//	const TiXmlElement *pElement = xml.FirstChildElement("offset");
	//	if (pElement)
	//	{
	//		const char* pszText = pElement->GetText();
	//		if(pszText)
	//		{
	//			CRect<float> rc;
	//			rc.strToRect(pszText);
	//			for (size_t i=0;i< CONTROL_STATE_MAX;++i)
	//			{
	//				setOffset[i] = rc;
	//			}
	//		}
	//		for (size_t i=0;i< CONTROL_STATE_MAX;++i)
	//		{
	//			pszText =  pElement->Attribute(szControlState[i]);
	//			if (pszText)
	//			{
	//				setOffset[i].strToRect(pszText);
	//			}
	//		}
	//	}
	//}
	//

}

const StyleFont& CUIStyle::getFontStyle()const
{
	return m_FontStyle;
}

CUIStyleMgr::CUIStyleMgr()
{
}

bool CUIStyleMgr::Create(const char* szFilename)
{
	m_strFilename = szFilename;
	m_strDir = szFilename;
	m_strDir.replace(m_strDir.find_last_of('\\'),1,"/");
	const size_t pos = m_strDir.find_last_of('/');
	m_strDir=m_strDir.substr(0,pos+1);

	m_mapStyleData.clear();
	// Load XML
	TiXmlDocument myDocument;
	myDocument.LoadFile(szFilename, TIXML_ENCODING_UTF8);
	if (myDocument.Error())
	{
		return false;
	}
	//获得根元素，即root。
	const TiXmlElement *pRootElement = myDocument.RootElement();
	if (pRootElement==NULL)
	{
		return false;
	}
	//获得第一个Style节点。
	const TiXmlElement *pStyleElement = pRootElement->FirstChildElement("element");
	while (pStyleElement)
	{
		// Style name
		if (pStyleElement->Attribute("name"))
		{
			char szNameList[256]={'\0'};
			strcpy(szNameList,pStyleElement->Attribute("name"));
			char* pszName=strtok(szNameList,",");
			while(pszName)
			{
				CUIStyle& styleData = m_mapStyleData[pszName];//.add(StyleData);
				styleData.XMLParse(*pStyleElement);
				pszName = strtok(pszName+strlen(pszName)+1,",");
			}
		}
		// 查找下一个
		pStyleElement = pStyleElement->NextSiblingElement("element");
	}
	return true;
}

bool CUIStyleMgr::CreateFromMemory(const unsigned char* pBuffer, size_t bufferSize)
{
	m_mapStyleData.clear();
	// Load XML
	TiXmlDocument myDocument;
	myDocument.LoadFormMemory((char*)pBuffer, bufferSize, TIXML_ENCODING_UTF8);
	if (myDocument.Error())
	{
		return false;
	}
	//获得根元素，即root。
	const TiXmlElement *pRootElement = myDocument.RootElement();
	if (pRootElement==NULL)
	{
		return false;
	}
	//获得第一个Style节点。
	const TiXmlElement *pStyleElement = pRootElement->FirstChildElement("element");
	while (pStyleElement)
	{
		// Style name
		if (pStyleElement->Attribute("name"))
		{
			char szNameList[256]={'\0'};
			strcpy(szNameList,pStyleElement->Attribute("name"));
			char* pszName=strtok(szNameList,",");
			while(pszName)
			{
				CUIStyle& styleData = m_mapStyleData[pszName];//.add(StyleData);
				styleData.XMLParse(*pStyleElement);
				pszName = strtok(pszName+strlen(pszName)+1,",");
			}
		}
		// 查找下一个
		pStyleElement = pStyleElement->NextSiblingElement("element");
	}
	return true;
}

const CUIStyle& CUIStyleMgr::getStyleData(const std::string& strName)
{
	if (m_mapStyleData.find(strName)!=m_mapStyleData.end())
	{
		return m_mapStyleData[strName];
	}
	static CUIStyle s_StyleData;
	return s_StyleData;
}