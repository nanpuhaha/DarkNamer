// FolderDialog.h: interface for the CFolderDialog class.
//	$Copyright � 1998, Kenneth M. Reed, All Rights Reserved. $
//	$Header: FolderDialog.h  Revision:1.11  Mon Apr 06 12:04:50 1998  KenReed $

#ifndef _CFolderDialog_
#define _CFolderDialog_

#include <shlobj.h>

class CFolderDialog  
{
friend static int CALLBACK BrowseDirectoryCallback(
				HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData);

public:
	CFolderDialog(CString strTitle, LPCTSTR lpszFolderName = NULL, DWORD dwFlags = NULL, CWnd* pParentWnd = NULL);
	virtual ~CFolderDialog();
	virtual int DoModal();
	CString GetPathName() const;

protected:
	virtual void OnInitDialog();
	virtual void OnSelChanged(ITEMIDLIST* pIdl);
	virtual void CallbackFunction(HWND hWnd, UINT uMsg,	LPARAM lParam);

	void EnableOK(BOOL bEnable = TRUE);
	void SetSelection(LPCTSTR pszSelection);
	void SetSelection(ITEMIDLIST* pIdl);
	void SetStatusText(LPCTSTR pszStatusText);
	CString ShortName(const CString& strName);

public:
	CString m_strTitle;
	BROWSEINFO m_bi;

protected:
	CString m_strInitialFolderName;
	CString m_strFinalFolderName;

	TCHAR m_szDisplayName[MAX_PATH];
	TCHAR m_szPath[MAX_PATH];

	HWND m_hDialogBox;


};

#endif // _CFolderDialog_
