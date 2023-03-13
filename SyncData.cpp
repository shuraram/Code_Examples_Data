//////////////////////////////////////////////////////////////////////
//                                                                  //
//  SyncData.cpp - ���������� �������� � ��������������             //
//                                                                  //
//                                                                  //
//////////////////////////////////////////////////////////////////////



////////////////   �������� ������ � ��������������   //////////////

//  �����������
SyncMasQt::SyncMasQt (int nMaxUnit,     //  ����� ��������� �������
                      int nSizeUnit)    //  ������ ��������, ����
    {
     //  ������ ������
     lpvMass = 0;                 //  ����� �������
     nSize = nSizeUnit;           //  ������ ��������, ����
     nLast = -1;                  //  ������ ���������� ��������
     nMax = nMaxUnit;             //  ����. ����� ���������
     dwBlock = 0;                 //  ������� ����������

     if (nSize <= 0 || nMax <= 0)
        {
        nSize = nMax = 0;
        return;
        }

     //  ������ ������ ��� �������
     lpvMass = new BYTE [nSize * nMax];
     if (lpvMass == 0)
        {
        return;
        }
     else
        {
        BYTE* pbyM = ((BYTE*)lpvMass) - 1;
        for (int m = 0; m < nSize * nMax; m++)
            *(++pbyM) = 0;
        }

     //  ������������� �������������
     pSyncro = new QReadWriteLock ();
     if (pSyncro != 0)
        return;
     else
        {
        delete (BYTE*)lpvMass;
        lpvMass = 0;
        }
     }


//  ����������
SyncMasQt::~SyncMasQt ()
   {
   //  ��������������� �������������
   if (pSyncro != 0)
      delete pSyncro;

   //  ������������ ������
   delete (BYTE*)lpvMass;
   }


//  ������ ������ �� �������
//  ���������� RC_OK ��� ��� ������
int SyncMasQt::Read (void* lpvBuf,      //  ���� ��������� ������
                     int   nOffset,     //  ������� � ������ ��������
                     int   nLen,        //  ������� ��������� ��������
                     int*  pnLast,      //  ������ ���������� ��������
                     DWORD dwTimeout)   //  ����� ��������, ��
    {
    //  ��� �������
    if (lpvMass == 0)
       return RC_NOSYNCMASS;

    int nRet = RC_OK;
    int nTimeout = dwTimeout;
    if (dwTimeout == INFINITE)
       nTimeout = -1;

    //  ���� ����������� ������
    if (!(pSyncro->tryLockForRead (nTimeout)))
       return RC_WAITTIMEOUT;

    //  ������ ���������� ��������
    if (pnLast != 0)
       *pnLast = nLast;

    //  ������ � ���������� ������
    if (nOffset < 0      || nLen <= 0 ||
        nOffset >= nMax  || lpvBuf == 0)
       {
       //  ������ ��������
       pSyncro->unlock ();
       return RC_ERRRWSYNCMASS;
       }

    int nRead = nLen;
    //  ��������� ������ ������, ��� ���� � �������
    if ((nOffset + nLen - 1) > nLast)
       {
       nRet = RC_LIMSYNCMASS;
       nRead = nLast - nOffset + 1;
       }

    //  �������� ������ �� �������
    if (nRead > 0)
       {
        BYTE* pbyB = ((BYTE*)lpvBuf) - 1;
        BYTE* pbyM = ((BYTE*)lpvMass) + nOffset * nSize - 1;
        for (int m = 0; m < nRead * nSize; m++)
           *(++pbyB) = *(++pbyM);
       }

    //  ������ ��������
    pSyncro->unlock ();

    return nRet;
    }


