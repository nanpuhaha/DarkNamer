// DarkNamerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DarkNamer.h"
#include "DarkNamerDlg.h"

#pragma warning(disable:4786)

#include <map>
#include <vector>
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "DlgInput.h"
#include "FolderDialog.h"
#include "EtcFunctions.h"

typedef vector<CListItem*> CListItemArray;
typedef vector<CString> CStrArray;
typedef map<CString, int> CExtMap; //Ȯ���ڿ� �ش��ϴ� �̹������� ��ȣ�� ���

#define COL_OLDNAME 0
#define COL_NEWNAME 1
#define COL_ROOTPATH 2
#define COL_FULLPATH 3
#define COL_FILESIZE 4
#define COL_TIMEMODIFY 5
#define COL_TIMECREATE 6
#define COL_TOTAL 7

static int nPathAddMethod=-1; //0=�������� ���� , 1=���� �̸� , -1=�ʱ�ȭ �ȵ�
static HIMAGELIST himl_sys;
static CExtMap mapExt;
static int nFolderImage=-1;

//SHGetFileInfo�� ��� �θ��� �θ� �����ܰ� ���ҽ��� ������ ���� �߻�
//�̸� �����ϱ� ���� Ư�� Ȯ���ڿ� �ش��ϴ� �������� �ѹ��� ã�ƺ����� 
//�� ������ �����ϰ� ���ο� �����ܿ� ���ؼ��� SHGetFileInfo�� ȣ�� 
//�ý��� �̹��� ����Ʈ�� SHGetFileInfoȣ��� �������� �����ʿ� ����
int GetExtIcon(CString strExt, BOOL bIsDirectory)
{  
	if (bIsDirectory) 
	{
		if (nFolderImage==-1) nFolderImage=GetFileImageIndex(strExt, bIsDirectory);
		return nFolderImage;
	}
	CExtMap::iterator it=mapExt.find(strExt);
	if (it==mapExt.end()) 
	{
		int nImage=GetFileImageIndex(strExt, bIsDirectory);
		mapExt.insert(CExtMap::value_type(strExt, nImage));
		return nImage;
	}
	return (*it).second;	
}

/////////////////////////////////////////////////////////////////////////////
// CDarkNamerDlg dialog

CDarkNamerDlg::CDarkNamerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDarkNamerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDarkNamerDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_nShowFlag=0;

	//ù��° ������̺긦 ���Ѵ�
/*	TCHAR firstDrive[4]; 
	memset(firstDrive, 0, sizeof(TCHAR)*4);
	DWORD drive=GetLogicalDrives();
	int n=4;
	for(int i=2; i<32; i++)
	{
		if ( (drive&n)!=0 ) break;
		n=n*2;
	}
	firstDrive[0]=i+_T('A');
	firstDrive[1]=_T(':');
	firstDrive[2]=_T('\\');
	firstDrive[3]=_T('\0');*/
	//�ش� ��θ� �̿��ؼ� �ý��� �̹�������Ʈ�� ���Ѵ�
}

void CDarkNamerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDarkNamerDlg)
	DDX_Control(pDX, IDC_LIST_FILE, m_list);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDarkNamerDlg, CDialog)
	//{{AFX_MSG_MAP(CDarkNamerDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	ON_WM_DROPFILES()
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_FILE, OnDblclkListFile)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDarkNamerDlg message handlers

BOOL CDarkNamerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon


	SHFILEINFO sfi;
	himl_sys=(HIMAGELIST)SHGetFileInfo(_T(""), 0, &sfi,sizeof(SHFILEINFO), SHGFI_SYSICONINDEX|SHGFI_SMALLICON );
//	m_img.Attach(himl_sys);
	ListView_SetImageList(m_list.GetSafeHwnd(), himl_sys, LVSIL_SMALL);
//	m_img.Create(CImageList::FromHandle(himl_sys));
//	m_list.SetImageList(&m_img, LVSIL_SMALL);

	DragAcceptFiles(TRUE);	

	m_tool1.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_LEFT
		| CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
	m_tool1.LoadToolBar(IDR_TOOLBAR1);

	m_tool2.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_LEFT
		| CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
	m_tool2.LoadToolBar(IDR_TOOLBAR2);

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_list.InsertColumn(COL_OLDNAME,_T("�����̸�"));
	m_list.InsertColumn(COL_NEWNAME,_T("�ٲ��̸�"));
	m_list.InsertColumn(COL_ROOTPATH,_T("������ġ"));
	m_list.InsertColumn(COL_FULLPATH,_T("��ü���")); 
	m_list.InsertColumn(COL_FILESIZE,_T("����ũ��"), LVCFMT_RIGHT); 
	m_list.InsertColumn(COL_TIMEMODIFY,_T("����ð�")); 
	m_list.InsertColumn(COL_TIMECREATE,_T("�����ð�")); 

	m_list.SetColumnWidth(COL_OLDNAME,150);
	m_list.SetColumnWidth(COL_NEWNAME,150);
	m_list.SetColumnWidth(COL_ROOTPATH,100);
	m_list.SetColumnWidth(COL_FULLPATH,0);
	m_list.SetColumnWidth(COL_FILESIZE,0);
	m_list.SetColumnWidth(COL_TIMEMODIFY,0);
	m_list.SetColumnWidth(COL_TIMECREATE,0);
	
	UpdateMenu();
	ArrangeCtrl();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDarkNamerDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDarkNamerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CDarkNamerDlg::OnOK() 
{
	// TODO: Add extra validation here
	//	CDialog::OnOK();
}

void CDarkNamerDlg::OnCancel() 
{
	ClearList();
	CDialog::OnCancel();
}

#define TOOLWIDTH 44
void CDarkNamerDlg::ArrangeCtrl()
{
	CRect rc;
	GetClientRect(rc);
	rc.DeflateRect(0,0,0,20);
	m_tool1.MoveWindow(0,0,TOOLWIDTH, rc.Height());
	m_tool2.MoveWindow(rc.right-TOOLWIDTH,0,TOOLWIDTH, rc.Height());
	rc.DeflateRect(TOOLWIDTH,0,TOOLWIDTH,0);
	m_list.MoveWindow(rc);
	GetClientRect(rc);
	GetDlgItem(IDC_ST_BAR)->MoveWindow(0,rc.bottom-18,rc.Width(),18);
}

void CDarkNamerDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	if (::IsWindow(m_list.GetSafeHwnd())!=FALSE) ArrangeCtrl();
}

BOOL CDarkNamerDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	switch (wParam)
	{
	case IDM_CLEAR_LIST:		ClearList();		break;
	case IDM_INIT_LIST:			InitList();		break;
	case IDM_SORT_LIST:			SortList();		break;
	case IDM_NAME_REPLACE:		NameReplace();	break;
	case IDM_NAME_ADD_FRONT:	NameAddFront();	break;
	case IDM_NAME_ADD_REAR:		NameAddRear();	break;
	case IDM_NAME_NUMBER_ONLY:	NameNumberOnly();	break;
	case IDM_NAME_DIGIT:		NameDigit();		break;
	case IDM_EXT_ADD:			ExtAdd();			break;
	case IDM_EXT_DEL:			ExtDel();			break;
	case IDM_EXT_REPLACE:		ExtReplace();		break;
	case IDM_MANUAL_CHANGE:		ManualChange();	break;
	case IDM_APPLY_CHANGE:		ApplyChange();	break;
	case IDM_NAME_ADDNUM:		NameAddNum();		break;
	case IDM_NAME_EMPTY:		NameEmpty();		break;
	case IDM_EDIT_UP:			ListUp();			break;
	case IDM_EDIT_DOWN:			ListDown();		break;
	case IDM_NAME_SAMEPATH:		NameSamePath();	break;
	case IDM_NAME_ADDPATH:		NameAddPath();	break;
	case IDM_NAME_ADDPATH_BACK:	NameAddPathBack();break;

	case IDM_NAME_DELPOS:		NameDelPos();		break;
	case IDM_NAME_DELTOKEN:		NameDelToken();	break;

	case IDM_EXPORT_CLIP:		Export(0);		break;
	case IDM_EXPORT_FILE:		Export(1);		break;
	case IDM_EXPORT_CLIP2:		Export(2);		break;
	case IDM_EXPORT_FILE2:		Export(3);		break;
	case IDM_IMPORT_FILE:		ImportName();		break;
	case IDM_IMPORT_FILE2:		ImportPath();		break;

	case IDM_LIST_ADD:			AddByFileDialog();break;

	case IDM_SHOW_FULLPATH: 
		FlagSET(m_nShowFlag, COL_FULLPATH, !FlagGET(m_nShowFlag, COL_FULLPATH));
		UpdateColumn(COL_FULLPATH);
		break;
	case IDM_SHOW_SIZE:
		FlagSET(m_nShowFlag, COL_FILESIZE, !FlagGET(m_nShowFlag, COL_FILESIZE));
		UpdateColumn(COL_FILESIZE);
		break;
	case IDM_SHOW_MODIFYTIME:
		FlagSET(m_nShowFlag, COL_TIMEMODIFY, !FlagGET(m_nShowFlag, COL_TIMEMODIFY));
		UpdateColumn(COL_TIMEMODIFY);
		break;
	case IDM_SHOW_CREATETIME:
		FlagSET(m_nShowFlag, COL_TIMECREATE, !FlagGET(m_nShowFlag, COL_TIMECREATE));
		UpdateColumn(COL_TIMECREATE);
		break;
	case IDM_VERSION:	AfxMessageBox(_T("DarkNamer 08.02.10 ����")); break;
	default:
		return CDialog::OnCommand(wParam, lParam);
	}
	UpdateMenu();
	return TRUE;
}

