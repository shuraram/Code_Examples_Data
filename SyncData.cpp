//////////////////////////////////////////////////////////////////////
//                                                                  //
//  SyncData.cpp - реализация массивов с синхронизацией             //
//                                                                  //
//                                                                  //
//////////////////////////////////////////////////////////////////////



////////////////   Линейный массив с синхронизацией   //////////////

//  Конструктор
SyncMasQt::SyncMasQt (int nMaxUnit,     //  число элементов массива
                      int nSizeUnit)    //  размер элемента, байт
    {
     //  Данные класса
     lpvMass = 0;                 //  адрес массива
     nSize = nSizeUnit;           //  размер элемента, байт
     nLast = -1;                  //  индекс последнего элемента
     nMax = nMaxUnit;             //  макс. число элементов
     dwBlock = 0;                 //  признак блокировки

     if (nSize <= 0 || nMax <= 0)
        {
        nSize = nMax = 0;
        return;
        }

     //  Запрос памяти для массива
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

     //  Инициализация синхрообъекта
     pSyncro = new QReadWriteLock ();
     if (pSyncro != 0)
        return;
     else
        {
        delete (BYTE*)lpvMass;
        lpvMass = 0;
        }
     }


//  Деструктор
SyncMasQt::~SyncMasQt ()
   {
   //  Деинициализация синхрообъекта
   if (pSyncro != 0)
      delete pSyncro;

   //  Освобождение памяти
   delete (BYTE*)lpvMass;
   }


//  Чтение данных из массива
//  Возвращает RC_OK или код ошибки
int SyncMasQt::Read (void* lpvBuf,      //  куда поместить данные
                     int   nOffset,     //  начиная с какого элемента
                     int   nLen,        //  сколько элементов прочесть
                     int*  pnLast,      //  индекс последнего элемента
                     DWORD dwTimeout)   //  время ожидания, мс
    {
    //  Нет массива
    if (lpvMass == 0)
       return RC_NOSYNCMASS;

    int nRet = RC_OK;
    int nTimeout = dwTimeout;
    if (dwTimeout == INFINITE)
       nTimeout = -1;

    //  Ждем возможности чтения
    if (!(pSyncro->tryLockForRead (nTimeout)))
       return RC_WAITTIMEOUT;

    //  Индекс последнего элемента
    if (pnLast != 0)
       *pnLast = nLast;

    //  Ошибки в параметрах чтения
    if (nOffset < 0      || nLen <= 0 ||
        nOffset >= nMax  || lpvBuf == 0)
       {
       //  Чтение окончено
       pSyncro->unlock ();
       return RC_ERRRWSYNCMASS;
       }

    int nRead = nLen;
    //  Требуется читать больше, чем есть в массиве
    if ((nOffset + nLen - 1) > nLast)
       {
       nRet = RC_LIMSYNCMASS;
       nRead = nLast - nOffset + 1;
       }

    //  Копируем данные из массива
    if (nRead > 0)
       {
        BYTE* pbyB = ((BYTE*)lpvBuf) - 1;
        BYTE* pbyM = ((BYTE*)lpvMass) + nOffset * nSize - 1;
        for (int m = 0; m < nRead * nSize; m++)
           *(++pbyB) = *(++pbyM);
       }

    //  Чтение окончено
    pSyncro->unlock ();

    return nRet;
    }


