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
typedef map<CString, int> CExtMap; //확장자에 해당하는 이미지맵의 번호를 기억

#define COL_OLDNAME 0
#define COL_NEWNAME 1
#define COL_FOLDER 2
#define COL_FULLPATH 3
#define COL_FILESIZE 4
#define COL_TIMEMODIFY 5
#define COL_TIMECREATE 6
#define COL_TOTAL 7

static int nPathAddMethod=-1; //0=폴더내의 파일 , 1=폴더 이름 , -1=초기화 안됨
static HIMAGELIST himl_sys;
static CExtMap mapExt;
static int nFolderImage=-1;

inline CString GetFolderString(CString strPath)
{
	return strPath.Right(strPath.GetLength()-strPath.ReverseFind(_T('\\'))-1);
}

inline CString GetTimeStringToShow(FILETIME& dt)
{
	CString strRet;
	SYSTEMTIME systime;
	FileTimeToSystemTime(&dt, &systime);
	strRet.Format(_T("%d-%02d-%02d %02d:%02d:%02d"), systime.wYear,  
	systime.wMonth,  systime.wDay,  systime.wHour, systime.wMinute, systime.wSecond);
	return strRet;
}

inline CString GetTimeStringToAdd(FILETIME& dt)
{
	CString strRet;
	SYSTEMTIME systime;
	FileTimeToSystemTime(&dt, &systime);
	strRet.Format(_T("%d%02d%02d_%02d%02d%02d"), systime.wYear,  
	systime.wMonth,  systime.wDay,  systime.wHour, systime.wMinute, systime.wSecond);
	return strRet;
}

