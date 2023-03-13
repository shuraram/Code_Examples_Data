//////////////////////////////////////////////////////////////////////
//                                                                  //
//   SyncData_d.h - �������� �������� � ��������������              //
//                                                                  //
//                                                                  //
//////////////////////////////////////////////////////////////////////



#if !defined(__SYNCDATA_D_H__)
#define __SYNCDATA_D_H__

#include <QReadWriteLock>


//                        ���� ������

#define RC_OK              0                 //  ��� ������

#define RC_NOSYNCMASS      10020101          //  ��� ������� � ��������������

#define RC_LIMSYNCMASS     10020200          //  ����������� ������/������
                                             //  ������� � ��������������

#define RC_NOREADSYNCMASS  10020200          //  ����������� ������
                                             //  ������� � ��������������

#define RC_ERRRWSYNCMASS   10020202          //  ������ ������/������
                                             //  ������� � ��������������

#define RC_WAITTIMEOUT     10090000          //  ������� ����� ��������

#define RC_NOWRITESYNCMASS 10020300          //  ������ ������ �������
                                             //  � ��������������

#define RC_NOBUFRING       10020102          //  ��� ���������� ������

#define RC_EMPTYBUFRING    10020202          //  ��������� ����� ����

#define RC_NOSYNCBUF       10020103          //  ��� ���������� ������
                                             //  � ��������������

#define RC_EMPTYSYNCBUF    10020203          //  ��������� ����� �
                                             //  �������������� ����

#define RC_LIMSYNCBUF      10020204          //  ����������� ������ �� ����������
                                             //  ������ � ��������������

#define RC_NOREADSYNCBUF   10020205          //  ������ ������ �� ����������
                                             //  ������ � ��������������

#define RC_NOWRITESYNCBUF  10020301          //  ������ ������ � ���������
                                             //  ����� � ��������������


///  �������� ������ � ��������������
class SyncMasQt
      {
      public:
      ///  �����������
      SyncMasQt (int nMax,              //  ����� ��������� �������
                 int nSize);            //  ������ ��������, ����

      ///  ����������
      ~SyncMasQt ();

      ///  ������ ������ �� �������
      //  ���������� RC_OK ��� ��� ������
      int Read (void*  lpvBuf,          //  ���� ��������� ������
                int nOffst = 0,         //  ������� � ������ ��������
                int nLen   = 1,         //  ������� ��������� ��������
                int* pnLast = 0,        //  ������ ���������� ��������
                DWORD dwTO = INFINITE); //  ����� ��������, ��

      ///  ������ ������ � ������
      //  ���������� RC_OK ��� ��� ������
      int Write (void*  lpvBuf,         //  ������ ����� ������
                 int nOffst = 0,        //  ������� � ������ ��������
                 int nLen   = 1,        //  ������� ��������� ��������
                 int* pnLast = 0,       //  ������ ���������� ��������
                 DWORD dwTO  = 300);    //  ����� ��������, ��

      ///  ������ ���� �� ��������� � ���� ������� ������� ��������
      //  ���������� RC_OK ��� ��� ������
      int WriteStr (void*  lpvBuf, 	//  ����� ��������� - ���������
                    void*  lpvField,	//  ����� ���� � ���� ���������
                    int nLenF,		//  ������ ����, ����
                    int nOffst = 0,    	//  � ����� ������� ��������
                    int* pnLast= 0, 	//  ������ ���������� ��������
                    DWORD dwTO = 300);  //  ����� ��������, ��

      private:
      void*   lpvMass;                  //  ����� �������
      int     nSize;                    //  ������ ��������, ����
      int     nLast;                    //  ������ ���������� ��������
      int     nMax;                     //  ����. ����� ���������
      QReadWriteLock* pSyncro;          //  ������ �������������
      DWORD   dwBlock;                  //  ������� ����������
      };