//  Запись данных в массив
//  Возвращает RC_OK или код ошибки
int SyncMasQt::Write (void* lpvBuf,     //  откуда брать данные
                      int   nOffset,    //  начиная с какого элемента
                      int   nLen,       //  сколько элементов записать
                      int*  pnLast,     //  индекс последнего элемента
                      DWORD dwTimeout)  //  время ожидания, мс
    {
    //  Нет массива
    if (lpvMass == 0)
       return RC_NOSYNCMASS;

    int nRet = RC_OK;
    int nTimeout = dwTimeout;
    if (dwTimeout == INFINITE)
       nTimeout = -1;

    //  Ждем возможности записи
    if (!(pSyncro->tryLockForWrite (nTimeout)))
       return RC_WAITTIMEOUT;

    //  Прежний индекс последнего элемента
    if (pnLast != 0)
       *pnLast = nLast;
    //  Ошибки в параметрах записи
    if (nOffset < 0        || nLen <= 0 ||
        nOffset > nMax - 1 || lpvBuf == 0)
       {
       //  "Запись" окончена
       dwBlock = 0;
       pSyncro->unlock ();
       return RC_ERRRWSYNCMASS;
       }

    int nWrite = nLen;
    //  Требуется записать больше, чем возможно
    if ((nOffset + nLen) > nMax)
       {
       nRet = RC_LIMSYNCMASS;
       nWrite = nMax - nOffset;
       }
    //  Копируем данные в массив
    BYTE* pbyB = ((BYTE*)lpvBuf) - 1;
    BYTE* pbyM = ((BYTE*)lpvMass) + nOffset * nSize - 1;
    for (int m = 0; m < nWrite * nSize; m++)
        *(++pbyM) = *(++pbyB);
    //  Корректируем индекс последнего элемента
    if (nOffset + nWrite - 1 > nLast)
       nLast = nOffset + nWrite - 1;
    //  Новый индекс последнего элемента
    if (pnLast != 0)
       *pnLast = nLast;

    //  Запись окончена
    dwBlock = 0;
    pSyncro->unlock ();

    return nRet;
    }


//  Запись поля из структуры в один элемент массива структур
//  Возвращает RC_OK или код ошибки
int SyncMasQt::WriteStr (void* lpvBuf, 	     //  адрес структуры - источника
                         void* lpvField,	//  адрес поля в структуре
                         int   nLenF,		//  размер поля, байт
                         int   nOffset,      //  в какой элемент записать
                         int*  pnLast,      	//  индекс последнего элемента
                         DWORD dwTimeout)  	//  время ожидания, мс
    {
    //  Нет массива
    if (lpvMass == 0)
       return RC_NOSYNCMASS;

    int nRet = RC_OK;
    int nTimeout = dwTimeout;
    if (dwTimeout == INFINITE)
       nTimeout = -1;

    //  Ждем возможности записи
    if (!(pSyncro->tryLockForWrite (nTimeout)))
       return RC_WAITTIMEOUT;

    //  Прежний индекс последнего элемента
    if (pnLast != 0)
       *pnLast = nLast;

    int nDel =(BYTE*)lpvField - (BYTE*)lpvBuf;

    //  Ошибки в параметрах записи
    if (nOffset < 0 || nDel < 0 || nDel + nLenF > nSize ||
        nLenF <= 0  || nOffset > nMax - 1 || lpvBuf == 0)
       {
       //  "Запись" окончена
       dwBlock = 0;
       pSyncro->unlock ();
       return RC_ERRRWSYNCMASS;
       }

    //  Копируем данные
    BYTE* pbyF = ((BYTE*)lpvField) - 1;
    BYTE* pbyM = ((BYTE*)lpvMass) + nOffset * nSize + nDel - 1;
    for (int m = 0; m < nLenF; m++)
        *(++pbyM) = *(++pbyF);
    //  Корректируем индекс последнего элемента
    if (nOffset > nLast)
       nLast = nOffset;
    //  Новый индекс последнего элемента
    if (pnLast != 0)
       *pnLast = nLast;

    //  Запись окончена
    dwBlock = 0;
    pSyncro->unlock ();

    return nRet;
    }

//////////////////////////////////////////////////////////////////////


/////////////////////   КОЛЬЦЕВОЙ БУФЕР   ////////////////////////////