//  ������ ������ � ������
//  ���������� RC_OK ��� ��� ������
int SyncMasQt::Write (void* lpvBuf,     //  ������ ����� ������
                      int   nOffset,    //  ������� � ������ ��������
                      int   nLen,       //  ������� ��������� ��������
                      int*  pnLast,     //  ������ ���������� ��������
                      DWORD dwTimeout)  //  ����� ��������, ��
    {
    //  ��� �������
    if (lpvMass == 0)
       return RC_NOSYNCMASS;

    int nRet = RC_OK;
    int nTimeout = dwTimeout;
    if (dwTimeout == INFINITE)
       nTimeout = -1;

    //  ���� ����������� ������
    if (!(pSyncro->tryLockForWrite (nTimeout)))
       return RC_WAITTIMEOUT;

    //  ������� ������ ���������� ��������
    if (pnLast != 0)
       *pnLast = nLast;
    //  ������ � ���������� ������
    if (nOffset < 0        || nLen <= 0 ||
        nOffset > nMax - 1 || lpvBuf == 0)
       {
       //  "������" ��������
       dwBlock = 0;
       pSyncro->unlock ();
       return RC_ERRRWSYNCMASS;
       }

    int nWrite = nLen;
    //  ��������� �������� ������, ��� ��������
    if ((nOffset + nLen) > nMax)
       {
       nRet = RC_LIMSYNCMASS;
       nWrite = nMax - nOffset;
       }
    //  �������� ������ � ������
    BYTE* pbyB = ((BYTE*)lpvBuf) - 1;
    BYTE* pbyM = ((BYTE*)lpvMass) + nOffset * nSize - 1;
    for (int m = 0; m < nWrite * nSize; m++)
        *(++pbyM) = *(++pbyB);
    //  ������������ ������ ���������� ��������
    if (nOffset + nWrite - 1 > nLast)
       nLast = nOffset + nWrite - 1;
    //  ����� ������ ���������� ��������
    if (pnLast != 0)
       *pnLast = nLast;

    //  ������ ��������
    dwBlock = 0;
    pSyncro->unlock ();

    return nRet;
    }


//  ������ ���� �� ��������� � ���� ������� ������� ��������
//  ���������� RC_OK ��� ��� ������
int SyncMasQt::WriteStr (void* lpvBuf, 	     //  ����� ��������� - ���������
                         void* lpvField,	//  ����� ���� � ���������
                         int   nLenF,		//  ������ ����, ����
                         int   nOffset,      //  � ����� ������� ��������
                         int*  pnLast,      	//  ������ ���������� ��������
                         DWORD dwTimeout)  	//  ����� ��������, ��
    {
    //  ��� �������
    if (lpvMass == 0)
       return RC_NOSYNCMASS;

    int nRet = RC_OK;
    int nTimeout = dwTimeout;
    if (dwTimeout == INFINITE)
       nTimeout = -1;

    //  ���� ����������� ������
    if (!(pSyncro->tryLockForWrite (nTimeout)))
       return RC_WAITTIMEOUT;

    //  ������� ������ ���������� ��������
    if (pnLast != 0)
       *pnLast = nLast;

    int nDel =(BYTE*)lpvField - (BYTE*)lpvBuf;

    //  ������ � ���������� ������
    if (nOffset < 0 || nDel < 0 || nDel + nLenF > nSize ||
        nLenF <= 0  || nOffset > nMax - 1 || lpvBuf == 0)
       {
       //  "������" ��������
       dwBlock = 0;
       pSyncro->unlock ();
       return RC_ERRRWSYNCMASS;
       }

    //  �������� ������
    BYTE* pbyF = ((BYTE*)lpvField) - 1;
    BYTE* pbyM = ((BYTE*)lpvMass) + nOffset * nSize + nDel - 1;
    for (int m = 0; m < nLenF; m++)
        *(++pbyM) = *(++pbyF);
    //  ������������ ������ ���������� ��������
    if (nOffset > nLast)
       nLast = nOffset;
    //  ����� ������ ���������� ��������
    if (pnLast != 0)
       *pnLast = nLast;

    //  ������ ��������
    dwBlock = 0;
    pSyncro->unlock ();

    return nRet;
    }

//////////////////////////////////////////////////////////////////////


/////////////////////   ��������� �����   ////////////////////////////