BOOL CDarkNamerDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	//�ַ� ����Ű�� ó��
	if (pMsg->message==WM_KEYDOWN)
	{
		if (pMsg->wParam==VK_DELETE)
		{
			int nItem=m_list.GetNextItem(-1, LVNI_SELECTED);
			while (nItem!=-1)
			{
				CListItem* pItem=(CListItem*)m_list.GetItemData(nItem);
				delete pItem;
				m_list.DeleteItem(nItem);
				nItem=m_list.GetNextItem(-1, LVNI_SELECTED);
				UpdateCount(m_list.GetItemCount());
			}
			return TRUE;
		}
		// <>�� �̿��� ����Ʈ�󿡼� �̵� ����
		if (pMsg->wParam==188)	{ListUp();return TRUE;}
		if (pMsg->wParam==190)	{ListDown();return TRUE;} 
	}
	
	if (pMsg->message==WM_KEYUP && (GetKeyState(VK_CONTROL)&0xFF00)!=0)
	{
		if (pMsg->wParam==_T('O')) {AddByFileDialog(); return TRUE;}
		if (m_list.GetItemCount()>0)
		{
			if (pMsg->wParam==_T('S')) {ApplyChange(); return TRUE;}
			else if (pMsg->wParam==_T('Z')) {InitList(); return TRUE;}
			else if (pMsg->wParam==_T('L')) {ClearList(); return TRUE;}
			else if (pMsg->wParam==_T('A')) {SortList(); return TRUE;}
			else if (pMsg->wParam==_T('C')) 
			{
				if ((GetKeyState(VK_SHIFT)&0xFF00)!=0)	Export(2);
				else									Export(0);
				return TRUE;
			}
			else if (pMsg->wParam==_T('X')) 
			{
				if ((GetKeyState(VK_SHIFT)&0xFF00)!=0)	Export(3);
				else									Export(1);
				return TRUE;
			}
		}
		if (pMsg->wParam==_T('V')) 
		{
			if ((GetKeyState(VK_SHIFT)&0xFF00)!=0)	ImportPath();
			else if (m_list.GetItemCount()>0)		ImportName(); 
			return TRUE;
		}
	}
	
	return CDialog::PreTranslateMessage(pMsg);
}

//���ڷ� ���� ������ ������ �о �������� ���ڷ� ���� ��̿� �߰��Ѵ�.
//������ ���� �ܼ� �߰��ϸ� ������ ���丮�� ���� �ٸ��� ó��
inline void AddFileItem(CListItemArray* aFile, CListItem* pItem)
{
	//�߰� ��Ʈ�� �ʿ������ �������� ���ؼ� �����Ѵ�
	CListItemArray::reverse_iterator it=aFile->rbegin();
	while (it!=aFile->rend())
	{
		CListItem* pCompare= *(it);
		if (CompareFileName(pCompare->strPath, pItem->strPath)<0) break;
		it++;
	}
	aFile->insert(it.base(), pItem);
}

void CDarkNamerDlg::AddFile(void* pArray, CString strFile, BOOL bIsDirectory)
{
	CListItemArray* aFile=(CListItemArray*)pArray;
	if (bIsDirectory) //������ ����� ó�� 
	{
		//���� �߰� ��� ����
		if (nPathAddMethod==-1) //�� ó��(-1�϶�)���� �����
		{
			if (AfxGetMainWnd()->MessageBox(_T("��θ� ���� �߰����� YES, ��γ� ������ �߰��Ϸ��� NO ����."), strFile, MB_YESNO)==IDNO) 
				nPathAddMethod=0;
			else nPathAddMethod=1;
		}

		if (nPathAddMethod==0) //���� ���� ���ϵ��� �߰��ϱ�� ������ ��� 
		{
			CFileFind find;
			if (find.FindFile(strFile+_T("\\*.*"))==FALSE) return;
			while (TRUE)
			{
				BOOL b=find.FindNextFile();
				//.�� ..�� ���� 
				CString strName=find.GetFileName();
				if (strName!=_T(".") && strName!=_T(".."))
				{
					if (find.IsDirectory()==TRUE) //������ ��� ���ȣ��
					{
						AddFile(aFile, find.GetFilePath(), find.IsDirectory()); 
					}
					else //���� �� ������ ��� CFileFind�� �̹� ���µǾ� �����Ƿ� Ȱ���Ѵ�.
					{
						CListItem* pItem=new CListItem; //������ ����
						memset(pItem, 0, sizeof(CListItem));
						_tcscpy(pItem->strPath, find.GetFilePath());
						pItem->bIsDirectory=find.IsDirectory();
						pItem->dwSize=find.GetLength();
						find.GetCreationTime(&(pItem->timeCreated));
						find.GetLastWriteTime(&(pItem->timeModified));
						AddFileItem(aFile, pItem);
					}
				}
				if (b==FALSE) break;
			}
			find.Close();
			return; //����
		}
		//else if (nPathAddMethod==1) {} //���� �̸��� ���� �߰��ϱ�� ������ ���� �ܼ� ���ϰ� �����ϰ� ó��		
	} 

	//������ ������ �߰��Ѵ�
	//�����ڵ� ������ ���ؼ��� CreateFile/OPEN_EXISTING ���� ���� �ڵ��� ����
	HANDLE hfile=(HANDLE)CreateFile((LPCTSTR)strFile,0,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS,NULL);
	if (hfile==INVALID_HANDLE_VALUE) return;
	CListItem* pItem=new CListItem; //������ ����
	memset(pItem, 0, sizeof(CListItem));
	_tcscpy(pItem->strPath, strFile);
	pItem->bIsDirectory=bIsDirectory;
	GetFileTime(hfile, &(pItem->timeCreated), NULL, &(pItem->timeModified));
	pItem->dwSize=GetFileSize(hfile, NULL);
	CloseHandle(hfile);
	AddFileItem(aFile, pItem);
}

// Drag & Drop �� ����ؼ� ������ ��Ͽ� �߰��Ѵ� 
void CDarkNamerDlg::OnDropFiles(HDROP hDropInfo) 
{
    WORD cFiles;
    TCHAR szFileName[_MAX_PATH];
	memset(szFileName, 0, sizeof(szFileName));
	CString strFile, strPath, strName, strTemp;
    cFiles = DragQueryFile(hDropInfo, (UINT)-1, NULL, 0);

	CListItemArray aFile; //������ CListItem�� ����

	int i;
	nPathAddMethod=-1; //������ ó�� ��� ���� ����
	SetDlgItemText(IDC_ST_BAR, _T("ó����..."));
	for (i=0; i<cFiles; i++)
	{
		DragQueryFile(hDropInfo, i, szFileName, sizeof(szFileName));

		strFile = (LPCTSTR)szFileName;
		//���� ����Ʈ �߿� ���� �̸��� �ִ��� Ȯ���Ѵ�
		BOOL b=TRUE;
		for (int j=0; j<m_list.GetItemCount(); j++)
		{
			if (strFile.CompareNoCase( ((CListItem*)m_list.GetItemData(j))->strPath )==0) 
			{
				b=FALSE;
				break;
			}
		}
		//���� �̸��� ���� ��� ���翩�� Ȯ�� �� ���丮 ���� ����
		if (b==TRUE) 
		{
			DWORD dwAttribute=GetFileAttributes(strFile);
			if (dwAttribute!=-1) 
			{
				BOOL bIsDirectory=(dwAttribute&FILE_ATTRIBUTE_DIRECTORY) ? TRUE : FALSE;
				AddFile(&aFile, strFile, bIsDirectory);
			}
		}
	}
    DragFinish(hDropInfo);	
	SetDlgItemText(IDC_ST_BAR, _T(""));
	
	//����Ʈ�� ������ �߰��Ѵ�
	m_list.SetRedraw(FALSE);
	CListItemArray::iterator it=aFile.begin();
	while (it!=aFile.end())	{FileListAdd(*(it)); it++;};
	m_list.SetRedraw(TRUE);

	UpdateMenu();
}

