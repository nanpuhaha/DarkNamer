#include "stdafx.h"
#include "EtcFunctions.h"

BOOL FlagGET(int& nFlagType, int nField)
{
	int i=0, flag=1;
	for (i=0;i<nField; i++) flag = flag * 2;
	
	if ((nFlagType & flag)!=0) return TRUE;
	return FALSE;
}

void FlagSET(int& nFlagType, int nField, BOOL bSet)
{
	int i=0, flag=1;
	for (i=0;i<nField; i++) flag = flag * 2;

	if (bSet==FALSE)
	{
		flag=0xFFFFFFFF^flag;
		nFlagType=nFlagType & flag;
	}
	if (bSet==TRUE)
	{
		nFlagType=nFlagType | flag;
	}
}


//해당 파일의 아이콘 정보를 가져온다
int GetFileImageIndex(CString strPath, BOOL bIsDirectory)
{
	SHFILEINFO sfi;
	SHGetFileInfo((LPCTSTR)strPath, bIsDirectory ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL,
		&sfi, sizeof(SHFILEINFO), SHGFI_USEFILEATTRIBUTES|SHGFI_SYSICONINDEX|SHGFI_ICON|SHGFI_SMALLICON);
//	return bIsDirectory ? 4 : 3;
	return sfi.iIcon;
}

//두 파일명 비교하기 : Locale 고려 비교 기능
int CompareFileName(TCHAR* name1, TCHAR* name2)
{
	int len1=_tcslen(name1);
	int len2=_tcslen(name2);
	int nRet=CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, name1, len1, name2, len2);
	if (nRet==CSTR_LESS_THAN) return -1;
	if (nRet==CSTR_GREATER_THAN) return 1;
	return 0; //nRet==CSTR_EQUAL
}

//파일내의 텍스트 처리를 위한 함수들
int GetLine(CString& strText, int nPos, CString& strLine, CString strToken)
{
	if (strText.IsEmpty()) {strLine.Empty();return -1;}
	int nPosNext = -1;
	
	if (strToken.GetLength()==1)
	{
		TCHAR c=strToken.GetAt(0);
		nPosNext=strText.Find(c,nPos);
	}
	else
	{
		nPosNext=strText.Find(strToken,nPos);
	}

	if (nPosNext!=-1)
	{
		strLine = strText.Mid(nPos,nPosNext-nPos);
		nPosNext+=strToken.GetLength();
	}
	else
	{
		nPosNext= strText.GetLength();
		strLine = strText.Mid(nPos,nPosNext-nPos);
		return -1;
	}
	return nPosNext;
}

BOOL WriteCStringToFile(CString strFile, CString& strContent)
{
	try
	{
		CFile file;
		if (file.Open(strFile, CFile::modeCreate|CFile::modeWrite)==FALSE) return FALSE;
#ifdef _UNICODE
		BYTE UnicodeIdentifier[] = {0xff,0xfe};
		file.Write(UnicodeIdentifier, 2);
#endif 
		if (strContent.IsEmpty()==FALSE)
		{
			file.Write(strContent.GetBuffer(0), strContent.GetLength()*sizeof(TCHAR));
			strContent.ReleaseBuffer();
		}
		file.Flush();
		file.Close();
	}
	catch(CFileException* e)
	{
		e->Delete(); 
		return FALSE;
	}
	return TRUE;
}

BOOL ReadFileToCString(CString strFile, CString& strData)
{
	//Unicode 식별해서 읽기
	try
	{
		CFile file;
		if (file.Open(strFile, CFile::modeRead)==FALSE) return FALSE;
		size_t filesize = file.GetLength();
		if (filesize > 2)
		{
			BYTE uidf[2];
			file.Read(uidf, 2);
			if (uidf[0]==0xff && uidf[1]==0xfe)	
			{
				filesize-=2;
#ifdef _UNICODE
				int nStrLen = ( filesize / sizeof(TCHAR) ) + 1;
				TCHAR* pBuf=strData.GetBufferSetLength(nStrLen);
				memset(pBuf, 0, filesize + sizeof(TCHAR));
				file.Read(pBuf, filesize);
				strData.ReleaseBuffer();
				file.Close();
#else
				int nStrLen = ( filesize / sizeof(WCHAR) ) + 1;
				WCHAR* pBuf=new WCHAR[nStrLen];
				memset(pBuf, 0, filesize + sizeof(WCHAR));
				file.Read(pBuf, filesize);
				file.Close();
				strData=pBuf;
				delete[] pBuf;
#endif 
			}
			else								
			{
				file.SeekToBegin();
				char* pBuf=new char[filesize + 1];
				memset(pBuf, 0, filesize + 1);
				file.Read(pBuf, filesize);
				strData = pBuf;	
				file.Close();
				delete[] pBuf;
			}
		}
	}
	catch(CFileException* e)
	{
		e->Delete(); 
		return FALSE;
	}
	return TRUE;
}

CString IDSTR(int nID)
{
	CString strRet;
	strRet.LoadString(nID);
	return strRet;
}