//  �����������
BuffRing::BuffRing(UINT uMax,          //  ����� ������, ���������
                   UINT uSizeU)        //  ������ ��������, ����
   {
    //  ������ ������
    uBufLen = 0;
    uSize = 0;
    uDelWR = 0;
    lpvStart = lpvEnd = lpvRead = lpvWrite = NULL;

    //  ������ � ����������
    if (uMax == 0 || uSizeU == 0)
        return;

    uBufLen = uMax;
    uSize = uSizeU;

    //  ������ ������ ��� ������
    lpvStart = new BYTE[uSize * uBufLen];
    if (lpvStart == 0)
    {
        return;
    }
    else
    {
        //             ZeroMemory (lpvStart, uSize * uBufLen);
        BYTE* pbyM = ((BYTE*)lpvStart) - 1;
        for (UINT m = 0; m < uSize * uBufLen; m++)
            *(++pbyM) = 0;
    }

    //  ����� ����
    lpvRead = lpvWrite = lpvStart;
    lpvEnd = (BYTE*)lpvStart + uBufLen * uSize;
   }

//  ����������
BuffRing::~BuffRing()
   {
    if (lpvStart != 0)
        delete (BYTE*)lpvStart;

    lpvStart = 0;
   }

//  �������� �������
//  ���������� RC_OK ��� ��� ������
int BuffRing::Write(void*  lpvData)    //  ������ ����� ������
   {
    //  ��� ������
    if (lpvStart == 0)
        return RC_NOBUFRING;

    //  �������� �������
    //    CopyMemory (lpvWrite, lpvData, uSize);
    BYTE* pbyB = ((BYTE*)lpvWrite) - 1;
    BYTE* pbyM = ((BYTE*)lpvData) - 1;
    for (UINT m = 0; m < uSize; m++)
        *(++pbyB) = *(++pbyM);

    //  �������� ����� ������
    if (uDelWR < uBufLen)     //  ���� ������ �����
    {
        lpvWrite = (BYTE*)lpvWrite + uSize;
        //  ������� �� ������
        if ((BYTE*)lpvWrite == (BYTE*)lpvEnd)
            lpvWrite = lpvStart;
        uDelWR++;
    }
    else                      //  ��� ������� �����
    {
        lpvRead = (BYTE*)lpvRead + uSize;
        //  ������� �� ������
        if ((BYTE*)lpvRead == (BYTE*)lpvEnd)
            lpvRead = lpvStart;
        lpvWrite = lpvRead;
    }

    return RC_OK;
   }

//  ��������� �������
//  ���������� RC_OK ��� ��� ������
int BuffRing::Read(void*  lpvData)     //  ���� ��������� ������
   {
    //  ��� ������
    if (lpvStart == 0)
        return RC_NOBUFRING;

    //  ������ � ���������
    if (lpvData == 0)
        return RC_NOREADSYNCBUF;

    //  ����� ����
    if (uDelWR == 0)
        return RC_EMPTYBUFRING;

    //  �������� �������
    //    CopyMemory (lpvData, lpvRead, uSize);
    BYTE* pbyB = ((BYTE*)lpvData) - 1;
    BYTE* pbyM = ((BYTE*)lpvRead) - 1;
    for (UINT m = 0; m < uSize; m++)
        *(++pbyB) = *(++pbyM);

    //  �������� ����� ������
    lpvRead = (BYTE*)lpvRead + uSize;
    //  ������� �� ������
    if ((BYTE*)lpvRead == (BYTE*)lpvEnd)
        lpvRead = lpvStart;
    uDelWR--;

    return RC_OK;
   }


//////////   ��������� ����� � ��������������   ///////////

//  �����������
SyncBufQt::SyncBufQt (int nMax,              //  ����� ������, ���������
                      int nSize)             //  ������ ��������, ����
    {
    //  ������ ������
    byFlag = TRUE;
    uSize  = nSize;
    dwBlock = 0;                      //  ������� ����������

    //  ������� ������ "��������� �����"
    pBuf = new BuffRing (nMax, nSize);

    //  �������������� ������������
    pSyncro = new QReadWriteLock ();
    if (pSyncro != 0)
       return;
    else
       {
       byFlag = FALSE;
       delete pBuf;
       }
    }



//  ����������
SyncBufQt::~SyncBufQt ()
    {
    if (byFlag)
       {
       //  ���������������� ������������
       if (pSyncro != 0)
          delete pSyncro;
       //  ������� ��������� �����
       delete pBuf;
       }
    }


