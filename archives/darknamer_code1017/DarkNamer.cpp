// DarkNamer.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "DarkNamer.h"
#include "DarkNamerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDarkNamerApp

BEGIN_MESSAGE_MAP(CDarkNamerApp, CWinApp)
	//{{AFX_MSG_MAP(CDarkNamerApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDarkNamerApp construction

CDarkNamerApp::CDarkNamerApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CDarkNamerApp object

CDarkNamerApp theApp;

bool SetThreadLocaleEx(LCID lcLocale)
{
	OSVERSIONINFO osVersion;
	osVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (::GetVersionEx(&osVersion) == FALSE)
	{
		return false;
	}
	if (osVersion.dwMajorVersion == 6)
	{   // >= Windows Vista
		HINSTANCE hKernelDll = ::LoadLibrary(_T("kernel32.dll"));  
		if (hKernelDll == NULL) return false;
		unsigned (WINAPI* SetThreadUILanguage)(LANGID) = (unsigned (WINAPI* )(LANGID))::GetProcAddress(hKernelDll, "SetThreadUILanguage");
		if (SetThreadUILanguage == NULL) return false;
		LANGID resLangID = SetThreadUILanguage(static_cast<LANGID>(lcLocale));
		return (resLangID == LOWORD(lcLocale));
	}
	else // <= Windows XP
	{
		return ::SetThreadLocale(lcLocale) ? true : false;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDarkNamerApp initialization

BOOL CDarkNamerApp::InitInstance()
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	SetThreadLocaleEx( MAKELCID( MAKELANGID(LANG_ENGLISH , SUBLANG_ENGLISH_US) , SORT_DEFAULT)); 
	//SetThreadLocale( MAKELCID( MAKELANGID(LANG_KOREAN , SUBLANG_KOREAN) , SORT_DEFAULT)); 

	CDarkNamerDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
