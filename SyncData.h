//////////////////////////////////////////////////////////////////////
//                                                                  //
//   SyncData_d.h - описания массивов с синхронизацией              //
//                                                                  //
//                                                                  //
//////////////////////////////////////////////////////////////////////



#if !defined(__SYNCDATA_D_H__)
#define __SYNCDATA_D_H__

#include <QReadWriteLock>


//                        Коды ответа

#define RC_OK              0                 //  нет ошибки

#define RC_NOSYNCMASS      10020101          //  нет массива с синхронизацией

#define RC_LIMSYNCMASS     10020200          //  ограничение чтения/записи
                                             //  массива с синхронизацией

#define RC_NOREADSYNCMASS  10020200          //  ограничение чтения
                                             //  массива с синхронизацией

#define RC_ERRRWSYNCMASS   10020202          //  ошибка чтения/записи
                                             //  массива с синхронизацией

#define RC_WAITTIMEOUT     10090000          //  истекло время ожидания

#define RC_NOWRITESYNCMASS 10020300          //  ошибка записи массива
                                             //  с синхронизацией

#define RC_NOBUFRING       10020102          //  нет кольцевого буфера

#define RC_EMPTYBUFRING    10020202          //  кольцевой буфер пуст

#define RC_NOSYNCBUF       10020103          //  нет кольцевого буфера
                                             //  с синхронизацией

#define RC_EMPTYSYNCBUF    10020203          //  кольцевой буфер с
                                             //  синхронизацией пуст

#define RC_LIMSYNCBUF      10020204          //  ограничение чтения из кольцевого
                                             //  буфера с синхронизацией

#define RC_NOREADSYNCBUF   10020205          //  ошибка чтения из кольцевого
                                             //  буфера с синхронизацией

#define RC_NOWRITESYNCBUF  10020301          //  ошибка записи в кольцевой
                                             //  буфер с синхронизацией


///  Линейный массив с синхронизацией
class SyncMasQt
      {
      public:
      ///  Конструктор
      SyncMasQt (int nMax,              //  число элементов массива
                 int nSize);            //  размер элемента, байт

      ///  Деструктор
      ~SyncMasQt ();

      ///  Чтение данных из массива
      //  Возвращает RC_OK или код ошибки
      int Read (void*  lpvBuf,          //  куда поместить данные
                int nOffst = 0,         //  начиная с какого элемента
                int nLen   = 1,         //  сколько элементов прочесть
                int* pnLast = 0,        //  индекс последнего элемента
                DWORD dwTO = INFINITE); //  время ожидания, мс

      ///  Запись данных в массив
      //  Возвращает RC_OK или код ошибки
      int Write (void*  lpvBuf,         //  откуда брать данные
                 int nOffst = 0,        //  начиная с какого элемента
                 int nLen   = 1,        //  сколько элементов записать
                 int* pnLast = 0,       //  индекс последнего элемента
                 DWORD dwTO  = 300);    //  время ожидания, мс

      ///  Запись поля из структуры в один элемент массива структур
      //  Возвращает RC_OK или код ошибки
      int WriteStr (void*  lpvBuf, 	//  адрес структуры - источника
                    void*  lpvField,	//  адрес поля в этой структуре
                    int nLenF,		//  размер поля, байт
                    int nOffst = 0,    	//  в какой элемент записать
                    int* pnLast= 0, 	//  индекс последнего элемента
                    DWORD dwTO = 300);  //  время ожидания, мс

      private:
      void*   lpvMass;                  //  адрес массива
      int     nSize;                    //  размер элемента, байт
      int     nLast;                    //  индекс последнего элемента
      int     nMax;                     //  макс. число элементов
      QReadWriteLock* pSyncro;          //  объект синхронизации
      DWORD   dwBlock;                  //  признак блокировки
      };