//  ������ ������ �� ������
//  ���������� RC_OK ��� ��� ������
int SyncBufQt::Read (void* lpvBuf,      //  ���� ��������� ������
                     int   nLen,        //  ������� ��������� ��������
                     DWORD dwTimeout)   //  ����� ��������, ��
    {
    //  ��� ������
    if (!byFlag)
       return RC_NOSYNCBUF;

    //  ������ � ���������� ������
    if (lpvBuf == 0 || nLen <= 0)
       return RC_NOREADSYNCBUF;

    int nRet = RC_OK;
    int nTimeout = dwTimeout;
    if (dwTimeout == INFINITE)
       nTimeout = -1;

    //  ���� ����������� ������
    if (!(pSyncro->tryLockForRead (nTimeout)))
       return RC_WAITTIMEOUT;

    //  ����� ����
    if (pBuf->GetSpaceBuf () == 0)
       nRet = RC_EMPTYSYNCBUF;
    else
       for (int i = 0; i < nLen; i++)
           {
           //  ������ �������� �� ������
           if (pBuf->Read ((BYTE*)lpvBuf + i * uSize) != RC_OK)
              {
              nRet =  RC_LIMSYNCBUF;
              break;
              }
           }

    //  ������ ��������
    pSyncro->unlock ();

    return nRet;
    }


//  ������ ������ � �����
//  ���������� RC_OK ��� ��� ������
int SyncBufQt::Write (void* lpvBuf,     //  ������ ����� ������
                      int   nLen,       //  ������� ��������� ��������
                      DWORD dwTimeout)  //  ����� ��������, ��
    {
    //  ��� ������
    if (!byFlag)
       return RC_NOSYNCBUF;

    //  ������ � ���������� ������
    if (lpvBuf == 0 || nLen <= 0)
       {
       return RC_NOWRITESYNCBUF;
       }

    int nRet = RC_OK;
    int nTimeout = dwTimeout;
    if (dwTimeout == INFINITE)
       nTimeout = -1;

    //  ���� ����������� ��������
    if (!(pSyncro->tryLockForWrite (nTimeout)))
       return RC_WAITTIMEOUT;

    //  ������ � �����
    for (int i = 0; i < nLen; i++)
        {
        pBuf->Write ((BYTE*)lpvBuf + i * uSize);
        }

    //  ������ ��������
    dwBlock = 0;
    pSyncro->unlock ();

    return nRet;
    }


//  ������ ������ ������
//  ���������� RC_OK ��� ��� ������
int SyncBufQt::GetLenBuf (UINT* puLenBuf,    //  ����� ������, ���������
                          DWORD dwTimeout)   //  ����� ��������, ��
    {
    //  ��� ������
    if (!byFlag)
       return RC_NOSYNCBUF;

    int nTimeout = dwTimeout;
    if (dwTimeout == INFINITE)
       nTimeout = -1;

    //  ���� ����������� ������
    if (!(pSyncro->tryLockForRead (nTimeout)))
       return RC_WAITTIMEOUT;

    //  ����� ���������� ������
    *puLenBuf = pBuf->GetLenBuf ();

    //  ������ ��������
    pSyncro->unlock ();

    return RC_OK;
    }


//  ������ �������� ������
//  ���������� RC_OK ��� ��� ������
int SyncBufQt::GetSpaceBuf (UINT* puSpace,   //  ������� �����, ���������
                            DWORD dwTimeout) //  ����� ��������, ��
    {
    //  ��� ������
    if (!byFlag)
       return RC_NOSYNCBUF;

    int nTimeout = dwTimeout;
    if (dwTimeout == INFINITE)
       nTimeout = -1;

    //  ���� ����������� ������
    if (!(pSyncro->tryLockForRead (nTimeout)))
       return RC_WAITTIMEOUT;

    //  ������� ������������ � ��������� ������
    *puSpace = pBuf->GetSpaceBuf ();

    //  ������ ��������
    pSyncro->unlock ();

    return RC_OK;
    }

//////////////////////////////////////////////////////////////////////

