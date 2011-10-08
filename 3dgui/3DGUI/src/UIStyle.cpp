#include "UIStyle.h"
#include "TextRender.h"
#include "tinyxml.h"
#include "UIGraph.h"

CUIStyleMgr& GetStyleMgr()
{
	static CUIStyleMgr g_UIStyleMgr;
	return g_UIStyleMgr;
}

const char* szControlState[]=
{
	"normal",
	"disabled",
	"hidden",
	"focus",
	"mouseover",
	"pressed",
};

inline unsigned int StrToTextFormat(const char* szFormat)
{
	unsigned int uFormat = 0;
	char szFormatList[256]={'\0'};
	strcpy(szFormatList,szFormat);
	const char* pszFormat=strtok(szFormatList,"|");
	while(pszFormat)
	{
		if (strcmp("TOP",pszFormat)==0)
		{
			//uFormat |= DTL_TOP;
		}
		else if (strcmp("VCENTER",pszFormat)==0)
		{
			//uFormat |= DTL_VCENTER;
		}
		else if (strcmp("BOTTOM",pszFormat)==0)
		{
			//uFormat |= DTL_BOTTOM;
		}
		if (strcmp("LEFT",pszFormat)==0)
		{
			//uFormat |= DTL_LEFT;
			uFormat=ALIGN_TYPE_LEFT;
		}
		else if (strcmp("UCENTER",pszFormat)==0)
		{
			//uFormat |= DTL_CENTER;
			uFormat=ALIGN_TYPE_CENTER;
		}
		else if (strcmp("RIGHT",pszFormat)==0)
		{
			//uFormat |= DTL_RIGHT;
			uFormat=ALIGN_TYPE_RIGHT;
		}
		pszFormat=strtok(NULL,"|");
	}
	return uFormat;
}

void CUIStyle::Blend(UINT iState, float fElapsedTime)
{
	if (uState!=iState)
	{
		uState = iState;
		fRate = 0.0f;
		if (CONTROL_STATE_HIDDEN!=iState)
		{
			m_nVisible=true;
		}
	}
	if (m_nVisible)
	{
		if (fRate<1.0f)
		{
			const CUIStyle& styleData = getStyleData();

			fRate += styleData.setBlendRate[iState]*fElapsedTime;//1.0f - powf(styleData.setBlendRate[iState], 30 * fElapsedTime);
			fRate = min(1.0f,fRate);
			vRotate			= interpolate(fRate, vRotate, styleData.setRotate[iState]);
			vTranslation	= interpolate(fRate, vTranslation, styleData.setTranslation[iState]);
		}
		else if (CONTROL_STATE_HIDDEN==iState)
		{
			const CUIStyle& styleData = getStyleData();

			for (size_t i=0; i<styleData.m_setStyleElement.size(); ++i)
			{
				m_mapStyleDrawData[i].uState = iState;
				m_mapStyleDrawData[i].fRate = 1.0f;
				m_mapStyleDrawData[i].color = m_mapStyleDrawData[i].setColor[iState];
			}
			m_FontStyle.uState = iState;
			m_FontStyle.fRate = 1.0f;
			m_FontStyle.color = m_FontStyle.setColor[iState];

			m_nVisible = false;
			return;
		}
		for (size_t i=0; i<styleData.m_setStyleElement.size(); ++i)
		{
			m_mapStyleDrawData[i].blend(iState,fElapsedTime);
		}
		m_FontStyle.blend(iState,fElapsedTime);
	}
}

void CUIStyle::SetStyle(const std::string& strName)
{
	m_strName = strName;
	Blend(CONTROL_STATE_HIDDEN,100);
}

const CUIStyle& CUIStyle::getStyleData()
{
	return GetStyleMgr().getStyleData(m_strName);
}

