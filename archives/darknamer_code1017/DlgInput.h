#if !defined(AFX_DLGINPUT_H__0E614BF1_396A_4743_B240_309AC4A19593__INCLUDED_)
#define AFX_DLGINPUT_H__0E614BF1_396A_4743_B240_309AC4A19593__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgInput.h : header file
//

#ifndef INPUT_NONE
#define INPUT_NONE 0
#define INPUT_ONE 1
#define INPUT_TWO 2
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgInput dialog

class CDlgInput : public CDialog
{
// Construction
public:
	void AddOption(CString strText, UINT nData);
	void InitValue(CString str1, CString str2);
	void InitInputDlg(CString strTitle, CString strStatic1, CString strStatic2);
	CString m_strTitle;
	void ArrangeCtrl();
	CString m_strStatic2;
	CString m_strStatic1;
	CString m_strReturn2;
	CString m_strReturn1;
	CStringArray m_aOptionText;
	CUIntArray m_aOptionData;
	int m_nCB;
	int m_nDlgType;
	CDlgInput(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgInput)
	enum { IDD = IDD_INPUT };
	CComboBox	m_cb;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgInput)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgInput)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSelchangeCbInput();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGINPUT_H__0E614BF1_396A_4743_B240_309AC4A19593__INCLUDED_)