//���� ���� �ý��� ���̾�α׸� ����ؼ� ������ ��Ͽ� �߰��Ѵ�
void CDarkNamerDlg::AddByFileDialog()
{
	CFileDialog dlg(TRUE, _T("*.*"), NULL, 
		OFN_ALLOWMULTISELECT|OFN_FILEMUSTEXIST|OFN_ENABLESIZING|OFN_LONGNAMES|OFN_HIDEREADONLY ,_T("All Files(*.*)|*.*||"),NULL);

	TCHAR *buf = new TCHAR[65536];           
	memset(buf, 0, sizeof(buf));
	dlg.m_ofn.lpstrTitle=_T("�̸� ���� ���� �ҷ�����");
	dlg.m_ofn.lpstrFile = buf;    
	dlg.m_ofn.nMaxFile = 65536;  

	if (dlg.DoModal()==IDOK)
	{
		CListItemArray aFile; //������ CListItem�� ����
		nPathAddMethod=-1; //������ ó�� ��� ���� ����

		SetDlgItemText(IDC_ST_BAR, _T("ó����..."));
		CString strFile;
		POSITION pos=dlg.GetStartPosition();
		while(pos)
		{
			strFile = dlg.GetNextPathName(pos);
			//���� ����Ʈ �߿� ���� �̸��� �ִ��� Ȯ���Ѵ�
			BOOL b=TRUE;
			for (int j=0; j<m_list.GetItemCount(); j++)
			{
				if (strFile.CompareNoCase( ((CListItem*)m_list.GetItemData(j))->strPath )==0) 
				{
					b=FALSE;
					break;
				}
			}
			//���� �̸��� ���� ��� ���翩�� Ȯ�� �� ���丮 ���� ����
			if (b==TRUE) 
			{
				DWORD dwAttribute=GetFileAttributes(strFile);
				if (dwAttribute!=-1) 
				{
					BOOL bIsDirectory=(dwAttribute&FILE_ATTRIBUTE_DIRECTORY) ? TRUE : FALSE;
					AddFile(&aFile, strFile, bIsDirectory);
				}
			}
		}
		SetDlgItemText(IDC_ST_BAR, _T(""));
		//����Ʈ�� ������ �߰��Ѵ�
		m_list.SetRedraw(FALSE);
		CListItemArray::iterator it=aFile.begin();
		while (it!=aFile.end())	{FileListAdd(*(it)); it++;};
		m_list.SetRedraw(TRUE);
	}
	delete[] buf;
}

//������ ����Ʈ��Ʈ�ѿ� ǥ���ϰ� �����Ų��
int CDarkNamerDlg::FileListAdd(CListItem* pItem)
{
	CString strPath = Get_Path(pItem->strPath);
	CString strName = Get_Name(pItem->strPath, TRUE);
	CString strExt = Get_Ext(pItem->strPath, pItem->bIsDirectory);

	int nItem;
	nItem=m_list.InsertItem(m_list.GetItemCount(), strName, GetExtIcon(strExt, pItem->bIsDirectory));
	//�ʿ��� ������ �� ����ִ� ��ü �����͸� ������ �����ͷ�
	m_list.SetItemData(nItem, (DWORD)pItem);

	//�⺻������ ǥ�õǾ�� �ϴ� ���� : ���� �̸�(Insert��)�� �ٲ� �̸�, �����ϴ� ���
	m_list.SetItemText(nItem, COL_NEWNAME, strName);
	m_list.SetItemText(nItem, COL_ROOTPATH, strPath);

	//���������� ǥ���� ����
	CString strTemp;

	if (FlagGET(m_nShowFlag,COL_FULLPATH)) 
	{
		//��ü ��� 
		m_list.SetItemText(nItem, COL_FULLPATH, pItem->strPath);
	}
	
	if (FlagGET(m_nShowFlag,COL_FILESIZE))
	{
		//���� ũ�� 
		strTemp.Format(_T("%d"), pItem->dwSize);
		m_list.SetItemText(nItem, COL_FILESIZE, strTemp);
	}
	
	SYSTEMTIME systime;

	if (FlagGET(m_nShowFlag,COL_TIMEMODIFY))
	{
		//������ �ð�
		FileTimeToSystemTime(&(pItem->timeModified), &systime);
		strTemp.Format(_T("%d-%02d-%02d %02d:%02d:%02d"), systime.wYear,  
			systime.wMonth,  systime.wDay,  systime.wHour, systime.wMinute, systime.wSecond);
		m_list.SetItemText(nItem, COL_TIMEMODIFY, strTemp);
	}

	if (FlagGET(m_nShowFlag,COL_TIMECREATE))
	{
		//������ �ð�
		FileTimeToSystemTime(&(pItem->timeCreated), &systime);
		strTemp.Format(_T("%d-%02d-%02d %02d:%02d:%02d"), systime.wYear,  
			systime.wMonth,  systime.wDay,  systime.wHour, systime.wMinute, systime.wSecond);
		m_list.SetItemText(nItem, COL_TIMECREATE, strTemp);
	}

	UpdateCount(m_list.GetItemCount());
	return nItem;
}


//����Ʈ ��� ����
void CDarkNamerDlg::ClearList()
{
	//����Ʈ�� ����� �����۵��� new�� ������ ���̹Ƿ� ��� delete �Ѵ�
	for (int i=0; i<m_list.GetItemCount(); i++)
	{
		CListItem* pItem=(CListItem*)m_list.GetItemData(i);
		delete pItem;
	}
	m_list.DeleteAllItems();
	UpdateCount(m_list.GetItemCount());
}

//�ٲ� �̸��� ���� �̸����� �ٽ� ����
void CDarkNamerDlg::InitList()
{	
	m_list.SetRedraw(FALSE);
	for (int i=0; i<m_list.GetItemCount(); i++)
	{
		m_list.SetItemText(i, COL_NEWNAME, m_list.GetItemText(i,COL_OLDNAME));
	}
	m_list.SetRedraw(TRUE);
}

void CDarkNamerDlg::ManualChange()
{
	int n=m_list.GetNextItem(-1, LVNI_SELECTED);
	if (n==-1) return;

	CDlgInput dlg;
	dlg.InitValue(m_list.GetItemText(n,COL_NEWNAME), _T(""));
	dlg.InitInputDlg(m_list.GetItemText(n,COL_NEWNAME)+_T(" ��"), _T("����"), _T(""));
	if (dlg.DoModal()==IDOK)
	{
		m_list.SetItemText(n, COL_NEWNAME, dlg.m_strReturn1);
	}
}

void CDarkNamerDlg::NameReplace()
{
	CDlgInput dlg;
	dlg.InitInputDlg(_T("�̸��� ����ִ� ���ڿ��� �ٲߴϴ�."), _T("��"), _T("����"));

	if (dlg.DoModal()==IDCANCEL) return;
	CString strTemp;
	m_list.SetRedraw(FALSE);
	for (int i=0; i<m_list.GetItemCount(); i++)
	{
		strTemp=m_list.GetItemText(i, COL_NEWNAME);
		strTemp.Replace(dlg.m_strReturn1, dlg.m_strReturn2);
		m_list.SetItemText(i, COL_NEWNAME, strTemp);
	}
	m_list.SetRedraw(TRUE);
}

void CDarkNamerDlg::NameAddFront()
{
	CDlgInput dlg;
	dlg.InitInputDlg(_T("�̸��� �տ� ������ ���ڿ��� �ٿ��ݴϴ�."), _T("���� ���ڿ�"), _T(""));

	if (dlg.DoModal()==IDCANCEL) return;
	CString strTemp;
	m_list.SetRedraw(FALSE);
	for (int i=0; i<m_list.GetItemCount(); i++)
	{
		strTemp=dlg.m_strReturn1+m_list.GetItemText(i, COL_NEWNAME);
		m_list.SetItemText(i, COL_NEWNAME, strTemp);
	}
	m_list.SetRedraw(TRUE);
}

//inline �Լ�. �̸� �� ��, Ȯ���� �ٷ� �տ� Ư�� ���ڿ��� �������ִ� ���
void CDarkNamerDlg::NameAppend(int nItem, CString strAppend)
{
	CListItem* pItem=(CListItem*)m_list.GetItemData(nItem);
	CString strName=Get_Name(m_list.GetItemText(nItem,COL_NEWNAME), pItem->bIsDirectory)+strAppend;
	CString	strExt=Get_Ext(m_list.GetItemText(nItem,COL_NEWNAME), pItem->bIsDirectory);
	if (strExt.IsEmpty()==FALSE) strName+=strExt;
	m_list.SetItemText(nItem, COL_NEWNAME, strName);
}

