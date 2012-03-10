//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once /* DlgMap.h */
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#include "UIDialog.h"
#include "TSingleton.h"
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

class CDlgMap : public CUIDialog, public TSingleton<CDlgMap>
{
public:
	void OnControlRegister();
protected:
	void OnMapMove();
protected:
	CUIListBox m_ListMap;
	std::vector<std::string> m_MoveCmdList;
};
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------