//  Конструктор
BuffRing::BuffRing(UINT uMax,          //  объем буфера, элементов
                   UINT uSizeU)        //  размер элемента, байт
   {
    //  Данные класса
    uBufLen = 0;
    uSize = 0;
    uDelWR = 0;
    lpvStart = lpvEnd = lpvRead = lpvWrite = NULL;

    //  Ошибка в параметрах
    if (uMax == 0 || uSizeU == 0)
        return;

    uBufLen = uMax;
    uSize = uSizeU;

    //  Запрос памяти для буфера
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

    //  Буфер пуст
    lpvRead = lpvWrite = lpvStart;
    lpvEnd = (BYTE*)lpvStart + uBufLen * uSize;
   }

//  Деструктор
BuffRing::~BuffRing()
   {
    if (lpvStart != 0)
        delete (BYTE*)lpvStart;

    lpvStart = 0;
   }

//  Записать элемент
//  Возвращает RC_OK или код ошибки
int BuffRing::Write(void*  lpvData)    //  откуда взять данные
   {
    //  Нет буфера
    if (lpvStart == 0)
        return RC_NOBUFRING;

    //  Записать элемент
    //    CopyMemory (lpvWrite, lpvData, uSize);
    BYTE* pbyB = ((BYTE*)lpvWrite) - 1;
    BYTE* pbyM = ((BYTE*)lpvData) - 1;
    for (UINT m = 0; m < uSize; m++)
        *(++pbyB) = *(++pbyM);

    //  Сместить фронт записи
    if (uDelWR < uBufLen)     //  есть пустое место
    {
        lpvWrite = (BYTE*)lpvWrite + uSize;
        //  Переход по кольцу
        if ((BYTE*)lpvWrite == (BYTE*)lpvEnd)
            lpvWrite = lpvStart;
        uDelWR++;
    }
    else                      //  нет пустого места
    {
        lpvRead = (BYTE*)lpvRead + uSize;
        //  Переход по кольцу
        if ((BYTE*)lpvRead == (BYTE*)lpvEnd)
            lpvRead = lpvStart;
        lpvWrite = lpvRead;
    }

    return RC_OK;
   }

//  Прочитать элемент
//  Возвращает RC_OK или код ошибки
int BuffRing::Read(void*  lpvData)     //  куда поместить данные
   {
    //  Нет буфера
    if (lpvStart == 0)
        return RC_NOBUFRING;

    //  Ошибка в параметре
    if (lpvData == 0)
        return RC_NOREADSYNCBUF;

    //  Буфер пуст
    if (uDelWR == 0)
        return RC_EMPTYBUFRING;

    //  Прочесть элемент
    //    CopyMemory (lpvData, lpvRead, uSize);
    BYTE* pbyB = ((BYTE*)lpvData) - 1;
    BYTE* pbyM = ((BYTE*)lpvRead) - 1;
    for (UINT m = 0; m < uSize; m++)
        *(++pbyB) = *(++pbyM);

    //  Сместить фронт чтения
    lpvRead = (BYTE*)lpvRead + uSize;
    //  Переход по кольцу
    if ((BYTE*)lpvRead == (BYTE*)lpvEnd)
        lpvRead = lpvStart;
    uDelWR--;

    return RC_OK;
   }


//////////   Кольцевой буфер с синхронизацией   ///////////

//  Конструктор
SyncBufQt::SyncBufQt (int nMax,              //  объем буфера, элементов
                      int nSize)             //  размер элемента, байт
    {
    //  Данные класса
    byFlag = TRUE;
    uSize  = nSize;
    dwBlock = 0;                      //  признак блокировки

    //  Создаем объект "кольцевой буфер"
    pBuf = new BuffRing (nMax, nSize);

    //  Инициализируем синхрообъект
    pSyncro = new QReadWriteLock ();
    if (pSyncro != 0)
       return;
    else
       {
       byFlag = FALSE;
       delete pBuf;
       }
    }



//  Деструктор
SyncBufQt::~SyncBufQt ()
    {
    if (byFlag)
       {
       //  Деинициализируем синхрообъект
       if (pSyncro != 0)
          delete pSyncro;
       //  Удаляем кольцевой буфер
       delete pBuf;
       }
    }