void CUIStyle::draw(const Matrix& mTransform, const CRect<float>& rc, const wchar_t* wcsText)
{
	RECT rcNew;
	rcNew.left =0;
	rcNew.right = rc.getWidth();
	rcNew.top =0;
	rcNew.bottom = rc.getHeight();
	// ----
	mWorld = UIGraph::getInstance().setUIMatrix(mTransform,rc,vTranslation,vRotate);
	// ----
	const CUIStyle& styleData = getStyleData();
	for (size_t i=0; i<m_mapStyleDrawData.size(); ++i)
	{
		RECT rcDest=m_mapStyleDrawData[i].updateRect(rcNew);

		Color32 color = m_mapStyleDrawData[i].color.getColor();
		if(color.a==0)
		{
			continue;
		}
		//if (m_bDecolor)
		//{
		//	GetRenderSystem().SetTextureStageStateDecolor();
		//}
		switch(sprite.m_nSpriteLayoutType)
		{
		case SPRITE_LAYOUT_WRAP:
			UIGraph::getInstance().drawSprite(&rcDest,sprite.m_pTexture,color,&rcDest);
			break;
		case SPRITE_LAYOUT_SIMPLE:
			UIGraph::getInstance().drawSprite(&rcDest,sprite.m_pTexture,color,&sprite.m_rcBorder);
			break;
		case SPRITE_LAYOUT_3X3GRID:
			UIGraph::getInstance().drawSprite(&rcDest,sprite.m_pTexture,color,&sprite.m_rcBorder,&sprite.m_rcCenter);
			break;
		default:
			break;
		}
		//if (m_bDecolor)
		//{
		//	GetRenderSystem().SetupRenderState();
		//}
	}

	if(m_FontStyle.color.a!=0)
	{
		RECT rcDest = m_FontStyle.updateRect(rcNew);
		UIGraph::getInstance().drawText(wcsText,-1,rcDest,m_FontStyle.uFormat,m_FontStyle.color.c);
	}
}

void CUIStyle::draw(const Matrix& mTransform, const CRect<float>& rc, const wchar_t* wcsText, CONTROL_STATE state, float fElapsedTime)
{
	Blend(state, fElapsedTime);
	draw(mTransform,rc,wcsText);
}

void CUIStyle::draw(const Matrix& mTransform, const CRect<int>& rc, const wchar_t* wcsText, CONTROL_STATE state, float fElapsedTime)
{
	Blend(state, fElapsedTime);
	draw(mTransform,rc.getRECT(),wcsText);
}

void CUIStyle::playSound()
{
	UIGraph::getInstance().playSound(getStyleData().m_strSound.c_str());
}

bool CUIStyle::isVisible()
{
	return m_nVisible!=0;
}

// CRect<float>& CUIStyle::getTextRect()
// {
// 	return m_mapStyleDrawData[m_mapStyleDrawData.size()-1].rc;
// }

CUIStyle::CUIStyle()
	:m_nVisible(0)
{
	vTranslation.x=0.0f;
	vTranslation.y=0.0f;
	vTranslation.z=0.0f;

	vRotate.x=0.0f;
	vRotate.y=0.0f;
	vRotate.z=0.0f;

	for (size_t i=0;i< CONTROL_STATE_MAX;++i)
	{
		setBlendRate[i]=0.8f;
	}
	for (size_t i=0;i< CONTROL_STATE_MAX;++i)
	{
		setTranslation[i].x=0.0f;
		setTranslation[i].y=0.0f;
		setTranslation[i].z=0.0f;
	}
	for (size_t i=0;i< CONTROL_STATE_MAX;++i)
	{
		setRotate[i].x=0.0f;
		setRotate[i].y=0.0f;
		setRotate[i].z=0.0f;
	}
}

CUIStyle::~CUIStyle()
{
	clear();
}

void CUIStyle::clear()
{
	for (size_t i=0; i<m_setStyleElement.size(); ++i)
	{
		delete m_setStyleElement[i];
		m_setStyleElement[i] = NULL;
	}
	m_setStyleElement.clear();
}

void CUIStyle::Refresh()
{
	//m_SpriteColor.Current = m_TextureColor.States[ CONTROL_STATE_HIDDEN ];
	//m_FontColor.Current = m_FontColor.States[ CONTROL_STATE_HIDDEN ];
}

void CUIStyle::add(const std::vector<StyleElement*>& setStyleElement)
{
	m_setStyleElement.insert(m_setStyleElement.end(), setStyleElement.begin(), setStyleElement.end()); 
}

