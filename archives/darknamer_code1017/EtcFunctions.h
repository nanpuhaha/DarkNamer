BOOL FlagGET(int& nFlagType, int nField);
void FlagSET(int& nFlagType, int nField, BOOL bSet);
int GetFileImageIndex(CString strPath, BOOL bIsDirectory);
int CompareFileName(TCHAR* name1, TCHAR* name2);
int GetLine(CString& strText, int nPos, CString& strLine, CString strToken);
BOOL WriteCStringToFile(CString strFile, CString& strContent);
BOOL ReadFileToCString(CString strFile, CString& strData);
CString IDSTR(int nID);
