// DlgInput.cpp : implementation file
//

#include "stdafx.h"
#include "DarkNamer.h"
#include "DlgInput.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgInput dialog


/////////////////////////////////////////////////////////////////////////////
// CDlgInput dialog


CDlgInput::CDlgInput(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgInput::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgInput)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_nCB=0;
}


void CDlgInput::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgInput)
	DDX_Control(pDX, IDC_CB_INPUT, m_cb);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgInput, CDialog)
	//{{AFX_MSG_MAP(CDlgInput)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgInput message handlers

BOOL CDlgInput::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	if (m_aCombo.GetSize()==0)
	{
		m_cb.ShowWindow(SW_HIDE);
	}
	else
	{
		for (int i=0; i<m_aCombo.GetSize(); i++) 
			m_cb.AddString(m_aCombo[i]);
		m_cb.SetCurSel(m_nCB);
	}
	ArrangeCtrl();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CDlgInput::ArrangeCtrl()
{
	SetDlgItemText(IDC_STATIC_TITLE, m_strTitle);
	SetDlgItemText(IDC_STATIC_1, m_strStatic1);
	SetDlgItemText(IDC_STATIC_2, m_strStatic2);
	SetDlgItemText(IDC_EDIT_1, m_strReturn1);
	SetDlgItemText(IDC_EDIT_2, m_strReturn2);

	if (m_strStatic1.IsEmpty()==FALSE)
	{
		GetDlgItem(IDC_STATIC_1)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_EDIT_1)->ShowWindow(SW_SHOW);
	}
	else
	{
		GetDlgItem(IDC_STATIC_1)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_1)->ShowWindow(SW_HIDE);
	}

	if (m_strStatic2.IsEmpty()==FALSE)
	{
		GetDlgItem(IDC_STATIC_2)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_EDIT_2)->ShowWindow(SW_SHOW);
	}
	else
	{
		GetDlgItem(IDC_STATIC_2)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_2)->ShowWindow(SW_HIDE);
	}
	if (m_strStatic1.IsEmpty() && m_strStatic2.IsEmpty())
	{
		m_cb.MoveWindow(20,40,300,500);
	}

}

void CDlgInput::InitInputDlg(CString strTitle, CString strStatic1, CString strStatic2)
{
	m_strTitle=strTitle;
	m_strStatic1=strStatic1;
	m_strStatic2=strStatic2;
}

void CDlgInput::OnOK() 
{
	GetDlgItemText(IDC_EDIT_1, m_strReturn1);
	GetDlgItemText(IDC_EDIT_2, m_strReturn2);
	m_nCB=m_cb.GetCurSel();
	CDialog::OnOK();
}

void CDlgInput::InitValue(CString str1, CString str2)
{
	m_strReturn1=str1;
	m_strReturn2=str2;
}


/////////////////////////////////////////////////////////////////////////////
// CDlgInput message handlers

void CDlgInput::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}
