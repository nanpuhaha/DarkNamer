// DarkNamerDlg.h : header file
//

#if !defined(AFX_DARKNAMERDLG_H__EEA38216_4BCB_45BD_840B_8BB08E72E82C__INCLUDED_)
#define AFX_DARKNAMERDLG_H__EEA38216_4BCB_45BD_840B_8BB08E72E82C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//������ ���� Compare�� �Ѱ��ִ� ����
struct CSortInfo
{
	CListCtrl* plist;	//�ش� ����Ʈ ��Ʈ��
	int nAsc;			//���� ���( 1 �̸� ��������, -1 �̸� �������� )
	int nSortType;		//���� ����
};

#ifndef CListItem
struct CListItem 
{
	TCHAR strPath[MAX_PATH];	//������ ���
	DWORD dwSize;				//������ ũ��
	FILETIME timeCreated;		//���� �ñ�
	FILETIME timeModified;		//���� �ñ�
 
	BOOL bIsDirectory;			//���丮�ΰ�
};
#endif

/////////////////////////////////////////////////////////////////////////////
// CDarkNamerDlg dialog

class CDarkNamerDlg : public CDialog
{
// Construction
public:
	void UpdateCount();
	void UpdateMenu();
	//����Ʈ�� �о���� �ش� ���� �׸��� ����
	void ArrangeCtrl();
	CDarkNamerDlg(CWnd* pParent = NULL);	// standard constructor
	CToolBar m_tool1;
	CToolBar m_tool2;
	CFont m_fontToolBar;
	BOOL m_bSelected;
// Dialog Data
	//{{AFX_DATA(CDarkNamerDlg)
	enum { IDD = IDD_DARKNAMER_DIALOG };
	CListCtrl	m_list;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDarkNamerDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
	void UpdateColumn(int nCol);
	void AddByFileDialog();
	void SortList();
	inline void NameAppend(int nItem, CString strAppend);
	void NameDelToken();
	void NameDelPos();
	void NameSamePath();
	void Export(int nMode);
	void ImportName();
	void ImportPath();
	void NameEmpty();
	void NameAddNum();
	void ListDown();
	void ListUp();
	void SwapItem(int n1, int n2);
	void ApplyChange();
	void ManualChange();
	void AddFile(void* pArray, CString strFile, BOOL bIsDirectory);
	int FileListAdd(CListItem* pItem);
	static CString Get_Path(CString strFile);
	static CString Get_Name(CString strFile, BOOL bIsDirectory);
	static CString Get_Ext (CString strFile, BOOL bIsDirectory);
	void UndoChanges();
	void ExtReplace();
	void ExtAdd();
	void ExtDel();
	void NameDigit();
	void NameDelType();
	void NameAdd(BOOL bFront);
	void NameReplace();
	void ClearList();
	CImageList m_img;
	int m_nShowFlag; //�߰��� ǥ���� Į�� �÷���
	int m_nSortFlag; //Į���� ���� ���� �÷���

	static int CALLBACK Compare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	// Generated message map functions
	//{{AFX_MSG(CDarkNamerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnDblclkListFile(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnclickListFile(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DARKNAMERDLG_H__EEA38216_4BCB_45BD_840B_8BB08E72E82C__INCLUDED_)