void CUIStyle::XMLParse(const TiXmlElement& xml)
{
	const TiXmlElement* pElement = xml.FirstChildElement();
	while (pElement)
	{
		if (pElement->ValueStr() == "texture")
		{
			StyleSprite* pNewStyleElement = new StyleSprite;
			pNewStyleElement->XMLParse(*pElement);
			m_setStyleElement.push_back(pNewStyleElement);
		}
		else if (pElement->ValueStr() == "font"||pElement->ValueStr() == "ubb")
		{
			m_FontStyle.XMLParse(*pElement);
		}
		pElement = pElement->NextSiblingElement();
	}
	//
	if (xml.Attribute("sound"))
	{
		m_strSound = GetStyleMgr().getDir()+xml.Attribute("sound");
	}
	//
	{
		const TiXmlElement *pElement = xml.FirstChildElement("blend");
		if (pElement)
		{
			const char* pszText = pElement->GetText();
			if(pszText)
			{
				float fBlend = (float)atof(pszText);
				for (size_t i=0;i< CONTROL_STATE_MAX;++i)
				{
					setBlendRate[i] = fBlend;
				}
			}
			for (size_t i=0;i< CONTROL_STATE_MAX;++i)
			{
				pszText =  pElement->Attribute(szControlState[i]);
				if (pszText)
				{
					setBlendRate[i] = (float)atof(pszText);
				}
			}
		}
	}
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
	{
		const TiXmlElement *pElement = xml.FirstChildElement("translation");
		if (pElement)
		{
			const char* pszText = pElement->GetText();
			if(pszText)
			{
				MY3DGUI_VEC3D v;
				sscanf_s(pszText, "%f,%f,%f", &v.x, &v.y, &v.z);
				for (size_t i=0;i< CONTROL_STATE_MAX;++i)
				{
					setTranslation[i] = v;
				}
			}
			for (size_t i=0;i< CONTROL_STATE_MAX;++i)
			{
				pszText =  pElement->Attribute(szControlState[i]);
				if (pszText)
				{
					sscanf_s(pszText, "%f,%f,%f", &setTranslation[i].x, &setTranslation[i].y, &setTranslation[i].z);
				}
			}
		}
	}
	{
		const TiXmlElement *pElement = xml.FirstChildElement("rotate");
		if (pElement)
		{
			const char* pszText = pElement->GetText();
			if(pszText)
			{
				MY3DGUI_VEC3D v;
				sscanf_s(pszText, "%f,%f,%f", &v.x, &v.y, &v.z);
				v.x*=3.14159f/180.0f;
				v.y*=3.14159f/180.0f;
				v.z*=3.14159f/180.0f;
				for (size_t i=0;i< CONTROL_STATE_MAX;++i)
				{
					setRotate[i] = v;
				}
			}
			for (size_t i=0;i< CONTROL_STATE_MAX;++i)
			{
				pszText =  pElement->Attribute(szControlState[i]);
				if (pszText)
				{
					sscanf_s(pszText, "%f,%f,%f", &setRotate[i].x, &setRotate[i].y, &setRotate[i].z);
					setRotate[i].x*=3.14159f/180.0f;
					setRotate[i].y*=3.14159f/180.0f;
					setRotate[i].z*=3.14159f/180.0f;
				}
			}
		}
	}
}

const StyleText& CUIStyle::getFontStyle()const
{
	return m_FontStyle;
}

CUIStyleMgr::CUIStyleMgr()
{
}

void StyleElement::blend(UINT iState,float fElapsedTime)
{
	if (uState!=iState)
	{
		uState = iState;
		fRate = 0.0f;
	}
	if (fRate<1.0f)
	{
		fRate += setBlendRate[iState]*fElapsedTime;
		fRate = min(1.0f,fRate);

		color	= interpolate(fRate, color,	setColor[iState]);
	}
}