//SHGetFileInfo를 계속 부르다 부면 아이콘과 리소스가 깨지는 현상 발생
//이를 방지하기 위해 특정 확장자에 해당하는 아이콘은 한번만 찾아보도록 
//맵 구조로 저장하고 새로운 아이콘에 대해서만 SHGetFileInfo를 호출 
//시스템 이미지 리스트는 SHGetFileInfo호출시 동적으로 구성됨에 주의
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
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_nShowFlag=0; //칼럼 표시 여부
	FlagSET(m_nShowFlag, COL_OLDNAME, TRUE);
	FlagSET(m_nShowFlag, COL_NEWNAME, TRUE);
	FlagSET(m_nShowFlag, COL_FOLDER, TRUE);
	m_nSortFlag=0; //칼럼별 정렬 방식
	m_bSelected=FALSE; //목록 중 항목 선택 여부 감지

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
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_FILE, OnColumnclickListFile)
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
	
	LOGFONT lf;
	this->GetFont()->GetLogFont(&lf);
	lf.lfHeight = 11;
	m_fontToolBar.CreateFontIndirect(&lf);

	m_tool1.CreateEx(this, TBSTYLE_FLAT | TBSTYLE_LIST, WS_CHILD | WS_VISIBLE | CBRS_LEFT
		| CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
	m_tool1.LoadToolBar(IDR_TOOLBAR1);
	m_tool1.SetFont(&m_fontToolBar, TRUE);

	m_tool2.CreateEx(this, TBSTYLE_FLAT | TBSTYLE_LIST, WS_CHILD | WS_VISIBLE | CBRS_LEFT
		| CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
	m_tool2.LoadToolBar(IDR_TOOLBAR2);
	m_tool2.SetFont(&m_fontToolBar, TRUE);

	for (int i=0; i<10; i++) 
	{
		m_tool1.SetButtonText(i, IDSTR(IDS_TB_00+i));
		m_tool2.SetButtonText(i, IDSTR(IDS_TB_10+i));
	}

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_list.InsertColumn(COL_OLDNAME,_T("현재이름"));
	m_list.InsertColumn(COL_NEWNAME,_T("바꿀이름"));
	m_list.InsertColumn(COL_FOLDER,_T("파일위치"));
	m_list.InsertColumn(COL_FULLPATH,_T("전체경로")); 
	m_list.InsertColumn(COL_FILESIZE,_T("파일크기"), LVCFMT_RIGHT); 
	m_list.InsertColumn(COL_TIMEMODIFY,_T("변경시각")); 
	m_list.InsertColumn(COL_TIMECREATE,_T("생성시각")); 

	m_list.SetColumnWidth(COL_OLDNAME,180);
	m_list.SetColumnWidth(COL_NEWNAME,180);
	m_list.SetColumnWidth(COL_FOLDER,100);
	m_list.SetColumnWidth(COL_FULLPATH,0);
	m_list.SetColumnWidth(COL_FILESIZE,0);
	m_list.SetColumnWidth(COL_TIMEMODIFY,0);
	m_list.SetColumnWidth(COL_TIMECREATE,0);
	
	UpdateMenu();
	UpdateCount();
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

#define TOOLWIDTH 100
#define BARHEIGHT 20
void CDarkNamerDlg::ArrangeCtrl()
{
	CRect rc;
	GetClientRect(rc);
	m_tool1.MoveWindow(0,0,TOOLWIDTH, rc.Height()-BARHEIGHT);
	m_tool2.MoveWindow(rc.right-TOOLWIDTH,0,TOOLWIDTH, rc.Height()-BARHEIGHT);
	m_list.MoveWindow(TOOLWIDTH, 0, rc.Width()-TOOLWIDTH*2, rc.Height()-BARHEIGHT);
	GetDlgItem(IDC_ST_BAR)->MoveWindow(0,rc.bottom-BARHEIGHT+1,rc.Width(),BARHEIGHT-2);
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
	case IDM_UNDO_CHANGE:		UndoChanges();		break;
	case IDM_SORT_LIST:			SortList();		break;
	case IDM_NAME_REPLACE:		NameReplace();	break;
	case IDM_NAME_ADD_FRONT:	NameAdd(TRUE);	break;
	case IDM_NAME_ADD_REAR:		NameAdd(FALSE);	break;
	case IDM_NAME_DEL_TYPE:		NameDelType();	break;
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
	case IDM_VERSION:	AfxMessageBox(_T("DarkNamer 18.08.30 Version")); break;
	default:
		return CDialog::OnCommand(wParam, lParam);
	}
	UpdateMenu();
	return TRUE;
}

BOOL CDarkNamerDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	//주로 단축키의 처리
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
				UpdateCount();
			}
			return TRUE;
		}
		// <>를 이용해 리스트상에서 이동 가능
		if (pMsg->wParam==188)	{ListUp();return TRUE;}
		if (pMsg->wParam==190)	{ListDown();return TRUE;} 
	}
	
	if (pMsg->message==WM_KEYUP && (GetKeyState(VK_CONTROL)&0xFF00)!=0)
	{
		if (pMsg->wParam==_T('O')) {AddByFileDialog(); return TRUE;}
		if (m_list.GetItemCount()>0)
		{
			if (pMsg->wParam==_T('S')) {ApplyChange(); return TRUE;}
			else if (pMsg->wParam==_T('Z')) {UndoChanges(); return TRUE;}
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

	BOOL b=(m_list.GetNextItem(-1, LVNI_SELECTED)!=-1);
	if (b!=m_bSelected)
	{
		GetMenu()->EnableMenuItem(IDM_MANUAL_CHANGE, b ? MF_ENABLED|MF_BYCOMMAND : MF_GRAYED|MF_BYCOMMAND);
		GetMenu()->EnableMenuItem(IDM_EDIT_UP, b ? MF_ENABLED|MF_BYCOMMAND : MF_GRAYED|MF_BYCOMMAND);
		GetMenu()->EnableMenuItem(IDM_EDIT_DOWN, b ? MF_ENABLED|MF_BYCOMMAND : MF_GRAYED|MF_BYCOMMAND);
		m_tool2.GetToolBarCtrl().EnableButton(IDM_MANUAL_CHANGE, b);
		m_tool2.GetToolBarCtrl().EnableButton(IDM_EDIT_UP, b);
		m_tool2.GetToolBarCtrl().EnableButton(IDM_EDIT_DOWN, b);
		m_bSelected=b;
	}
	
	return CDialog::PreTranslateMessage(pMsg);
}

//인자로 받은 파일의 정보를 읽어서 아이템을 인자로 받은 어레이에 추가한다.
//파일인 경우는 단순 추가하면 되지만 디렉토리의 경우는 다르게 처리
inline void AddFileItem(CListItemArray* aFile, CListItem* pItem)
{
	//추가 소트가 필요없도록 역순으로 비교해서 삽입한다
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
	if (bIsDirectory) //폴더인 경우의 처리 
	{
		//폴더 추가 방법 묻기
		if (nPathAddMethod==-1) //맨 처음(-1일때)에만 물어본다
		{
			if (AfxGetMainWnd()->MessageBox(_T("경로를 직접 추가려면 YES, 경로내 파일을 추가하려면 NO 선택."), strFile, MB_YESNO)==IDNO) 
				nPathAddMethod=0;
			else nPathAddMethod=1;
		}

		if (nPathAddMethod==0) //폴더 내의 파일들을 추가하기로 선택한 경우 
		{
			CFileFind find;
			if (find.FindFile(strFile+_T("\\*.*"))==FALSE) return;
			while (TRUE)
			{
				BOOL b=find.FindNextFile();
				//.과 ..을 제외 
				CString strName=find.GetFileName();
				if (strName!=_T(".") && strName!=_T(".."))
				{
					if (find.IsDirectory()==TRUE) //폴더인 경우 재귀호출
					{
						AddFile(aFile, find.GetFilePath(), find.IsDirectory()); 
					}
					else //폴더 내 파일의 경우 CFileFind로 이미 오픈되어 있으므로 활용한다.
					{
						CListItem* pItem=new CListItem; //아이템 생성
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
			return; //복귀
		}
		//else if (nPathAddMethod==1) {} //폴더 이름을 직접 추가하기로 선택한 경우는 단순 파일과 동일하게 처리		
	} 

	//실제로 파일을 추가한다
	//유니코드 지원을 위해서는 CreateFile/OPEN_EXISTING 으로 파일 핸들을 연다
	HANDLE hfile=(HANDLE)CreateFile((LPCTSTR)strFile,0,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS,NULL);
	if (hfile==INVALID_HANDLE_VALUE) return;
	CListItem* pItem=new CListItem; //아이템 생성
	memset(pItem, 0, sizeof(CListItem));
	_tcscpy(pItem->strPath, strFile);
	pItem->bIsDirectory=bIsDirectory;
	GetFileTime(hfile, &(pItem->timeCreated), NULL, &(pItem->timeModified));
	pItem->dwSize=GetFileSize(hfile, NULL);
	CloseHandle(hfile);
	AddFileItem(aFile, pItem);
}

// Drag & Drop 을 사용해서 파일을 목록에 추가한다 
void CDarkNamerDlg::OnDropFiles(HDROP hDropInfo) 
{
    WORD cFiles;
    TCHAR szFileName[_MAX_PATH];
	memset(szFileName, 0, sizeof(szFileName));
	CString strFile, strPath, strName, strTemp;
    cFiles = DragQueryFile(hDropInfo, (UINT)-1, NULL, 0);

	CListItemArray aFile; //생성된 CListItem을 저장

	int i;
	nPathAddMethod=-1; //폴더의 처리 방법 결정 변수
	SetDlgItemText(IDC_ST_BAR, IDSTR(IDS_WORKING));
	for (i=0; i<cFiles; i++)
	{
		DragQueryFile(hDropInfo, i, szFileName, sizeof(szFileName));

		strFile = (LPCTSTR)szFileName;
		//현재 리스트 중에 같은 이름이 있는지 확인한다
		BOOL b=TRUE;
		for (int j=0; j<m_list.GetItemCount(); j++)
		{
			if (strFile.CompareNoCase( ((CListItem*)m_list.GetItemData(j))->strPath )==0) 
			{
				b=FALSE;
				break;
			}
		}
		//같은 이름이 없는 경우 존재여부 확인 겸 디렉토리 여부 판정
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
	
	//리스트에 파일을 추가한다
	m_list.SetRedraw(FALSE);
	CListItemArray::iterator it=aFile.begin();
	while (it!=aFile.end())	{FileListAdd(*(it)); it++;};
	m_list.SetRedraw(TRUE);

	UpdateMenu();
}

//파일 열기 시스템 다이얼로그를 사용해서 파일을 목록에 추가한다
void CDarkNamerDlg::AddByFileDialog()
{
	CFileDialog dlg(TRUE, _T("*.*"), NULL, 
		OFN_ALLOWMULTISELECT|OFN_FILEMUSTEXIST|OFN_ENABLESIZING|OFN_LONGNAMES|OFN_HIDEREADONLY ,_T("All Files(*.*)|*.*||"),NULL);

	TCHAR *buf = new TCHAR[65536];           
	memset(buf, 0, sizeof(buf));
	dlg.m_ofn.lpstrTitle=_T("이름 붙일 파일 불러오기");
	dlg.m_ofn.lpstrFile = buf;    
	dlg.m_ofn.nMaxFile = 65536;  

	if (dlg.DoModal()==IDOK)
	{
		CListItemArray aFile; //생성된 CListItem을 저장
		nPathAddMethod=-1; //폴더의 처리 방법 결정 변수

		SetDlgItemText(IDC_ST_BAR, IDSTR(IDS_WORKING));
		CString strFile;
		POSITION pos=dlg.GetStartPosition();
		while(pos)
		{
			strFile = dlg.GetNextPathName(pos);
			//현재 리스트 중에 같은 이름이 있는지 확인한다
			BOOL b=TRUE;
			for (int j=0; j<m_list.GetItemCount(); j++)
			{
				if (strFile.CompareNoCase( ((CListItem*)m_list.GetItemData(j))->strPath )==0) 
				{
					b=FALSE;
					break;
				}
			}
			//같은 이름이 없는 경우 존재여부 확인 겸 디렉토리 여부 판정
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
		//리스트에 파일을 추가한다
		m_list.SetRedraw(FALSE);
		CListItemArray::iterator it=aFile.begin();
		while (it!=aFile.end())	{FileListAdd(*(it)); it++;};
		m_list.SetRedraw(TRUE);
	}
	delete[] buf;
}

//파일을 리스트컨트롤에 표시하고 연결시킨다
int CDarkNamerDlg::FileListAdd(CListItem* pItem)
{
	CString strPath = Get_Path(pItem->strPath);
	CString strName = Get_Name(pItem->strPath, TRUE);
	CString strExt = Get_Ext(pItem->strPath, pItem->bIsDirectory);

	int nItem;
	nItem=m_list.InsertItem(m_list.GetItemCount(), strName, GetExtIcon(strExt, pItem->bIsDirectory));
	//필요한 정보가 다 들어있는 객체 포인터를 아이템 데이터로
	m_list.SetItemData(nItem, (DWORD)pItem);

	//기본적으로 표시되어야 하는 정보 : 원래 이름(Insert시)과 바뀔 이름, 존재하는 경로
	m_list.SetItemText(nItem, COL_NEWNAME, strName);
	m_list.SetItemText(nItem, COL_FOLDER, strPath);

	//선택적으로 표시할 정보
	CString strTemp;

	if (FlagGET(m_nShowFlag,COL_FULLPATH)) 
	{
		//전체 경로 
		m_list.SetItemText(nItem, COL_FULLPATH, pItem->strPath);
	}
	
	if (FlagGET(m_nShowFlag,COL_FILESIZE))
	{
		//파일 크기 
		strTemp.Format(_T("%d"), pItem->dwSize);
		m_list.SetItemText(nItem, COL_FILESIZE, strTemp);
	}
	
	if (FlagGET(m_nShowFlag,COL_TIMEMODIFY))
	{
		//수정된 시각
		m_list.SetItemText(nItem, COL_TIMEMODIFY, GetTimeStringToShow(pItem->timeModified));
	}

	if (FlagGET(m_nShowFlag,COL_TIMECREATE))
	{
		//생성된 시각
		m_list.SetItemText(nItem, COL_TIMECREATE, GetTimeStringToShow(pItem->timeCreated));
	}

	UpdateCount();
	return nItem;
}


//리스트 모두 삭제
void CDarkNamerDlg::ClearList()
{
	//리스트에 연결된 아이템들은 new로 생성된 것이므로 모두 delete 한다
	for (int i=0; i<m_list.GetItemCount(); i++)
	{
		CListItem* pItem=(CListItem*)m_list.GetItemData(i);
		delete pItem;
	}
	m_list.DeleteAllItems();
	m_bSelected=FALSE;
	UpdateCount();
}

//바뀔 이름을 원래 이름으로 다시 복구
void CDarkNamerDlg::UndoChanges()
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
	dlg.InitInputDlg(m_list.GetItemText(n,COL_NEWNAME)+_T(" 를"), _T("으로"), _T(""));
	if (dlg.DoModal()==IDOK)
	{
		m_list.SetItemText(n, COL_NEWNAME, dlg.m_strReturn1);
	}
}

void CDarkNamerDlg::NameReplace()
{
	CDlgInput dlg;
	dlg.InitInputDlg(_T("이름에 들어있는 문자열을 바꿉니다."), _T("를"), _T("으로"));

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

void CDarkNamerDlg::NameAdd(BOOL bFront=TRUE)
{
	CDlgInput dlg;
	dlg.InitInputDlg(IDSTR(bFront ? IDS_ADDFRONT : IDS_ADDBACK), IDSTR(IDS_ADDSTRING), _T(""));
	dlg.AddOption(_T("직접 입력한 문자열 추가"), INPUT_ONE);
	dlg.AddOption(_T("파일이 들어있는 폴더명 추가"), INPUT_NONE);
	dlg.AddOption(_T("파일 변경 일시 추가"), INPUT_NONE);
	dlg.AddOption(_T("파일 생성 일시 추가"), INPUT_NONE);

	if (dlg.DoModal()==IDCANCEL) return;
	CString strTemp;
	CListItem* pItem;
	m_list.SetRedraw(FALSE);
	for (int i=0; i<m_list.GetItemCount(); i++)
	{
		switch (dlg.m_nCB)
		{
		case 0: //직접 입력
			strTemp=dlg.m_strReturn1;
			break;
		case 1: //폴더명
			strTemp = GetFolderString(m_list.GetItemText(i, COL_FOLDER)); 
			//c:, d: 등 드라이브 루트 경로인 경우 추가히지 않음
			if (strTemp.CompareNoCase(m_list.GetItemText(i, COL_FOLDER))==0) strTemp.Empty(); 
			break;
		case 2: //변경일시
			pItem=(CListItem*)m_list.GetItemData(i);
			strTemp = GetTimeStringToAdd(pItem->timeModified);
			break;
		case 3: //생성일시
			pItem=(CListItem*)m_list.GetItemData(i);
			strTemp = GetTimeStringToAdd(pItem->timeCreated);
			break;
		}
		if (bFront)
		{
			strTemp += m_list.GetItemText(i, COL_NEWNAME);
			m_list.SetItemText(i, COL_NEWNAME, strTemp);
		}
		else
		{
			NameAppend(i, strTemp);
		}
	}
	m_list.SetRedraw(TRUE);
}

//inline 함수. 이름 맨 뒤, 확장자 바로 앞에 특정 문자열을 삽입해주는 기능
void CDarkNamerDlg::NameAppend(int nItem, CString strAppend)
{
	CListItem* pItem=(CListItem*)m_list.GetItemData(nItem);
	CString strName=Get_Name(m_list.GetItemText(nItem,COL_NEWNAME), pItem->bIsDirectory)+strAppend;
	CString	strExt=Get_Ext(m_list.GetItemText(nItem,COL_NEWNAME), pItem->bIsDirectory);
	if (strExt.IsEmpty()==FALSE) strName+=strExt;
	m_list.SetItemText(nItem, COL_NEWNAME, strName);
}

void CDarkNamerDlg::NameDelType()
{
	CDlgInput dlg;
	dlg.InitInputDlg(_T("선택에 따라 숫자를 남기거나 삭제합니다."), _T(""), _T(""));
	dlg.AddOption(_T("숫자만 남기기"), INPUT_NONE);
	dlg.AddOption(_T("숫자를 모두 삭제하기"), INPUT_NONE);

	if (dlg.DoModal()==IDCANCEL) return;
	//이름에서 숫자만 남기기
	CString strName, strExt;
	m_list.SetRedraw(FALSE);
	for (int i=0; i<m_list.GetItemCount(); i++)
	{
		CListItem* pItem=(CListItem*)m_list.GetItemData(i);

		strName=Get_Name(m_list.GetItemText(i, COL_NEWNAME), pItem->bIsDirectory);
		strExt=Get_Ext(m_list.GetItemText(i, COL_NEWNAME), pItem->bIsDirectory);

		for (int j=strName.GetLength()-1; j>=0; j--)
		{
			if (dlg.m_nCB == 0)  // 숫자만 남기기
			{
				if (_T('0')>strName.GetAt(j) || strName.GetAt(j)>_T('9'))  strName.Delete(j);
			}
			else // 숫자를 지우기
			{
				if (_T('0')<=strName.GetAt(j) && strName.GetAt(j)<=_T('9'))  strName.Delete(j);
			}
		}
		if (strExt.IsEmpty()==FALSE) strName+=strExt;
		m_list.SetItemText(i, COL_NEWNAME, strName);
	}
	m_list.SetRedraw(TRUE);
}

void CDarkNamerDlg::NameDigit()
{
	CDlgInput dlg;
	dlg.InitInputDlg(_T("숫자부분의 자리수를 맞춰 0을 붙입니다."), _T("자리수"), _T(""));
	dlg.AddOption(_T("제일 뒷번호 맞춤"), INPUT_ONE);
	dlg.AddOption(_T("제일 앞번호 맞춤"), INPUT_ONE);

	if (dlg.DoModal()==IDCANCEL) return;
	int nDigit=_ttoi(dlg.m_strReturn1);
	if (nDigit<=0) 
	{
		AfxMessageBox(_T("자리수 입력이 잘못되었습니다."));
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
		if (dlg.m_nCB==0) //뒷번호
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
		else //if (dlg.m_nCB==1) //앞번호
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

void CDarkNamerDlg::ExtDel() //확장자 삭제
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
	dlg.InitInputDlg(_T("확장자를 뒤에 붙입니다."), _T("붙일 확장자"), _T(""));
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
	dlg.InitInputDlg(_T("확장자를 바꿔 줍니다."), _T("바꿀 확장자"),_T(""));

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

//실제 파일 시스템상의 정보를 바꿔 파일 이름 변경하기
void CDarkNamerDlg::ApplyChange()
{
	if (AfxMessageBox(_T("실제 파일 이름을 변경하시겠습니까?"), MB_OKCANCEL)==IDCANCEL) return;
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
			AfxMessageBox(_T("이름이 지정되지 않은 경우가 있습니다."));
			m_list.SetFocus();
			m_list.SetItemState(i,LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
			m_list.EnsureVisible(i, FALSE);
			return;
		}

		strNewPath=m_list.GetItemText(i,COL_FOLDER)+_T("\\")+strTemp;
		//중복되는 이름이 있는지 검사한다
		CStrArray::iterator it=aAfter.begin();
		int j=0;
		while (it!=aAfter.end())
		{
			if (strNewPath.CompareNoCase(*(it))==0) 
			{
				strTemp.Format(_T("중복되는 이름이 있습니다.\n%s"), strNewPath);
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
	if (aAfter.size()!=nCount) {AfxMessageBox(_T("정체불명의 내부 오류 발생."));return;}
	
	//실제 파일이름을 바꾸는 곳
	m_list.SetRedraw(FALSE);
	for (i=0; i<nCount; i++)
	{
		CListItem* pItem=(CListItem*)m_list.GetItemData(i);
		try
		{
			if (aAfter.at(i).CompareNoCase(pItem->strPath)!=0) 
			{
				if (MoveFileExW(pItem->strPath, aAfter[i], MOVEFILE_COPY_ALLOWED)==FALSE) 
				{
					strTemp.Format(_T("%s -> %s 변경 실패.\n"), pItem->strPath, aAfter[i]);
					strLog+=strTemp;
				}
				else
				{
					//변경이 성공한 경우 리스트에 표시하기 
					_tcscpy(pItem->strPath, aAfter[i]);
					CString strPath = Get_Path(pItem->strPath);
					CString strName = Get_Name(pItem->strPath, TRUE);
					CString strExt	= Get_Ext(pItem->strPath, pItem->bIsDirectory);
					m_list.SetItem(i, COL_OLDNAME, LVIF_TEXT|LVIF_IMAGE ,strName, GetExtIcon(strExt, pItem->bIsDirectory),0,0,0);
					m_list.SetItemText(i, COL_NEWNAME, strName);
					m_list.SetItemText(i, COL_FOLDER, strPath);
					if (FlagGET(m_nShowFlag, COL_FULLPATH)) m_list.SetItemText(i, COL_FULLPATH, pItem->strPath);				
				}
			}
		}
		catch (CFileException *e)
		{
			e->ReportError();
			e->Delete();
			strTemp.Format(_T("%s를 %s로 바꾸는데 실패.\n"), pItem->strPath, aAfter[i]);
			strLog+=strTemp;
		}
	}
	m_list.SetRedraw(TRUE);
	if (strLog.IsEmpty()==FALSE) AfxMessageBox(strLog);
	else						 AfxMessageBox(_T("파일 이름을 변경하였습니다."));
}	

//두개의 아이템 교환
void CDarkNamerDlg::SwapItem(int n1, int n2)
{
	CString str1, str2;
	int nCol=COL_TOTAL, i=0;

	//아이템 데이타 교환
	DWORD dw1=m_list.GetItemData(n1);
	DWORD dw2=m_list.GetItemData(n2);
	m_list.SetItemData(n1, dw2);
	m_list.SetItemData(n2, dw1);

	//아이콘 이미지 교환
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

	//칼럼 정보 교환 
	for (i=0; i<nCol; i++) 
	{
		str1=m_list.GetItemText(n1,i);
		str2=m_list.GetItemText(n2,i);
		m_list.SetItemText(n1, i, str2);
		m_list.SetItemText(n2, i, str1);
	}

	//아이템 상태 교환
	DWORD d1=m_list.GetItemState(n1, LVIS_SELECTED|LVIS_FOCUSED);
	DWORD d2=m_list.GetItemState(n2, LVIS_SELECTED|LVIS_FOCUSED);
	m_list.SetItemState(n1,d2, LVIS_SELECTED|LVIS_FOCUSED);
	m_list.SetItemState(n2,d1, LVIS_SELECTED|LVIS_FOCUSED);

	m_list.EnsureVisible(n2, FALSE);
}

//선택된 리스트 아이템을 한칸 위로 올린다 
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

//선택된 리스트 아이템을 한칸 아래로 내린다 
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
	dlg.InitInputDlg(_T("붙일 숫자의 자리수와 시작값을 지정합니다."), _T("자리수"), _T("시작값"));
	dlg.AddOption(_T("이름뒤에 번호붙임"), INPUT_TWO);
	dlg.AddOption(_T("이름앞에 번호붙임"), INPUT_TWO);
	dlg.AddOption(_T("폴더별로 뒤 번호붙임"), INPUT_TWO);
	dlg.AddOption(_T("폴더별로 앞 번호붙임"), INPUT_TWO);

	if (dlg.DoModal()==IDCANCEL) return;
	int nDigit=_ttoi(dlg.m_strReturn1);
	int nStart=_ttoi(dlg.m_strReturn2);
	if (nDigit<=0) 
	{
		AfxMessageBox(_T("자리수 입력이 잘못되었습니다."));
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
			//폴더별 번호 붙이기 기능
			if (m_list.GetItemText(i-1,COL_FOLDER).CompareNoCase(m_list.GetItemText(i,COL_FOLDER))!=0) nCurrent=nStart;
		}
		CListItem* pItem=(CListItem*)m_list.GetItemData(i);
		strName=Get_Name(m_list.GetItemText(i,COL_NEWNAME), pItem->bIsDirectory);
		strExt=Get_Ext(m_list.GetItemText(i,COL_NEWNAME), pItem->bIsDirectory);
		strTemp.Format(_T("%d"),nCurrent);
		while(nDigit>strTemp.GetLength()) strTemp=_T('0')+strTemp;
		if (dlg.m_nCB==0 || dlg.m_nCB==2)	strName+=strTemp; //뒤에 붙이기
		else								strName=strTemp+strName; //앞에 붙이기
		if (strExt.IsEmpty()==FALSE) strName+=strExt;
		m_list.SetItemText(i, COL_NEWNAME, strName);

		nCurrent++;
	}
	m_list.SetRedraw(TRUE);
}

void CDarkNamerDlg::NameEmpty() //바꿀 이름 부분을 확장자만 제외하고 모두 지운다
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

//nMode=0 : 클립보드로 파일명 저장, nMode=1 : 파일로 파일명 저장
//nMode=2 : 클립보드로 경로명 저장, nMode=3 : 파일로 경로명 저장
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

		if (nMode==1)	dlg.m_ofn.lpstrTitle=_T("파일명 저장");
		else			dlg.m_ofn.lpstrTitle=_T("경로명 저장"); //nMode==3
		if (dlg.DoModal()==IDCANCEL) return;
		WriteCStringToFile(dlg.GetPathName(), strData);
	}
}

//파일명을 텍스트 파일에서 읽어와서 차례대로 새로 바뀔 이름인 COL_NEWNAME 을 수정한다
//텍스트 파일은 이름 하나당 엔터(\n)로 구분해서 저장한 타입
//현재 리스트에 있는 개수만큼 읽어온다
void CDarkNamerDlg::ImportName()
{
	CFileDialog dlg(TRUE, _T("*.txt"), NULL, OFN_ENABLESIZING|OFN_LONGNAMES|OFN_FILEMUSTEXIST|OFN_HIDEREADONLY ,_T("Text Files(*.txt)|*.txt|All Files(*.*)|*.*||"),NULL);
	dlg.m_ofn.lpstrTitle=_T("바꿀 파일 이름 불러오기");
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
			//원래 이름이 존재해야만 하므로 맨 위부터 교체하는 형식
			if (i>=m_list.GetItemCount()) break; //리스트 넘어가면 끝
			m_list.SetItemText(i, COL_NEWNAME, strName);
			i++;
		}
	}
	m_list.SetRedraw(TRUE);
}

//경로명을 텍스트 파일에서 읽어와서 현재 목록 뒤에 추가한다
//텍스트 파일은 이름 하나당 엔터(\n)로 구분해서 저장한 타입
void CDarkNamerDlg::ImportPath()
{
	CFileDialog dlg(TRUE, _T("*.txt"), NULL, OFN_ENABLESIZING|OFN_LONGNAMES|OFN_FILEMUSTEXIST|OFN_HIDEREADONLY ,_T("Text Files(*.txt)|*.txt|All Files(*.*)|*.*||"),NULL);
	dlg.m_ofn.lpstrTitle=_T("파일에서 경로목록 읽어 추가하기");
	if (dlg.DoModal()==IDCANCEL) return;
	CString strImportData;
	ReadFileToCString(dlg.GetPathName(), strImportData);

	CString strFile;
	CString& strData=strImportData;
	int nPos = 0;
	CListItemArray aFile;

	SetDlgItemText(IDC_ST_BAR, IDSTR(IDS_WORKING));
	while(nPos!=-1)
	{
		nPos=GetLine(strData, nPos, strFile, _T("\n"));
		strFile.TrimLeft();strFile.TrimRight();
		if (strFile.IsEmpty()==FALSE)
		{
			//현재 리스트 중에 같은 이름이 있는지 확인한다
			BOOL b=TRUE;
			for (int j=0; j<m_list.GetItemCount(); j++)
			{
				if (strFile.CompareNoCase( ((CListItem*)m_list.GetItemData(j))->strPath )==0) 
				{
					b=FALSE;
					break;
				}
			}
			//같은 이름이 없는 경우 존재여부 확인 겸 디렉토리 여부 판정
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
	//리스트에 파일을 추가한다
	m_list.SetRedraw(FALSE);
	CListItemArray::iterator it=aFile.begin();
	while (it!=aFile.end())	{FileListAdd(*(it)); it++;};
	m_list.SetRedraw(TRUE);
}


//파일의 경로를 하나로 통일. 일종의 MoveFile이 됨. 중복 체크 필요
void CDarkNamerDlg::NameSamePath()
{
	CFolderDialog dlg(_T("경로 선택"));
	if (dlg.DoModal()==IDCANCEL) return;
	CString strPath=dlg.GetPathName();
	//e:\ 같은 경우를 대비하여 끝에 오는 \를 삭제
	if (strPath.GetAt(strPath.GetLength()-1)==_T('\\')) strPath.Delete(strPath.GetLength()-1);
	m_list.SetRedraw(FALSE);
	for (int i=0; i<m_list.GetItemCount(); i++)
	{
		//COL_FOLDER 변경해 주면 나중에 실제 변경때 이값이 COL_NEWNAME과 결합되면서 새 경로가 된다
		m_list.SetItemText(i, COL_FOLDER, strPath); 
	}
	m_list.SetRedraw(TRUE);
}

void CDarkNamerDlg::NameDelPos()
{
	CDlgInput dlg;
	dlg.InitInputDlg(_T("지정위치를 삭제합니다.(첫글자는 1번째)"), _T("번째부터"), _T("번째까지"));
	dlg.AddOption(_T("앞에서부터 삭제"), INPUT_TWO);
	dlg.AddOption(_T("제일 뒤부터 삭제"), INPUT_TWO);
	if (dlg.DoModal()==IDCANCEL) return;

	int nStart=_ttoi(dlg.m_strReturn1);
	int nEnd=_ttoi(dlg.m_strReturn2);
	if (nStart==0 && nEnd==0) return;

	if ( nStart<0 || nEnd<0 )
		{AfxMessageBox(_T("음수값이나 잘못된 값이 입력되었습니다."));return;}
	if ( dlg.m_nCB==0 && nEnd>0 && nStart>nEnd )
		{AfxMessageBox(_T("시작점이 끝점보다 뒤에 있습니다."));return;}
	if ( dlg.m_nCB==1 && nStart!=0 )
		{AfxMessageBox(_T("맨 뒤에서부터 삭제할때는 '~까지'만 필요합니다."));return;}

	CString strName, strExt;

	m_list.SetRedraw(FALSE);
	for (int i=0; i<m_list.GetItemCount(); i++)
	{
		CListItem* pItem=(CListItem*)m_list.GetItemData(i);
		strName=Get_Name(m_list.GetItemText(i, COL_NEWNAME), pItem->bIsDirectory);
		strExt=Get_Ext(m_list.GetItemText(i, COL_NEWNAME), pItem->bIsDirectory);

		if (dlg.m_nCB==0)	//앞의 n부터 m까지
		{
			if (nStart==0) nStart=1;
			int nLen=strName.GetLength();
			if (nStart<=nLen)
			{
				if (nEnd>0 && nEnd<nLen) nLen=nEnd;
				strName.Delete(nStart-1, nLen-nStart+1);
			}
		}
		else if (dlg.m_nCB==1) //뒤의 n개
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
	dlg.InitInputDlg(_T("지정된 문자로 묶인 부분을 삭제합니다."), _T("시작문자"), _T("끝문자"));

	if (dlg.DoModal()==IDCANCEL) return;
	if (dlg.m_strReturn1.IsEmpty() || dlg.m_strReturn2.IsEmpty())
	{
		AfxMessageBox(_T("시작/끝 문자가 정확하게 지정되지 않았습니다."));
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

//리스트를 정해진 기준에 따라 정렬한다
void CDarkNamerDlg::SortList()
{
	CDlgInput dlg;
	dlg.InitInputDlg(_T("정렬 기준 설정"), _T(""), _T(""));
	dlg.AddOption(_T("파일 이름에 따라 오름차순"), INPUT_NONE);
	dlg.AddOption(_T("파일 이름에 따라 내림차순"), INPUT_NONE);
	dlg.AddOption(_T("전체경로에 따라 오름차순"), INPUT_NONE);
	dlg.AddOption(_T("전체경로에 따라 내림차순"), INPUT_NONE);
	dlg.AddOption(_T("파일 크기에 따라 오름차순"), INPUT_NONE);
	dlg.AddOption(_T("파일 크기에 따라 내림차순"), INPUT_NONE);
	dlg.AddOption(_T("수정한 시각에 따라 오름차순"), INPUT_NONE);
	dlg.AddOption(_T("수정한 시각에 따라 내림차순"), INPUT_NONE);
	dlg.AddOption(_T("만든 시각에 따라 오름차순"), INPUT_NONE);
	dlg.AddOption(_T("만든 시각에 따라 내림차순"), INPUT_NONE);

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
	CSortInfo* psi=(CSortInfo*)lParamSort;
	CListItem* pItem1=(CListItem*)lParam1;
	CListItem* pItem2=(CListItem*)lParam2;

	int nOrder;
	//칼럼 종류에 따라 비교 방법 바꾸기
	switch (psi->nSortType)
	{
	case COL_OLDNAME:
		{
			CString str1 = Get_Name(pItem1->strPath, TRUE);
			CString str2 = Get_Name(pItem2->strPath, TRUE);
			nOrder=CompareFileName(str1.GetBuffer(0), str2.GetBuffer(0));
		}
		break;
	case COL_NEWNAME:
		nOrder =0; 
		break;
	case COL_FOLDER:
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

	nOrder=nOrder * psi->nAsc; //오름차순, 내림차순의 반영
	return nOrder; //lParamSort is ASC / DSC
}

void CDarkNamerDlg::UpdateColumn(int nCol)
{
	BOOL bShow=FlagGET(m_nShowFlag, nCol);
	if (bShow)	
	{
		if (nCol==COL_FILESIZE) m_list.SetColumnWidth(nCol, 80);
		else					m_list.SetColumnWidth(nCol, 150);
	}
	else		m_list.SetColumnWidth(nCol, 0);
	if (bShow==FALSE) return;


	int nCount=m_list.GetItemCount();
	//맨 끝 아이템을 검사해서 내용이 이미 채워져 있으면 패스 아니면 새로 채운다
	if (nCount>1)
	{
		if (m_list.GetItemText(nCount-1, nCol).IsEmpty()==FALSE) return;
	}
	
	m_list.SetRedraw(FALSE);
	CString strTemp;
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
			m_list.SetItemText(i, COL_TIMEMODIFY, GetTimeStringToShow(pItem->timeModified));
			break;
		case COL_TIMECREATE: 
			m_list.SetItemText(i, COL_TIMECREATE, GetTimeStringToShow(pItem->timeCreated));
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
	GetMenu()->EnableMenuItem(IDM_UNDO_CHANGE, b ? MF_ENABLED|MF_BYCOMMAND : MF_GRAYED|MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_NAME_REPLACE, b ? MF_ENABLED|MF_BYCOMMAND : MF_GRAYED|MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_NAME_ADD_FRONT, b ? MF_ENABLED|MF_BYCOMMAND : MF_GRAYED|MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_NAME_ADD_REAR, b ? MF_ENABLED|MF_BYCOMMAND : MF_GRAYED|MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_NAME_DEL_TYPE, b ? MF_ENABLED|MF_BYCOMMAND : MF_GRAYED|MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_NAME_DIGIT, b ? MF_ENABLED|MF_BYCOMMAND : MF_GRAYED|MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_NAME_ADDNUM, b ? MF_ENABLED|MF_BYCOMMAND : MF_GRAYED|MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_NAME_EMPTY, b ? MF_ENABLED|MF_BYCOMMAND : MF_GRAYED|MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_NAME_SAMEPATH, b ? MF_ENABLED|MF_BYCOMMAND : MF_GRAYED|MF_BYCOMMAND);
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
	m_tool1.GetToolBarCtrl().EnableButton(IDM_NAME_REPLACE, b);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_NAME_ADD_FRONT, b);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_NAME_ADD_REAR, b);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_NAME_DEL_TYPE, b);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_NAME_DIGIT, b);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_NAME_ADDNUM, b);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_NAME_EMPTY, b);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_NAME_DELPOS, b);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_NAME_DELTOKEN, b);

	m_tool2.GetToolBarCtrl().EnableButton(IDM_CLEAR_LIST, b);
	m_tool2.GetToolBarCtrl().EnableButton(IDM_SORT_LIST, b);
	m_tool2.GetToolBarCtrl().EnableButton(IDM_UNDO_CHANGE, b);
	m_tool2.GetToolBarCtrl().EnableButton(IDM_NAME_SAMEPATH, b);	
	m_tool2.GetToolBarCtrl().EnableButton(IDM_EXT_ADD, b);
	m_tool2.GetToolBarCtrl().EnableButton(IDM_EXT_DEL, b);
	m_tool2.GetToolBarCtrl().EnableButton(IDM_EXT_REPLACE, b);

/*	m_tool1.GetToolBarCtrl().EnableButton(IDM_EXPORT_CLIP, b);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_EXPORT_FILE, b);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_EXPORT_CLIP2, b);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_EXPORT_FILE2, b);
	m_tool1.GetToolBarCtrl().EnableButton(IDM_IMPORT_FILE, b);*/

	b=(m_list.GetNextItem(-1, LVNI_SELECTED)!=-1);
	GetMenu()->EnableMenuItem(IDM_MANUAL_CHANGE, b ? MF_ENABLED|MF_BYCOMMAND : MF_GRAYED|MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_EDIT_UP, b ? MF_ENABLED|MF_BYCOMMAND : MF_GRAYED|MF_BYCOMMAND);
	GetMenu()->EnableMenuItem(IDM_EDIT_DOWN, b ? MF_ENABLED|MF_BYCOMMAND : MF_GRAYED|MF_BYCOMMAND);
	m_tool2.GetToolBarCtrl().EnableButton(IDM_MANUAL_CHANGE, b);
	m_tool2.GetToolBarCtrl().EnableButton(IDM_EDIT_UP, b);
	m_tool2.GetToolBarCtrl().EnableButton(IDM_EDIT_DOWN, b);
	
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

void CDarkNamerDlg::UpdateCount()
{
	int nCount = m_list.GetItemCount();
	CString strTemp;
	if (nCount==0) 
	{
		strTemp = _T("드래그 앤 드롭 또는 메뉴를 이용하여 파일을 추가해 주세요");
		UpdateMenu();
	}
	else strTemp.Format(_T("%d 개"), nCount);
	SetDlgItemText(IDC_ST_BAR, strTemp);
}

void CDarkNamerDlg::OnColumnclickListFile(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	
	int nCol = pNMListView->iSubItem;
	BOOL bShow = FlagGET(m_nShowFlag, nCol);
	BOOL bAsc = FlagGET(m_nSortFlag, nCol);

	if (!bShow)
	{
		FlagSET(m_nShowFlag, nCol, TRUE);
		UpdateColumn(nCol);
	}
	
	if (nCol != COL_NEWNAME)
	{
		CSortInfo si;
		si.plist = &m_list;
		si.nSortType = nCol;
		
		FlagSET(m_nSortFlag, nCol, !bAsc);
		si.nAsc = bAsc ? 1 : -1;

		m_list.SetRedraw(FALSE);
		m_list.SortItems(Compare, (DWORD)&si);
		m_list.SetRedraw(TRUE);
	}
	*pResult = 0;
}