//  Чтение данных из буфера
//  Возвращает RC_OK или код ошибки
int SyncBufQt::Read (void* lpvBuf,      //  куда поместить данные
                     int   nLen,        //  сколько элементов прочесть
                     DWORD dwTimeout)   //  время ожидания, мс
    {
    //  Нет буфера
    if (!byFlag)
       return RC_NOSYNCBUF;

    //  Ошибки в параметрах чтения
    if (lpvBuf == 0 || nLen <= 0)
       return RC_NOREADSYNCBUF;

    int nRet = RC_OK;
    int nTimeout = dwTimeout;
    if (dwTimeout == INFINITE)
       nTimeout = -1;

    //  Ждем возможности чтения
    if (!(pSyncro->tryLockForRead (nTimeout)))
       return RC_WAITTIMEOUT;

    //  Буфер пуст
    if (pBuf->GetSpaceBuf () == 0)
       nRet = RC_EMPTYSYNCBUF;
    else
       for (int i = 0; i < nLen; i++)
           {
           //  Читаем элементы из буфера
           if (pBuf->Read ((BYTE*)lpvBuf + i * uSize) != RC_OK)
              {
              nRet =  RC_LIMSYNCBUF;
              break;
              }
           }

    //  Чтение окончено
    pSyncro->unlock ();

    return nRet;
    }


//  Запись данных в буфер
//  Возвращает RC_OK или код ошибки
int SyncBufQt::Write (void* lpvBuf,     //  откуда брать данные
                      int   nLen,       //  сколько элементов записать
                      DWORD dwTimeout)  //  время ожидания, мс
    {
    //  Нет буфера
    if (!byFlag)
       return RC_NOSYNCBUF;

    //  Ошибки в параметрах записи
    if (lpvBuf == 0 || nLen <= 0)
       {
       return RC_NOWRITESYNCBUF;
       }

    int nRet = RC_OK;
    int nTimeout = dwTimeout;
    if (dwTimeout == INFINITE)
       nTimeout = -1;

    //  Ждем возможности записать
    if (!(pSyncro->tryLockForWrite (nTimeout)))
       return RC_WAITTIMEOUT;

    //  Запись в буфер
    for (int i = 0; i < nLen; i++)
        {
        pBuf->Write ((BYTE*)lpvBuf + i * uSize);
        }

    //  Запись окончена
    dwBlock = 0;
    pSyncro->unlock ();

    return nRet;
    }


//  Запрос объема буфера
//  Возвращает RC_OK или код ошибки
int SyncBufQt::GetLenBuf (UINT* puLenBuf,    //  объем буфера, элементов
                          DWORD dwTimeout)   //  время ожидания, мс
    {
    //  Нет буфера
    if (!byFlag)
       return RC_NOSYNCBUF;

    int nTimeout = dwTimeout;
    if (dwTimeout == INFINITE)
       nTimeout = -1;

    //  Ждем возможности чтения
    if (!(pSyncro->tryLockForRead (nTimeout)))
       return RC_WAITTIMEOUT;

    //  Объем кольцевого буфера
    *puLenBuf = pBuf->GetLenBuf ();

    //  Чтение окончено
    pSyncro->unlock ();

    return RC_OK;
    }


//  Запрос занятого объема
//  Возвращает RC_OK или код ошибки
int SyncBufQt::GetSpaceBuf (UINT* puSpace,   //  занятый объем, элементов
                            DWORD dwTimeout) //  время ожидания, мс
    {
    //  Нет буфера
    if (!byFlag)
       return RC_NOSYNCBUF;

    int nTimeout = dwTimeout;
    if (dwTimeout == INFINITE)
       nTimeout = -1;

    //  Ждем возможности чтения
    if (!(pSyncro->tryLockForRead (nTimeout)))
       return RC_WAITTIMEOUT;

    //  Занятое пространство в кольцевом буфере
    *puSpace = pBuf->GetSpaceBuf ();

    //  Чтение окончено
    pSyncro->unlock ();

    return RC_OK;
    }

//////////////////////////////////////////////////////////////////////