template <class T>
///  Защищенные данные
class PROSYN
      {
      public:
      ///  Конструктор
      PROSYN  (int nMax = 1)       //  размер массива
              {pMas = new SyncMasQt (nMax, sizeof(T));}

      ///  Деструктор
      ~PROSYN ()
              {delete pMas;}

      ///  Сохранить данные - код возврата см. SyncMasQt::Write
      int set (T*  pData,          //  источник данных
               int nPos = 0,       //  в какой элемент массива
               int nNum = 1)       //  сколько элементов
              {return pMas->Write (pData, nPos, nNum);}

      ///  Получить данные - код возврата см. SyncMasQt::Read
      int get (T*  pData,          //  приемник данных
               int nPos = 0,       //  из какого элемента массива
               int nNum = 1)       //  сколько элементов
              {return pMas->Read (pData, nPos, nNum);}

      ///  Сохранить поле структуры - код возврата см. SyncMasQt::WriteStr
      int setsf (T*    pData,      //  источник данных - структура
                 void* pvField,    //  адрес поля структуры
                 int   nLenF,      //  размер поля, байт
                 int   nPos = 0)   //  в какой элемент массива
                {return pMas->WriteStr (pData, pvField, nLenF, nPos);}

      private:
      SyncMasQt* pMas;
      };


///  Кольцевой буфер
class BuffRing
      {
      public:
      ///  Конструктор
      BuffRing(UINT uMax,        //  объем буфера, элементов
               UINT uSizeU);     //  размер элемента, байт

      ///  Деструктор
      ~BuffRing();

      ///  Получить объем буфера
      UINT GetLenBuf() { return uBufLen; }

      ///  Получить занятый объем
      UINT GetSpaceBuf() { return uDelWR; }

      ///  Записать элемент
      //  Возвращает RC_OK или код ошибки
      int  Write(void* lpvData); //  откуда взять данные

      ///  Прочитать элемент
      //  Возвращает RC_OK или код ошибки
      int  Read(void* lpvData);  //  куда поместить данные

      private:
      UINT    uBufLen;       //  объем буфера, элементов
      UINT    uSize;         //  размер элемента, байт
      void*   lpvStart;      //  начало буфера
      void*   lpvEnd;        //  конец буфера
      void*   lpvRead;       //  фронт чтения
      void*   lpvWrite;      //  фронт записи
      UINT    uDelWR;        //  разность фронтов чтения - записи, элементов
      };


///  Кольцевой буфер с синхронизацией
class SyncBufQt
      {
      public:
      ///  Конструктор
      SyncBufQt(int nMax,              //  объем буфера, элементов
                int nSize);            //  размер элемента, байт

      ///  Деструктор
      ~SyncBufQt();

      ///  Чтение данных из буфера
      //  Возвращает RC_OK или код ошибки
      int Read(void*  lpvBuf,          //  куда поместить данные
               int nLen = 1,         //  сколько элементов прочесть
               DWORD dwTO = INFINITE); //  время ожидания, мс

      ///  Запись данных в буфер
      //  Возвращает RC_OK или код ошибки
      int Write(void*  lpvBuf,         //  откуда брать данные
                int nLen = 1,        //  сколько элементов записать
                DWORD dwTO = INFINITE);//  время ожидания, мс

      ///  Запрос объема буфера
      //  Возвращает RC_OK или код ошибки
      int GetLenBuf(UINT* puLenBuf,         //  объем буфера, элементов
                    DWORD dwTO = INFINITE); //  время ожидания, мс

      ///  Запрос занятого объема
      //  Возвращает RC_OK или код ошибки
      int GetSpaceBuf(UINT* puSpace,        //  занятый объем, элементов
                      DWORD dwTO = INFINITE);//  время ожидания, мс

      private:
      UINT uSize;                       //  размер элемента, байт
      BYTE byFlag;                      //  флаг наличия
      BuffRing* pBuf;                   //  кольцевой буфер
      QReadWriteLock* pSyncro;          //  объект синхронизации
      DWORD   dwBlock;                  //  признак блокировки
      };


#endif // !defined(__SYNCDATA_D_H__)