void StyleElement::XMLParse(const TiXmlElement& element)
{
	for (size_t i=0;i< CONTROL_STATE_MAX;++i)
	{
		setColor[i].set(0.0f,0.0f,0.0f,0.0f);
	}
	for (size_t i=0;i< CONTROL_STATE_MAX;++i)
	{
		setBlendRate[i]=0.8f;
	}
	SetRect(&rcOffset,0,0,0,0);
	SetRect(&rcScale,0,0,1,1);
	//
	const TiXmlElement *pElement = element.FirstChildElement("color");
	if (pElement)
	{
		const char* pszText = pElement->GetText();
		if(pszText)
		{
			Color32 c;
			c = pszText;
			for (int i = 0; i < CONTROL_STATE_MAX; ++i)
			{
				setColor[i] = c;
			}
			setColor[CONTROL_STATE_HIDDEN].w = 0.0f;
		}
		for (int i = 0; i < CONTROL_STATE_MAX; ++i)
		{
			pszText =  pElement->Attribute(szControlState[i]);
			if (pszText)
			{
				Color32 c;
				c = pszText;
				setColor[i] = c;
			}
		}
	}
	//
	pElement = element.FirstChildElement("blend");
	if (pElement)
	{
		const char* pszText = pElement->GetText();
		if(pszText)
		{
			float fBlend = (float)atof(pszText);
			for (size_t i=0;i< CONTROL_STATE_MAX;++i)
			{
				setBlendRate[i] = fBlend;
			}
		}
		for (size_t i=0;i< CONTROL_STATE_MAX;++i)
		{
			pszText =  pElement->Attribute(szControlState[i]);
			if (pszText)
			{
				setBlendRate[i] = (float)atof(pszText);
			}
		}
	}
	//
	pElement = element.FirstChildElement("offset");
	if (pElement)
	{
		const char* pszText = pElement->GetText();
		if (!pszText)
		{
			pszText = element.Attribute("offset");
		}
		if(pszText)
		{
			sscanf_s(pszText, "%d,%d,%d,%d", &rcOffset.right, &rcOffset.top, &rcOffset.right, &rcOffset.bottom);
		}

	}
	// 
	pElement = element.FirstChildElement("scale");
	if (pElement)
	{
		const char* pszText = pElement->GetText();
		if (!pszText)
		{
			pszText =  element.Attribute("scale");
		}
		if(pszText)
		{
			sscanf_s(pszText, "%d,%d,%d,%d", &rcScale.right, &rcScale.top, &rcScale.right, &rcScale.bottom);
		}
	}
}

void StyleSprite::XMLParse(const TiXmlElement& element)
{
	//
	StyleElement::XMLParse(element);
	if (element.Attribute("filename"))
	{
		std::string strTexture = GetStyleMgr().getDir()+element.Attribute("filename");
		m_pTexture = UIGraph::getInstance().createTexture(strTexture.c_str());
	}
	if (element.Attribute("rect"))
	{
		const char* strRect = element.Attribute("rect");
		sscanf_s(strRect, "%d,%d,%d,%d", &m_rcBorder.right, &m_rcBorder.top, &m_rcBorder.right, &m_rcBorder.bottom);
		m_rcBorder.right	+= m_rcBorder.left;
		m_rcBorder.bottom	+= m_rcBorder.top;
		if (element.Attribute("center_rect"))
		{
			m_nSpriteLayoutType = SPRITE_LAYOUT_3X3GRID;
			const char* strCenterRect = element.Attribute("center_rect");
			sscanf_s(strCenterRect, "%d,%d,%d,%d", &m_rcCenter.right, &m_rcCenter.top, &m_rcCenter.right, &m_rcCenter.bottom);
			m_rcCenter.left		+= m_rcBorder.left;
			m_rcCenter.top		+= m_rcBorder.top;
			m_rcCenter.right	+= m_rcCenter.left;
			m_rcCenter.bottom	+= m_rcCenter.top;
		}
		else
		{
			m_nSpriteLayoutType = SPRITE_LAYOUT_SIMPLE;
		}
	}
	else
	{
		m_nSpriteLayoutType = SPRITE_LAYOUT_WRAP;
	}

	m_bDecolor = false;
	if (element.Attribute("decolor"))
	{
		if (std::string("true")==element.Attribute("decolor"))
		{
			m_bDecolor = true;
		}
	}
}

void StyleText::XMLParse(const TiXmlElement& element)
{
	StyleElement::XMLParse(element);
	uFormat = 0;
	if (element.Attribute("format"))
	{
		uFormat = StrToTextFormat(element.Attribute("format"));
	}
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