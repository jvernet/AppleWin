#pragma once

#include "IPropertySheet.h"
#include "PropertySheetHelper.h"
#include "PageConfig.h"
#include "PageInput.h"
#include "PageSound.h"
#include "PageDisk.h"
#include "PageAdvanced.h"

class CPropertySheet : public IPropertySheet
{
public:
	CPropertySheet() :
		m_PageConfig(m_PropertySheetHelper),
		m_PageInput(m_PropertySheetHelper),
		m_PageSound(m_PropertySheetHelper),
		m_PageDisk(m_PropertySheetHelper),
		m_PageAdvanced(m_PropertySheetHelper)
	{
	}
	virtual ~CPropertySheet(){}

	virtual void Init(void);
	virtual DWORD GetVolumeMax(void);								// TODO:TC: Move out of here
	virtual bool SaveStateSelectImage(HWND hWindow, bool bSave);	// TODO:TC: Move out of here

	virtual UINT GetScrollLockToggle(void){ return m_PageInput.GetScrollLockToggle(); }
	virtual void SetScrollLockToggle(UINT uValue){ m_PageInput.SetScrollLockToggle(uValue); }
	virtual UINT GetMouseShowCrosshair(void){ return m_PageInput.GetMouseShowCrosshair(); }
	virtual void SetMouseShowCrosshair(UINT uValue){ m_PageInput.SetMouseShowCrosshair(uValue); }
	virtual UINT GetMouseRestrictToWindow(void){ return m_PageInput.GetMouseRestrictToWindow(); }
	virtual void SetMouseRestrictToWindow(UINT uValue){ m_PageInput.SetMouseRestrictToWindow(uValue); }
	virtual UINT GetTheFreezesF8Rom(void){ return m_PageAdvanced.GetTheFreezesF8Rom(); }
	virtual void SetTheFreezesF8Rom(UINT uValue){ m_PageAdvanced.SetTheFreezesF8Rom(uValue); }

private:
	CPropertySheetHelper m_PropertySheetHelper;
	CPageConfig m_PageConfig;
	CPageInput m_PageInput;
	CPageSound m_PageSound;
	CPageDisk m_PageDisk;
	CPageAdvanced m_PageAdvanced;
};