template <class T>
///  ���������� ������
class PROSYN
      {
      public:
      ///  �����������
      PROSYN  (int nMax = 1)       //  ������ �������
              {pMas = new SyncMasQt (nMax, sizeof(T));}

      ///  ����������
      ~PROSYN ()
              {delete pMas;}

      ///  ��������� ������ - ��� �������� ��. SyncMasQt::Write
      int set (T*  pData,          //  �������� ������
               int nPos = 0,       //  � ����� ������� �������
               int nNum = 1)       //  ������� ���������
              {return pMas->Write (pData, nPos, nNum);}

      ///  �������� ������ - ��� �������� ��. SyncMasQt::Read
      int get (T*  pData,          //  �������� ������
               int nPos = 0,       //  �� ������ �������� �������
               int nNum = 1)       //  ������� ���������
              {return pMas->Read (pData, nPos, nNum);}

      ///  ��������� ���� ��������� - ��� �������� ��. SyncMasQt::WriteStr
      int setsf (T*    pData,      //  �������� ������ - ���������
                 void* pvField,    //  ����� ���� ���������
                 int   nLenF,      //  ������ ����, ����
                 int   nPos = 0)   //  � ����� ������� �������
                {return pMas->WriteStr (pData, pvField, nLenF, nPos);}

      private:
      SyncMasQt* pMas;
      };


///  ��������� �����
class BuffRing
      {
      public:
      ///  �����������
      BuffRing(UINT uMax,        //  ����� ������, ���������
               UINT uSizeU);     //  ������ ��������, ����

      ///  ����������
      ~BuffRing();

      ///  �������� ����� ������
      UINT GetLenBuf() { return uBufLen; }

      ///  �������� ������� �����
      UINT GetSpaceBuf() { return uDelWR; }

      ///  �������� �������
      //  ���������� RC_OK ��� ��� ������
      int  Write(void* lpvData); //  ������ ����� ������

      ///  ��������� �������
      //  ���������� RC_OK ��� ��� ������
      int  Read(void* lpvData);  //  ���� ��������� ������

      private:
      UINT    uBufLen;       //  ����� ������, ���������
      UINT    uSize;         //  ������ ��������, ����
      void*   lpvStart;      //  ������ ������
      void*   lpvEnd;        //  ����� ������
      void*   lpvRead;       //  ����� ������
      void*   lpvWrite;      //  ����� ������
      UINT    uDelWR;        //  �������� ������� ������ - ������, ���������
      };


///  ��������� ����� � ��������������
class SyncBufQt
      {
      public:
      ///  �����������
      SyncBufQt(int nMax,              //  ����� ������, ���������
                int nSize);            //  ������ ��������, ����

      ///  ����������
      ~SyncBufQt();

      ///  ������ ������ �� ������
      //  ���������� RC_OK ��� ��� ������
      int Read(void*  lpvBuf,          //  ���� ��������� ������
               int nLen = 1,         //  ������� ��������� ��������
               DWORD dwTO = INFINITE); //  ����� ��������, ��

      ///  ������ ������ � �����
      //  ���������� RC_OK ��� ��� ������
      int Write(void*  lpvBuf,         //  ������ ����� ������
                int nLen = 1,        //  ������� ��������� ��������
                DWORD dwTO = INFINITE);//  ����� ��������, ��

      ///  ������ ������ ������
      //  ���������� RC_OK ��� ��� ������
      int GetLenBuf(UINT* puLenBuf,         //  ����� ������, ���������
                    DWORD dwTO = INFINITE); //  ����� ��������, ��

      ///  ������ �������� ������
      //  ���������� RC_OK ��� ��� ������
      int GetSpaceBuf(UINT* puSpace,        //  ������� �����, ���������
                      DWORD dwTO = INFINITE);//  ����� ��������, ��

      private:
      UINT uSize;                       //  ������ ��������, ����
      BYTE byFlag;                      //  ���� �������
      BuffRing* pBuf;                   //  ��������� �����
      QReadWriteLock* pSyncro;          //  ������ �������������
      DWORD   dwBlock;                  //  ������� ����������
      };


#endif // !defined(__SYNCDATA_D_H__)