void CDarkNamerDlg::NameAddRear()
{
	CDlgInput dlg;
	dlg.InitInputDlg(_T("�̸��� �ڿ� ������ ���ڿ��� �ٿ��ݴϴ�."), _T("���� ���ڿ�"), _T(""));

	if (dlg.DoModal()==IDCANCEL) return;
	CString strName, strExt;
	m_list.SetRedraw(FALSE);
	for (int i=0; i<m_list.GetItemCount(); i++)
	{
		NameAppend(i, dlg.m_strReturn1);
	}
	m_list.SetRedraw(TRUE);

}
void CDarkNamerDlg::NameAddPath()
{
	//�̸��տ� �ٷ� ���� ������ ���̱�
	m_list.SetRedraw(FALSE);
	for (int i=0; i<m_list.GetItemCount(); i++)
	{
		CString strPath = m_list.GetItemText(i, COL_ROOTPATH);
		CString strTemp = strPath.Right(strPath.GetLength()-strPath.ReverseFind(_T('\\'))-1);
		if (strPath!=strTemp) //�������� c:, d: ���� ����̺� ���϶�.
		{
			strTemp+=_T("_")+m_list.GetItemText(i, COL_NEWNAME);
			m_list.SetItemText(i, COL_NEWNAME, strTemp);
		}
	}
	m_list.SetRedraw(TRUE);
}

void CDarkNamerDlg::NameAddPathBack()
{
	//�̸��ڿ� �ٷ� ���� ������ ���̱�
	for (int i=0; i<m_list.GetItemCount(); i++)
	{
		CString strPath = m_list.GetItemText(i, COL_ROOTPATH);
		CString strTemp = strPath.Right(strPath.GetLength()-strPath.ReverseFind(_T('\\'))-1); //��� �ǵ� �ڸ���
		if (strPath!=strTemp) //�������� c:, d: ���� ����̺� ���϶�.
		{
			NameAppend(i, _T("_")+strTemp);
		}
	}
}


void CDarkNamerDlg::NameNumberOnly()
{
	//�̸����� ���ڸ� �����
	CString strName, strExt;
	m_list.SetRedraw(FALSE);
	for (int i=0; i<m_list.GetItemCount(); i++)
	{
		CListItem* pItem=(CListItem*)m_list.GetItemData(i);

		strName=Get_Name(m_list.GetItemText(i, COL_NEWNAME), pItem->bIsDirectory);
		strExt=Get_Ext(m_list.GetItemText(i, COL_NEWNAME), pItem->bIsDirectory);

		for (int j=strName.GetLength()-1; j>=0; j--)
		{
			if (_T('0')>strName.GetAt(j) || strName.GetAt(j)>_T('9'))  strName.Delete(j);
		}
		if (strExt.IsEmpty()==FALSE) strName+=strExt;
		m_list.SetItemText(i, COL_NEWNAME, strName);
	}
	m_list.SetRedraw(TRUE);
}

void CDarkNamerDlg::NameDigit()
{
	CDlgInput dlg;
	dlg.InitInputDlg(_T("���ںκ��� �ڸ����� ���� 0�� ���Դϴ�."), _T("�ڸ���"), _T(""));
	dlg.m_aCombo.Add(_T("���� �޹�ȣ ����"));
	dlg.m_aCombo.Add(_T("���� �չ�ȣ ����"));

	if (dlg.DoModal()==IDCANCEL) return;
	int nDigit=_ttoi(dlg.m_strReturn1);
	if (nDigit<=0) 
	{
		AfxMessageBox(_T("�ڸ��� �Է��� �߸��Ǿ����ϴ�."));
		return;
	}
	CString strName,strExt;
	int nStatus;
	int nStart;
	int nEnd;
	m_list.SetRedraw(FALSE);
	for (int i=0; i<m_list.GetItemCount(); i++)
	{
		CListItem* pItem=(CListItem*)m_list.GetItemData(i);

		strName=Get_Name(m_list.GetItemText(i, COL_NEWNAME), pItem->bIsDirectory);
		strExt=Get_Ext(m_list.GetItemText(i, COL_NEWNAME), pItem->bIsDirectory);
		nStatus=0;
		nStart=-1;
		nEnd=-1;
		if (dlg.m_nCB==0) //�޹�ȣ
		{
			for (int j=strName.GetLength()-1; j>=0; j--)
			{
				if ((_T('0')<=strName.GetAt(j) && strName.GetAt(j)<=_T('9')) && nStatus==0)  
				{
					nEnd=j; 
					nStatus=1;
				}
				if ((_T('0')>strName.GetAt(j) || strName.GetAt(j)>_T('9')) && nStatus==1)  
				{
					nStatus=2;
					nStart=j+1; 
					break;
				}
			}
		}
		else //if (dlg.m_nCB==1) //�չ�ȣ
		{
			for (int j=0; j<=strName.GetLength()-1; j++)
			{
				if ((_T('0')<=strName.GetAt(j) && strName.GetAt(j)<=_T('9')) && nStatus==0)  
				{
					nStart=j; 
					nStatus=1;
				}
				if ((_T('0')>strName.GetAt(j) || strName.GetAt(j)>_T('9')) && nStatus==1)  
				{
					nStatus=2;
					nEnd=j-1; 
					break;
				}
			}
		}

		if (nStatus==1) nStart=0;
		int nNumLength=nEnd-nStart+1;
		if (nStart!=-1 && nEnd!=-1 && nNumLength>0)
		{
			for (int i=0; i<nDigit-nNumLength; i++) strName.Insert(nStart, _T("0"));
		}
		if (strExt.IsEmpty()==FALSE) strName+=strExt;
		m_list.SetItemText(i, COL_NEWNAME, strName);
	}
	m_list.SetRedraw(TRUE);
}

void CDarkNamerDlg::ExtDel() //Ȯ���� ����
{
	CString strTemp;
	m_list.SetRedraw(FALSE);
	for (int i=0; i<m_list.GetItemCount(); i++)
	{
		strTemp=Get_Name(m_list.GetItemText(i, COL_NEWNAME), ((CListItem*)m_list.GetItemData(i))->bIsDirectory);
		m_list.SetItemText(i, COL_NEWNAME, strTemp);
	}
	m_list.SetRedraw(TRUE);
}

void CDarkNamerDlg::ExtAdd()
{
	CDlgInput dlg;
	dlg.InitInputDlg(_T("Ȯ���ڸ� �ڿ� ���Դϴ�."), _T("���� Ȯ����"), _T(""));
	if (dlg.DoModal()==IDCANCEL) return;
	CString strTemp;
	CString strExt=dlg.m_strReturn1;
	if (strExt.IsEmpty()) return;
	if (strExt.GetAt(0)!=_T('.')) strExt=_T(".")+strExt;
	m_list.SetRedraw(FALSE);
	for (int i=0; i<m_list.GetItemCount(); i++)
	{
		strTemp=m_list.GetItemText(i, COL_NEWNAME);
		strTemp+=strExt;
		m_list.SetItemText(i, COL_NEWNAME, strTemp);
	}
	m_list.SetRedraw(TRUE);
}

void CDarkNamerDlg::ExtReplace()
{
	CDlgInput dlg;
	dlg.InitInputDlg(_T("Ȯ���ڸ� �ٲ� �ݴϴ�."), _T("�ٲ� Ȯ����"),_T(""));

	if (dlg.DoModal()==IDCANCEL) return;
	CString strTemp;
	CString strExt=dlg.m_strReturn1;
	if (strExt.IsEmpty()) return;
	if (strExt.GetAt(0)!=_T('.')) strExt=_T(".")+strExt;
	m_list.SetRedraw(FALSE);
	for (int i=0; i<m_list.GetItemCount(); i++)
	{
		strTemp=Get_Name(m_list.GetItemText(i, COL_NEWNAME), ((CListItem*)m_list.GetItemData(i))->bIsDirectory);
		strTemp+=strExt;
		m_list.SetItemText(i, COL_NEWNAME, strTemp);
	}
	m_list.SetRedraw(TRUE);
}

CString CDarkNamerDlg::Get_Path(CString strFile)
{
	CString strReturn;
	int n = strFile.ReverseFind(_T('\\'));
	strReturn = strFile.Left(n);
	return strReturn;
}

