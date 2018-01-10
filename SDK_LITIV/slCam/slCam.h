// slCam.h : main header file for the slCam DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CslCamApp
// See slCam.cpp for the implementation of this class
//

class CslCamApp : public CWinApp
{
public:
	CslCamApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