CString CDarkNamerDlg::Get_Name(CString strFile, BOOL bKeepExt)
{
	CString strReturn;
	int n1 = strFile.ReverseFind(_T('\\'));
	int n2 = -1;
	if (bKeepExt==FALSE)	n2=strFile.ReverseFind(_T('.'));
	else					n2=strFile.GetLength();
	if (n1==-1) n1=-1;
	if (n2==-1) n2=strFile.GetLength();
	strReturn = strFile.Mid(n1+1, n2-n1-1);
	return strReturn;
}

CString CDarkNamerDlg::Get_Ext(CString strFile, BOOL bIsDirectory)
{
	CString strReturn;
	int n = strFile.ReverseFind(_T('.'));
	if (n==-1 || bIsDirectory==TRUE) return _T("");
	strReturn = strFile.Right(strFile.GetLength()-n);
	return strReturn;
}

//���� ���� �ý��ۻ��� ������ �ٲ� ���� �̸� �����ϱ�
void CDarkNamerDlg::ApplyChange()
{
	if (AfxMessageBox(_T("���� ���� �̸��� �����Ͻðڽ��ϱ�?"), MB_OKCANCEL)==IDCANCEL) return;
	CString strNewPath, strTemp,strLog;
	int i;
	CStrArray aAfter;
	int nItemSel=m_list.GetNextItem(-1, LVNI_SELECTED);
	while (nItemSel!=-1)
	{
		m_list.SetItemState(nItemSel,0, LVIS_SELECTED|LVIS_FOCUSED);
		nItemSel=m_list.GetNextItem(-1, LVNI_SELECTED);
	}

	int nCount=m_list.GetItemCount();
	for (i=0; i<nCount; i++)
	{
		strTemp=m_list.GetItemText(i,COL_NEWNAME);
		if (strTemp.IsEmpty()==TRUE) 
		{
			AfxMessageBox(_T("�̸��� �������� ���� ��찡 �ֽ��ϴ�."));
			m_list.SetFocus();
			m_list.SetItemState(i,LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
			m_list.EnsureVisible(i, FALSE);
			return;
		}

		strNewPath=m_list.GetItemText(i,COL_ROOTPATH)+_T("\\")+strTemp;
		//�ߺ��Ǵ� �̸��� �ִ��� �˻��Ѵ�
		CStrArray::iterator it=aAfter.begin();
		int j=0;
		while (it!=aAfter.end())
		{
			if (strNewPath.CompareNoCase(*(it))==0) 
			{
				strTemp.Format(_T("�ߺ��Ǵ� �̸��� �ֽ��ϴ�.\n%s"), strNewPath);
				AfxMessageBox(strTemp);
				m_list.SetFocus();
				m_list.SetItemState(i,LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
				m_list.SetItemState(j,LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
				m_list.EnsureVisible(i, FALSE);
				return;
			}
			it++;
			j++;
		}
		aAfter.push_back(strNewPath);
	}
	if (aAfter.size()!=nCount) {AfxMessageBox(_T("��ü�Ҹ��� ���� ���� �߻�."));return;}
	
	//���� �����̸��� �ٲٴ� ��
	m_list.SetRedraw(FALSE);
	for (i=0; i<nCount; i++)
	{
		CListItem* pItem=(CListItem*)m_list.GetItemData(i);
		try
		{
			if (aAfter.at(i).CompareNoCase(pItem->strPath)!=0) 
			{
				if (MoveFile(pItem->strPath, aAfter[i])==FALSE) 
				{
					strTemp.Format(_T("%s -> %s ���� ����.\n"), pItem->strPath, aAfter[i]);
					strLog+=strTemp;
				}
				else
				{
					//������ ������ ��� ����Ʈ�� ǥ���ϱ� 
					_tcscpy(pItem->strPath, aAfter[i]);
					CString strPath = Get_Path(pItem->strPath);
					CString strName = Get_Name(pItem->strPath, TRUE);
					CString strExt	= Get_Ext(pItem->strPath, pItem->bIsDirectory);
					m_list.SetItem(i, COL_OLDNAME, LVIF_TEXT|LVIF_IMAGE ,strName, GetExtIcon(strExt, pItem->bIsDirectory),0,0,0);
					m_list.SetItemText(i, COL_NEWNAME, strName);
					m_list.SetItemText(i, COL_ROOTPATH, strPath);
					if (FlagGET(m_nShowFlag, COL_FULLPATH)) m_list.SetItemText(i, COL_FULLPATH, pItem->strPath);				
				}
			}
		}
		catch (CFileException *e)
		{
			e->ReportError();
			e->Delete();
			strTemp.Format(_T("%s�� %s�� �ٲٴµ� ����.\n"), pItem->strPath, aAfter[i]);
			strLog+=strTemp;
		}
	}
	m_list.SetRedraw(TRUE);
	if (strLog.IsEmpty()==FALSE) AfxMessageBox(strLog);
	else						 AfxMessageBox(_T("���� �̸��� �����Ͽ����ϴ�."));
}	

//�ΰ��� ������ ��ȯ
void CDarkNamerDlg::SwapItem(int n1, int n2)
{
	CString str1, str2;
	int nCol=COL_TOTAL, i=0;

	//������ ����Ÿ ��ȯ
	DWORD dw1=m_list.GetItemData(n1);
	DWORD dw2=m_list.GetItemData(n2);
	m_list.SetItemData(n1, dw2);
	m_list.SetItemData(n2, dw1);

	//������ �̹��� ��ȯ
	LVITEM li;
	memset(&li, 0, sizeof(LVITEM));
	li.mask=LVIF_IMAGE;
	li.iItem=n1;
	m_list.GetItem(&li);
	int img1=li.iImage;
	li.iItem=n2;
	m_list.GetItem(&li);
	int img2=li.iImage;

	m_list.SetItem(n1, 0, LVIF_IMAGE ,NULL, img2, 0, 0, 0);
	m_list.SetItem(n2, 0, LVIF_IMAGE ,NULL, img1, 0, 0, 0);

	//Į�� ���� ��ȯ 
	for (i=0; i<nCol; i++) 
	{
		str1=m_list.GetItemText(n1,i);
		str2=m_list.GetItemText(n2,i);
		m_list.SetItemText(n1, i, str2);
		m_list.SetItemText(n2, i, str1);
	}

	//������ ���� ��ȯ
	DWORD d1=m_list.GetItemState(n1, LVIS_SELECTED|LVIS_FOCUSED);
	DWORD d2=m_list.GetItemState(n2, LVIS_SELECTED|LVIS_FOCUSED);
	m_list.SetItemState(n1,d2, LVIS_SELECTED|LVIS_FOCUSED);
	m_list.SetItemState(n2,d1, LVIS_SELECTED|LVIS_FOCUSED);

	m_list.EnsureVisible(n2, FALSE);
}

//���õ� ����Ʈ �������� ��ĭ ���� �ø��� 
void CDarkNamerDlg::ListUp()
{
	m_list.SetRedraw(FALSE);
	int nItem=m_list.GetNextItem(-1, LVNI_SELECTED);
	while(nItem!=-1)
	{
		if (nItem>0) 
		{
			SwapItem(nItem, nItem-1);
			nItem=m_list.GetNextItem(nItem, LVNI_SELECTED);
		}
		else
		{
			nItem=-1;
		}
	}
	m_list.SetRedraw(TRUE);

}

//���õ� ����Ʈ �������� ��ĭ �Ʒ��� ������ 
void CDarkNamerDlg::ListDown()
{
	int nItem=m_list.GetNextItem(m_list.GetItemCount()-2, LVNI_SELECTED);
	if (nItem!=-1) return; //Bottom Item Selected
	nItem=m_list.GetNextItem(m_list.GetItemCount()-1, LVNI_SELECTED|LVNI_ABOVE);
	while(nItem!=-1)
	{
		if (nItem>=0 && nItem<m_list.GetItemCount()-1) 
		{
			SwapItem(nItem, nItem+1);
			nItem=m_list.GetNextItem(nItem, LVNI_SELECTED|LVNI_ABOVE);
		}
		else
		{
			nItem=-1;
		}
	}
}

void CDarkNamerDlg::NameAddNum()
{
	CDlgInput dlg;
	dlg.InitInputDlg(_T("���� ������ �ڸ����� ���۰��� �����մϴ�."), _T("�ڸ���"), _T("���۰�"));
	dlg.m_aCombo.Add(_T("�̸��ڿ� ��ȣ����"));
	dlg.m_aCombo.Add(_T("�̸��տ� ��ȣ����"));
	dlg.m_aCombo.Add(_T("�������� �� ��ȣ����"));
	dlg.m_aCombo.Add(_T("�������� �� ��ȣ����"));

	if (dlg.DoModal()==IDCANCEL) return;
	int nDigit=_ttoi(dlg.m_strReturn1);
	int nStart=_ttoi(dlg.m_strReturn2);
	if (nDigit<=0) 
	{
		AfxMessageBox(_T("�ڸ��� �Է��� �߸��Ǿ����ϴ�."));
		return;
	}
	if (nStart<=0) nStart=0;
	CString strName,strExt, strTemp;
	int nCurrent=nStart;
	int nCount=m_list.GetItemCount();
	m_list.SetRedraw(FALSE);
	for (int i=0; i<nCount; i++)
	{
		if (dlg.m_nCB>1 && i>0) //dlg.m_nCB==2 || dlg.m_nCB==3 
		{
			//������ ��ȣ ���̱� ���
			if (m_list.GetItemText(i-1,COL_ROOTPATH).CompareNoCase(m_list.GetItemText(i,COL_ROOTPATH))!=0) nCurrent=nStart;
		}
		CListItem* pItem=(CListItem*)m_list.GetItemData(i);
		strName=Get_Name(m_list.GetItemText(i,COL_NEWNAME), pItem->bIsDirectory);
		strExt=Get_Ext(m_list.GetItemText(i,COL_NEWNAME), pItem->bIsDirectory);
		strTemp.Format(_T("%d"),nCurrent);
		while(nDigit>strTemp.GetLength()) strTemp=_T('0')+strTemp;
		if (dlg.m_nCB==0 || dlg.m_nCB==2)	strName+=strTemp; //�ڿ� ���̱�
		else								strName=strTemp+strName; //�տ� ���̱�
		if (strExt.IsEmpty()==FALSE) strName+=strExt;
		m_list.SetItemText(i, COL_NEWNAME, strName);

		nCurrent++;
	}
	m_list.SetRedraw(TRUE);
}

void CDarkNamerDlg::NameEmpty() //�ٲ� �̸� �κ��� Ȯ���ڸ� �����ϰ� ��� �����
{
	CString strExt;
	m_list.SetRedraw(FALSE);
	for (int i=0; i<m_list.GetItemCount(); i++)
	{
		strExt=Get_Ext(m_list.GetItemText(i,COL_NEWNAME), ((CListItem*)m_list.GetItemData(i))->bIsDirectory);
		m_list.SetItemText(i, COL_NEWNAME, strExt);
	}
	m_list.SetRedraw(TRUE);
}

//nMode=0 : Ŭ������� ���ϸ� ����, nMode=1 : ���Ϸ� ���ϸ� ����
//nMode=2 : Ŭ������� ��θ� ����, nMode=3 : ���Ϸ� ��θ� ����
void CDarkNamerDlg::Export(int nMode)
{
	CString strData;
	for (int i=0; i<m_list.GetItemCount(); i++)
	{
		if (nMode==0 || nMode==1) strData+=m_list.GetItemText(i, COL_NEWNAME)+_T("\r\n");
		else if (nMode==2 || nMode==3) 
		{
			strData+=((CListItem*)m_list.GetItemData(i))->strPath;
			strData+=_T("\r\n");
		}
	}

	if (nMode==0 || nMode==2) 
	{
		CEdit edit;
		edit.Create(ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN , CRect(0,0,0,0), this, 38111);
		edit.SetWindowText(strData);
		edit.SetSel(0,-1);
		edit.Copy();
		edit.SetWindowText(_T(""));
	}
	else if (nMode==1 || nMode==3)
	{
		CFileDialog dlg(FALSE, _T("txt"), NULL, OFN_ENABLESIZING|OFN_LONGNAMES|OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY ,_T("Text Files(*.txt)|*.txt|All Files(*.*)|*.*||"),NULL);

		if (nMode==1)	dlg.m_ofn.lpstrTitle=_T("���ϸ� ����");
		else			dlg.m_ofn.lpstrTitle=_T("��θ� ����"); //nMode==3
		if (dlg.DoModal()==IDCANCEL) return;
		WriteCStringToFile(dlg.GetPathName(), strData);
	}
}

//���ϸ��� �ؽ�Ʈ ���Ͽ��� �о�ͼ� ���ʴ�� ���� �ٲ� �̸��� COL_NEWNAME �� �����Ѵ�
//�ؽ�Ʈ ������ �̸� �ϳ��� ����(\n)�� �����ؼ� ������ Ÿ��
//���� ����Ʈ�� �ִ� ������ŭ �о�´�
void CDarkNamerDlg::ImportName()
{
	CFileDialog dlg(TRUE, _T("*.txt"), NULL, OFN_ENABLESIZING|OFN_LONGNAMES|OFN_FILEMUSTEXIST|OFN_HIDEREADONLY ,_T("Text Files(*.txt)|*.txt|All Files(*.*)|*.*||"),NULL);
	dlg.m_ofn.lpstrTitle=_T("�ٲ� ���� �̸� �ҷ�����");
	if (dlg.DoModal()==IDCANCEL) return;
	CString strData, strName;
	ReadFileToCString(dlg.GetPathName(), strData);
	int nPos = 0;
	int i=0;
	m_list.SetRedraw(FALSE);
	while(nPos!=-1)
	{
		nPos=GetLine(strData, nPos, strName, _T("\n"));
		strName.TrimLeft();strName.TrimRight();
		if (strName.IsEmpty()==FALSE)
		{
			//���� �̸��� �����ؾ߸� �ϹǷ� �� ������ ��ü�ϴ� ����
			if (i>=m_list.GetItemCount()) break; //����Ʈ �Ѿ�� ��
			m_list.SetItemText(i, COL_NEWNAME, strName);
			i++;
		}
	}
	m_list.SetRedraw(TRUE);
}

//��θ��� �ؽ�Ʈ ���Ͽ��� �о�ͼ� ���� ��� �ڿ� �߰��Ѵ�
//�ؽ�Ʈ ������ �̸� �ϳ��� ����(\n)�� �����ؼ� ������ Ÿ��
void CDarkNamerDlg::ImportPath()
{
	CFileDialog dlg(TRUE, _T("*.txt"), NULL, OFN_ENABLESIZING|OFN_LONGNAMES|OFN_FILEMUSTEXIST|OFN_HIDEREADONLY ,_T("Text Files(*.txt)|*.txt|All Files(*.*)|*.*||"),NULL);
	dlg.m_ofn.lpstrTitle=_T("���Ͽ��� ��θ�� �о� �߰��ϱ�");
	if (dlg.DoModal()==IDCANCEL) return;
	CString strImportData;
	ReadFileToCString(dlg.GetPathName(), strImportData);

	CString strFile;
	CString& strData=strImportData;
	int nPos = 0;
	CListItemArray aFile;

	SetDlgItemText(IDC_ST_BAR, _T("ó����..."));
	while(nPos!=-1)
	{
		nPos=GetLine(strData, nPos, strFile, _T("\n"));
		strFile.TrimLeft();strFile.TrimRight();
		if (strFile.IsEmpty()==FALSE)
		{
			//���� ����Ʈ �߿� ���� �̸��� �ִ��� Ȯ���Ѵ�
			BOOL b=TRUE;
			for (int j=0; j<m_list.GetItemCount(); j++)
			{
				if (strFile.CompareNoCase( ((CListItem*)m_list.GetItemData(j))->strPath )==0) 
				{
					b=FALSE;
					break;
				}
			}
			//���� �̸��� ���� ��� ���翩�� Ȯ�� �� ���丮 ���� ����
			if (b==TRUE) 
			{
				DWORD dwAttribute=GetFileAttributes(strFile);
				if (dwAttribute!=-1) 
				{
					BOOL bIsDirectory=(dwAttribute&FILE_ATTRIBUTE_DIRECTORY) ? TRUE : FALSE;
					AddFile(&aFile, strFile, bIsDirectory);
				}
			}
		}
	}
	SetDlgItemText(IDC_ST_BAR, _T(""));
	//����Ʈ�� ������ �߰��Ѵ�
	m_list.SetRedraw(FALSE);
	CListItemArray::iterator it=aFile.begin();
	while (it!=aFile.end())	{FileListAdd(*(it)); it++;};
	m_list.SetRedraw(TRUE);
}


//������ ��θ� �ϳ��� ����. ������ MoveFile�� ��. �ߺ� üũ �ʿ�
void CDarkNamerDlg::NameSamePath()
{
	CFolderDialog dlg(_T("��� ����"));
	if (dlg.DoModal()==IDCANCEL) return;
	CString strPath=dlg.GetPathName();
	//e:\ ���� ��츦 ����Ͽ� ���� ���� \�� ����
	if (strPath.GetAt(strPath.GetLength()-1)==_T('\\')) strPath.Delete(strPath.GetLength()-1);
	m_list.SetRedraw(FALSE);
	for (int i=0; i<m_list.GetItemCount(); i++)
	{
		//COL_ROOTPATH�� ������ �ָ� ���߿� ���� ���涧 �̰��� COL_NEWNAME�� ���յǸ鼭 �� ��ΰ� �ȴ�
		m_list.SetItemText(i, COL_ROOTPATH, strPath); 
	}
	m_list.SetRedraw(TRUE);
}

void CDarkNamerDlg::NameDelPos()
{
	CDlgInput dlg;
	dlg.InitInputDlg(_T("������ġ�� �����մϴ�.(ù���ڴ� 1��°)"), _T("��°����"), _T("��°����"));
	dlg.m_aCombo.Add(_T("�տ������� ����"));
	dlg.m_aCombo.Add(_T("���� �ں��� ����"));
	if (dlg.DoModal()==IDCANCEL) return;

	int nStart=_ttoi(dlg.m_strReturn1);
	int nEnd=_ttoi(dlg.m_strReturn2);
	if (nStart==0 && nEnd==0) return;

	if ( nStart<0 || nEnd<0 )
		{AfxMessageBox(_T("�������̳� �߸��� ���� �ԷµǾ����ϴ�."));return;}
	if ( dlg.m_nCB==0 && nEnd>0 && nStart>nEnd )
		{AfxMessageBox(_T("�������� �������� �ڿ� �ֽ��ϴ�."));return;}
	if ( dlg.m_nCB==1 && nStart!=0 )
		{AfxMessageBox(_T("�� �ڿ������� �����Ҷ��� '~����'�� �ʿ��մϴ�."));return;}

	CString strName, strExt;

	m_list.SetRedraw(FALSE);
	for (int i=0; i<m_list.GetItemCount(); i++)
	{
		CListItem* pItem=(CListItem*)m_list.GetItemData(i);
		strName=Get_Name(m_list.GetItemText(i, COL_NEWNAME), pItem->bIsDirectory);
		strExt=Get_Ext(m_list.GetItemText(i, COL_NEWNAME), pItem->bIsDirectory);

		if (dlg.m_nCB==0)	//���� n���� m����
		{
			if (nStart==0) nStart=1;
			int nLen=strName.GetLength();
			if (nStart<=nLen)
			{
				if (nEnd>0 && nEnd<nLen) nLen=nEnd;
				strName.Delete(nStart-1, nLen-nStart+1);
			}
		}
		else if (dlg.m_nCB==1) //���� n��
		{
			int nLen=strName.GetLength();
			if (nEnd<nLen) nLen=nEnd;
			strName.Delete(strName.GetLength()-nLen, nLen);
		}

		if (strExt.IsEmpty()==FALSE) strName+=strExt;
		m_list.SetItemText(i, COL_NEWNAME, strName);
	}
	m_list.SetRedraw(TRUE);
}

void CDarkNamerDlg::NameDelToken()
{
	CDlgInput dlg;
	dlg.InitInputDlg(_T("������ ���ڷ� ���� �κ��� �����մϴ�."), _T(":���۹���"), _T(":������"));

	if (dlg.DoModal()==IDCANCEL) return;
	if (dlg.m_strReturn1.IsEmpty() || dlg.m_strReturn2.IsEmpty())
	{
		AfxMessageBox(_T("����/�� ���ڰ� ��Ȯ�ϰ� �������� �ʾҽ��ϴ�."));
		return;
	}

	TCHAR c1=dlg.m_strReturn1.GetAt(0);
	TCHAR c2=dlg.m_strReturn2.GetAt(0);

	CString strName, strExt;
	int n1, n2, nStart, nEnd;
	m_list.SetRedraw(FALSE);
	for (int i=0; i<m_list.GetItemCount(); i++)
	{
		CListItem* pItem=(CListItem*)m_list.GetItemData(i);
		strName=Get_Name(m_list.GetItemText(i, COL_NEWNAME), pItem->bIsDirectory);
		strExt=Get_Ext(m_list.GetItemText(i, COL_NEWNAME), pItem->bIsDirectory);
		
		nStart=-1; nEnd=strName.GetLength()-1;
		n1=0; n2=0;

		for (n1=0; n1<=nEnd; n1++) 
		{
			if (strName.GetAt(n1)==c1) {nStart=n1;n1++;break;}
		}
		for (n2=n1; n2<=nEnd; n2++) 
		{
			if (strName.GetAt(n2)==c2) {nEnd=n2;break;}
		}
		
		if (nStart!=-1 && nStart<nEnd && nEnd==n2)	
		{
			strName.Delete(nStart, nEnd-nStart+1);
		}
		if (strExt.IsEmpty()==FALSE) strName+=strExt;
		m_list.SetItemText(i, COL_NEWNAME, strName);
	}
	m_list.SetRedraw(TRUE);
}

//����Ʈ�� ������ ���ؿ� ���� �����Ѵ�
void CDarkNamerDlg::SortList()
{
	CDlgInput dlg;
	dlg.InitInputDlg(_T("���� ���� ����"), _T(""), _T(""));
	dlg.m_aCombo.Add(_T("���� �̸��� ���� ��������"));
	dlg.m_aCombo.Add(_T("���� �̸��� ���� ��������"));
	dlg.m_aCombo.Add(_T("��ü��ο� ���� ��������"));
	dlg.m_aCombo.Add(_T("��ü��ο� ���� ��������"));
	dlg.m_aCombo.Add(_T("���� ũ�⿡ ���� ��������"));
	dlg.m_aCombo.Add(_T("���� ũ�⿡ ���� ��������"));
	dlg.m_aCombo.Add(_T("������ �ð��� ���� ��������"));
	dlg.m_aCombo.Add(_T("������ �ð��� ���� ��������"));
	dlg.m_aCombo.Add(_T("���� �ð��� ���� ��������"));
	dlg.m_aCombo.Add(_T("���� �ð��� ���� ��������"));

	if (dlg.DoModal()==IDCANCEL) return;
	
	CSortInfo si;
	si.plist=&m_list;
	switch (dlg.m_nCB)
	{
	case 0: si.nSortType=COL_OLDNAME; si.nAsc=1; break;
	case 1: si.nSortType=COL_OLDNAME; si.nAsc=-1; break;
	case 2: si.nSortType=COL_FULLPATH; si.nAsc=1; break;
	case 3: si.nSortType=COL_FULLPATH; si.nAsc=-1; break;
	case 4: si.nSortType=COL_FILESIZE; si.nAsc=1; break;
	case 5: si.nSortType=COL_FILESIZE; si.nAsc=-1; break;
	case 6: si.nSortType=COL_TIMEMODIFY; si.nAsc=1; break;
	case 7: si.nSortType=COL_TIMEMODIFY; si.nAsc=-1; break;
	case 8: si.nSortType=COL_TIMECREATE; si.nAsc=1; break;
	case 9: si.nSortType=COL_TIMECREATE; si.nAsc=-1; break;
	}
	m_list.SetRedraw(FALSE);
	m_list.SortItems(Compare, (DWORD)&si);
	m_list.SetRedraw(TRUE);
}


int CALLBACK CDarkNamerDlg::Compare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	//lParam1, lParam2�� �񱳵� �� ����Ʈ �׸��� ItemData��
	//�ַ� �ش� �׸��� �ε��� ���� �״�� �־ ����
	//lParamSort�� SortItems�� �ι�° ���ڰ�(�ַ� �ش� ����Ʈ ������)
	CSortInfo* psi=(CSortInfo*)lParamSort;
	CListItem* pItem1=(CListItem*)lParam1;
	CListItem* pItem2=(CListItem*)lParam2;

	int nOrder;
	//Į�� ������ ���� �� ��� �ٲٱ�
	switch (psi->nSortType)
	{
	case COL_OLDNAME:
		{
			CString str1 = Get_Name(pItem1->strPath, TRUE);
			CString str2 = Get_Name(pItem2->strPath, TRUE);
			nOrder=CompareFileName(str1.GetBuffer(0), str2.GetBuffer(0));
		}
		break;
	case COL_FULLPATH:
		nOrder=CompareFileName(pItem1->strPath, pItem2->strPath);
		break;
	case COL_FILESIZE:
		nOrder=pItem1->dwSize-pItem2->dwSize;
		break;
	case COL_TIMEMODIFY:
		nOrder=CompareFileTime(&pItem1->timeModified, &pItem2->timeModified);
		break;
	case COL_TIMECREATE:
		nOrder=CompareFileTime(&pItem1->timeCreated, &pItem2->timeCreated);
		break;
	}

	nOrder=nOrder * psi->nAsc; //��������, ���������� �ݿ�
	return nOrder; //lParamSort is ASC / DSC
}

void CDarkNamerDlg::UpdateColumn(int nCol)
{
	BOOL bShow=FlagGET(m_nShowFlag, nCol);
	if (bShow)	
	{
		if (nCol==COL_FILESIZE) m_list.SetColumnWidth(nCol, 80);
		else					m_list.SetColumnWidth(nCol, 120);
	}
	else		m_list.SetColumnWidth(nCol, 0);
	if (bShow==FALSE) return;


	int nCount=m_list.GetItemCount();
	//�� �� �������� �˻��ؼ� ������ �̹� ä���� ������ �н� �ƴϸ� ���� ä���
	if (nCount>1)
	{
		if (m_list.GetItemText(nCount-1, nCol).IsEmpty()==FALSE) return;
	}
	
	m_list.SetRedraw(FALSE);
	CString strTemp;
	SYSTEMTIME systime;
	for (int i=0; i<nCount; i++)
	{
		CListItem* pItem=(CListItem*)m_list.GetItemData(i);
		switch (nCol)
		{
		case COL_FULLPATH: 
			m_list.SetItemText(i, COL_FULLPATH, pItem->strPath);
			break;
		case COL_FILESIZE: 
			strTemp.Format(_T("%d"), pItem->dwSize);
			m_list.SetItemText(i, COL_FILESIZE, strTemp);
			break;
		case COL_TIMEMODIFY: 
			FileTimeToSystemTime(&(pItem->timeModified), &systime);
			strTemp.Format(_T("%d-%02d-%02d %02d:%02d:%02d"), systime.wYear,  
				systime.wMonth,  systime.wDay,  systime.wHour, systime.wMinute, systime.wSecond);
			m_list.SetItemText(i, COL_TIMEMODIFY, strTemp);
			break;
		case COL_TIMECREATE: 
			FileTimeToSystemTime(&(pItem->timeCreated), &systime);
			strTemp.Format(_T("%d-%02d-%02d %02d:%02d:%02d"), systime.wYear,  
				systime.wMonth,  systime.wDay,  systime.wHour, systime.wMinute, systime.wSecond);
			m_list.SetItemText(i, COL_TIMECREATE, strTemp);
			break;
		}
	}
	m_list.SetRedraw(TRUE);
}

void CDarkNamerDlg::OnDblclkListFile(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	ManualChange();	
	*pResult = 0;
}

void CDarkNamerDlg::UpdateMenu()
{
	BOOL b=(m_list.GetItemCount()>0);
	GetMenu()->EnableMenuItem(IDM_APPLY_CHANGE, b ? MF_ENABLED|MF_BYCOMMAND : MF_GRAYED|MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_CLEAR_LIST, b ? MF_ENABLED|MF_BYCOMMAND : MF_GRAYED|MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_SORT_LIST, b ? MF_ENABLED|MF_BYCOMMAND : MF_GRAYED|MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_INIT_LIST, b ? MF_ENABLED|MF_BYCOMMAND : MF_GRAYED|MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_NAME_REPLACE, b ? MF_ENABLED|MF_BYCOMMAND : MF_GRAYED|MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_NAME_ADD_FRONT, b ? MF_ENABLED|MF_BYCOMMAND : MF_GRAYED|MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_NAME_ADD_REAR, b ? MF_ENABLED|MF_BYCOMMAND : MF_GRAYED|MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_NAME_NUMBER_ONLY, b ? MF_ENABLED|MF_BYCOMMAND : MF_GRAYED|MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_NAME_DIGIT, b ? MF_ENABLED|MF_BYCOMMAND : MF_GRAYED|MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_NAME_ADDNUM, b ? MF_ENABLED|MF_BYCOMMAND : MF_GRAYED|MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_NAME_EMPTY, b ? MF_ENABLED|MF_BYCOMMAND : MF_GRAYED|MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_NAME_SAMEPATH, b ? MF_ENABLED|MF_BYCOMMAND : MF_GRAYED|MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_NAME_ADDPATH, b ? MF_ENABLED|MF_BYCOMMAND : MF_GRAYED|MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_NAME_ADDPATH_BACK, b ? MF_ENABLED|MF_BYCOMMAND : MF_GRAYED|MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_NAME_DELPOS, b ? MF_ENABLED|MF_BYCOMMAND : MF_GRAYED|MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_NAME_DELTOKEN, b ? MF_ENABLED|MF_BYCOMMAND : MF_GRAYED|MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_EXT_ADD, b ? MF_ENABLED|MF_BYCOMMAND : MF_GRAYED|MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_EXT_DEL, b ? MF_ENABLED|MF_BYCOMMAND : MF_GRAYED|MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_EXT_REPLACE, b ? MF_ENABLED|MF_BYCOMMAND : MF_GRAYED|MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_EXPORT_CLIP, b ? MF_ENABLED|MF_BYCOMMAND : MF_GRAYED|MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_EXPORT_FILE, b ? MF_ENABLED|MF_BYCOMMAND : MF_GRAYED|MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_EXPORT_CLIP2, b ? MF_ENABLED|MF_BYCOMMAND : MF_GRAYED|MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_EXPORT_FILE2, b ? MF_ENABLED|MF_BYCOMMAND : MF_GRAYED|MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_IMPORT_FILE, b ? MF_ENABLED|MF_BYCOMMAND : MF_GRAYED|MF_BYCOMMAND);

	m_tool1.GetToolBarCtrl().EnableButton(IDM_APPLY_CHANGE, b);
	m_tool2.GetToolBarCtrl().EnableButton(IDM_CLEAR_LIST, b);
	m_tool2.GetToolBarCtrl().EnableButton(IDM_SORT_LIST, b);
	m_tool2.GetToolBarCtrl().EnableButton(IDM_INIT_LIST, b);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_NAME_REPLACE, b);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_NAME_ADD_FRONT, b);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_NAME_ADD_REAR, b);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_NAME_NUMBER_ONLY, b);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_NAME_DIGIT, b);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_NAME_ADDNUM, b);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_NAME_EMPTY, b);
	m_tool2.GetToolBarCtrl().EnableButton(IDM_NAME_SAMEPATH, b);	
	m_tool2.GetToolBarCtrl().EnableButton(IDM_NAME_ADDPATH, b);
	m_tool2.GetToolBarCtrl().EnableButton(IDM_NAME_ADDPATH_BACK, b);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_NAME_DELPOS, b);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_NAME_DELTOKEN, b);
	m_tool2.GetToolBarCtrl().EnableButton(IDM_EXT_ADD, b);
	m_tool2.GetToolBarCtrl().EnableButton(IDM_EXT_DEL, b);
	m_tool2.GetToolBarCtrl().EnableButton(IDM_EXT_REPLACE, b);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_EXPORT_CLIP, b);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_EXPORT_FILE, b);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_EXPORT_CLIP2, b);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_EXPORT_FILE2, b);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_IMPORT_FILE, b);

	b=(m_list.GetNextItem(-1, LVNI_SELECTED)!=-1);
	GetMenu()->EnableMenuItem(IDM_MANUAL_CHANGE, b ? MF_ENABLED|MF_BYCOMMAND : MF_GRAYED|MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_EDIT_UP, b ? MF_ENABLED|MF_BYCOMMAND : MF_GRAYED|MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_EDIT_DOWN, b ? MF_ENABLED|MF_BYCOMMAND : MF_GRAYED|MF_BYCOMMAND);
	m_tool2.GetToolBarCtrl().EnableButton(IDM_MANUAL_CHANGE, b);

	GetMenu()->CheckMenuItem(IDM_SHOW_FULLPATH, 
			FlagGET(m_nShowFlag, COL_FULLPATH) ? MF_CHECKED|MF_BYCOMMAND : MF_UNCHECKED|MF_BYCOMMAND);
	GetMenu()->CheckMenuItem(IDM_SHOW_SIZE, 
			FlagGET(m_nShowFlag, COL_FILESIZE) ? MF_CHECKED|MF_BYCOMMAND : MF_UNCHECKED|MF_BYCOMMAND);
	GetMenu()->CheckMenuItem(IDM_SHOW_MODIFYTIME, 
			FlagGET(m_nShowFlag, COL_TIMEMODIFY) ? MF_CHECKED|MF_BYCOMMAND : MF_UNCHECKED|MF_BYCOMMAND);
	GetMenu()->CheckMenuItem(IDM_SHOW_CREATETIME, 
			FlagGET(m_nShowFlag, COL_TIMECREATE) ? MF_CHECKED|MF_BYCOMMAND : MF_UNCHECKED|MF_BYCOMMAND);

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

}

void CDarkNamerDlg::UpdateCount(int nCount)
{
	CString strTemp;
	if (nCount==0) 
	{
		strTemp.Empty();
		UpdateMenu();
	}
	else strTemp.Format(_T("%d ��"), nCount);
	SetDlgItemText(IDC_ST_BAR, strTemp);
}
