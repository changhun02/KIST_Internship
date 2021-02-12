#include "pch.h"
#include "framework.h"
#include "Comm.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CComm, CObject)

// 통신프로토콜 Table
BYTE _nFlow[] = { FC_NONE,FC_DTRDSR,FC_RTSCTS,FC_XONXOFF };

//통신 데이터 크기
int _nDataValues[] = { 7,8 };


BYTE _nStopBits[] = { ONESTOPBIT, TWOSTOPBITS };
//BOOL	m_bTxEmpty=TRUE;

UINT ReceMessage;



//Comm()
//객체 생성자

CComm::CComm()
{
	//맴버변수를 초기화한다.
	m_idComDev = NULL;
	m_fConnected = FALSE;
	m_bTxEmpty = TRUE;
	m_OverLappedWriteRead.hEvent = NULL;
	m_OverLappedWrite.hEvent = NULL;
}



//Comm
//객체 소멸자

CComm::~CComm()
{
	DestroyComm();
}


/////////////////////////////////////////////////////////////////////////////
// CComm message handlers

//CommWatchProc(LPVOID lpData)
// - LPVOID lpData : 스레드가 생성될 때 넘겨받는 인자

//통신을 하는 프로세저 즉 데이타가 들어왔을대 감시하는
//루틴 본루틴은 OpenComPort 함수 실행시 프로시저로 연결됨
//OpenComPort 함수 참조
/////////////////////////////////////////////////////////////////////////////

DWORD CommWatchProc(LPVOID lpData)
{
	DWORD       dwEvtMask;
	OVERLAPPED  os;
	CComm*      npComm = (CComm*)lpData;
	char		InData[COM_MAXBLOCK + 1];
	int			nLength;

	//idCommDev에 연결된 Com Port가 없는 경우
	/*
	if (npComm == NULL || !npComm->IsKindOf(RUNTIME_CLASS(CComm)))
		return (DWORD)(-1);
	*/

	memset(&os, 0, sizeof(OVERLAPPED));


	//이벤트 핸들을 만든다.
	os.hEvent = CreateEvent(NULL,    // no security
		TRUE,    // explicit reset req
		FALSE,   // initial event reset
		NULL); // no name



	if (os.hEvent == NULL) {
		MessageBox(NULL, _T("Failed to create event for thread!"), _T("comm Error!"),
			MB_ICONEXCLAMATION | MB_OK);
		return (FALSE);
	}



	DWORD dwEventFlags = EV_BREAK | EV_CTS | EV_DSR | EV_ERR | EV_RING |
		EV_RLSD | EV_RXCHAR | EV_RXFLAG | EV_TXEMPTY;




	if (!SetCommMask(npComm->m_idComDev, dwEventFlags))
		return (FALSE);



	//Com Port에 연결이 있는 동안 반복
	while (npComm->m_fConnected) {
		dwEvtMask = 0;

		WaitCommEvent(npComm->m_idComDev, &dwEvtMask, NULL);


		if ((dwEvtMask & EV_RXCHAR) == EV_RXCHAR) {

			//데이터를 받을 때 까지 반복
			do
			{
				memset(InData, 0, 80);

				//수신된 데이터의 크기 얻는다.
				if (nLength = npComm->ReadCommBlock((LPSTR)InData, COM_MAXBLOCK)) {

					//수신된 데이터의 크기가 0이 아닌경우
					//수신된 데이터 길이만큼 데이터를 얻는다.
					npComm->SetReadData(InData, nLength);
				}
			} while (nLength > 0);

		}

	}

	//디바이스 핸들을 정리한다.
	CloseHandle(os.hEvent);

	//스레드를 종료한다.
	return(TRUE);
}


/////////////////////////////////////////////////////////////////////////////
//SetReadData(LPSTR data, int nLen)
// - LPSTR data	: 데이터가 저장된 버퍼의 주소
// - int nLen	: 읽을 데이터의 길이

//nLen크기 만큼 Com Port로 부터 받은 데이터를 읽는다.
/////////////////////////////////////////////////////////////////////////////

void CComm::SetReadData(LPSTR data, int nLen)
{
	//받은 데이터를 복사한다.
	memcpy(abIn, data, nLen);

	//hWnd가 등록된 객체에게 메세지를 호출한다.
	SendMessage(m_hwnd, WM_RECEIVEDATA, nLen, 0);
}



/////////////////////////////////////////////////////////////////////////////
//SetHwnd(HWND hwnd)
// - HWND hwnd : 메세지를 전달받을 객체의 hWnd 

//데이터를 받는 경우 메세지를 전달할 객체의 hwnd를 전달받아 등록한다.
/////////////////////////////////////////////////////////////////////////////

void CComm::SetHwnd(HWND hwnd)
{
	//hWnd를 등록한다.
	m_hwnd = hwnd;
}


/////////////////////////////////////////////////////////////////////////////
//OpenCommPort(LPTTYSTRUCT lpTTY)
// - LPTTYSTRUCT lpTTY : Com Port를 설정하기 위한 TTYSTRUCT 구조체의 주소값

//TTY구조체에 저장된 데이터로 Com Port 연결을 시도한다.
/////////////////////////////////////////////////////////////////////////////

BOOL CComm::OpenCommPort(LPTTYSTRUCT lpTTY)
{
	char       szPort[15];
	BOOL       bRetVal;
	COMMTIMEOUTS  CommTimeOuts;

	//overlapped 구조체 초기화
	m_OverLappedWrite.Offset = 0;
	m_OverLappedWrite.OffsetHigh = 0;
	m_OverLappedWriteRead.Offset = 0;
	m_OverLappedWriteRead.OffsetHigh = 0;

	//IO동기화를 위한 구조체에 Event핸들을 생성한다.
	//Read Event생성, 핸들 획득
	m_OverLappedWriteRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	//Read Event생성에 실패 한 경우 FALSE를 Return하고 종료
	if (m_OverLappedWriteRead.hEvent == NULL) return FALSE;


	//Write Event생성, 핸들 획득
	m_OverLappedWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	//Write Event생성에 실패 한 경우 Read Event를 정리하고 FALSE를 Return한 뒤 종료
	if (NULL == m_OverLappedWrite.hEvent) {
		CloseHandle(m_OverLappedWriteRead.hEvent);
		return FALSE;
	}

	//CreateFile함수에 전달할 Com Port이름을 생성.
	wsprintf(szPort, _T("\\\\.\\COM%d"), lpTTY->byCommPort);


	//File을 열어 디바이스 핸들을 얻어온다.
	//Window는 주변장치와 File형식으로 데이터를 주고 받는다.
	//따라서 주변장치를 컨트롤하거나 데이터를 주고받기 위해서 File을 생성하고 핸들을 얻어온다.

	//File생성을 시도
	if ((m_idComDev =
		CreateFile(szPort, GENERIC_READ | GENERIC_WRITE,
			0,                    // exclusive access
			NULL,                 // no security attrs
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL |
			FILE_FLAG_OVERLAPPED, // overlapped I/O(동기)
			NULL)) == (HANDLE)-1)
		//실패한 경우 FALSE를 Return하고 종료
		return (FALSE);

	//성공한 경우
	else {

		//Com Port의 통신 데이터 크기를 Char크기로 지정한다.
		SetCommMask(m_idComDev, EV_RXCHAR);
		SetupComm(m_idComDev, 4096, 4096);

		//Open하는 Com Port의 버퍼를 초기화한다.
		PurgeComm(m_idComDev, PURGE_TXABORT | PURGE_RXABORT |
			PURGE_TXCLEAR | PURGE_RXCLEAR);


		//Com 통신 한계시간 설정
		CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFFF;
		CommTimeOuts.ReadTotalTimeoutMultiplier = 0;
		CommTimeOuts.ReadTotalTimeoutConstant = 1000;
		CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
		CommTimeOuts.WriteTotalTimeoutConstant = 1000;
		SetCommTimeouts(m_idComDev, &CommTimeOuts);
	}

	//Com Port 연결을 시도한다.
	//성공한 경우 1
	//실패한 경우 0을 Return한다.
	bRetVal = SetupConnection(lpTTY);

	//연결에 성공한 경우
	if (bRetVal) {
		//플래그 변수에 TRUE를 지정
		m_fConnected = TRUE;

		//데이터 수신을 확인하고 처리하는 스레드를 실행한다.
		//스레드가 별도로 데이터가 들어오는지를 확인하고 메세지를 호출하게 된다.
		AfxBeginThread((AFX_THREADPROC)CommWatchProc, (LPVOID)this);


		//연결에 실패한 경우
	}
	else {
		//플래그 변수에 FALSE를 지정
		m_fConnected = FALSE;

		//Open한 File을 정리한다.
		CloseHandle(m_idComDev);
	}

	//Open 성공여부를 Return한다.
	//성공한 경우 1
	//실패한 경우 0
	return (bRetVal);
}


/////////////////////////////////////////////////////////////////////////////
//SetupConnection(LPTTYSTRUCT lpTTY)
// - LPTTYSTRUCT lpTTY : Com Port를 설정하기 위한 TTYSTRUCT 구조체의 주소값

//Com Port연결의 통신규약을 설정한다.
//SetupConnection 이전에 CreateComPort를 해주어야 한다.
/////////////////////////////////////////////////////////////////////////////

BOOL CComm::SetupConnection(LPTTYSTRUCT lpTTY)
{
	BOOL       bRetVal;
	DCB        dcb;

	dcb.DCBlength = sizeof(DCB);

	//현재 Com Port의 상태를 읽어온다.
	GetCommState(m_idComDev, &dcb);

	//Com Port의 통신속도, 데이터 크기, 패러티, 스톱비트 값을 지정
	dcb.BaudRate = lpTTY->dwBaudRate;
	dcb.ByteSize = lpTTY->byByteSize;
	dcb.Parity = lpTTY->byParity;
	dcb.StopBits = lpTTY->byStopBits;

	//하드웨어 흐름제어를 설정한다.
	BOOL bSet = (BYTE)((lpTTY->byFlowCtrl & FC_DTRDSR) != 0);

	dcb.fOutxDsrFlow = bSet;

	if (bSet)
		dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;
	else
		dcb.fDtrControl = DTR_CONTROL_ENABLE;

	// FC_RTSCTS
	bSet = (BYTE)((lpTTY->byFlowCtrl & FC_RTSCTS) != 0);
	dcb.fOutxCtsFlow = bSet;

	if (bSet)
		dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
	else
		dcb.fRtsControl = RTS_CONTROL_ENABLE;

	// setup software flow control FC_XONXOFF
	bSet = (BYTE)((lpTTY->byFlowCtrl & FC_XONXOFF) != 0);

	dcb.fInX = dcb.fOutX = bSet;
	dcb.XonChar = ASCII_XON;
	dcb.XoffChar = ASCII_XOFF;
	dcb.XonLim = 1;
	dcb.XoffLim = 1;

	// other various settings
	dcb.fBinary = TRUE;
	dcb.fParity = TRUE;

	//설정된 dcb값을 Com Port에 등록한다.
	bRetVal = SetCommState(m_idComDev, &dcb); //변경된 Dcb 설정

	return (bRetVal);
}


/////////////////////////////////////////////////////////////////////////////
//ReadCommBlock(LPSTR lpszBlock, int nMaxLength )
// - LPSTR lpszBlock		: 데이터가 저장된 버퍼의 주소
// - nt nMaxLength		: 최대 데이터의 크기

//Com Port로 부터 수신된 데이터의 크기를 반환한다.
/////////////////////////////////////////////////////////////////////////////

int CComm::ReadCommBlock(LPSTR lpszBlock, int nMaxLength)
{
	BOOL       fReadStat;
	COMSTAT    ComStat;
	DWORD      dwErrorFlags;
	DWORD      dwLength;

	// only try to read number of bytes in queue 
	ClearCommError(m_idComDev, &dwErrorFlags, &ComStat);
	dwLength = min((DWORD)nMaxLength, ComStat.cbInQue);


	if (dwLength > 0) {

		//크기만큼 데이터를 Com Port로부터 읽는다.
		fReadStat = ReadFile(m_idComDev, lpszBlock, dwLength, &dwLength, &m_OverLappedWriteRead);


		if (!fReadStat) {

			if (GetLastError() == ERROR_IO_PENDING) {

				OutputDebugString(_T("\n\rIO Pending"));
				// we have to wait for read to complete.  This function will timeout
				// according to the CommTimeOuts.ReadTotalTimeoutConstant variable
				// Every time it times out, check for port errors			

				while (!GetOverlappedResult(m_idComDev, &m_OverLappedWriteRead,
					&dwLength, TRUE))
				{

					DWORD dwError = GetLastError();

					if (dwError == ERROR_IO_INCOMPLETE) {
						// normal result if not finished
						continue;

					}
					else {
						// CAN DISPLAY ERROR MESSAGE HERE
						// an error occured, try to recover
						::ClearCommError(m_idComDev, &dwErrorFlags, &ComStat);

						//if(dwErrorFlags > 0);

							// CAN DISPLAY ERROR MESSAGE HERE

						break;
					}
				}


			}
			else {

				// some other error occured
				dwLength = 0;
				ClearCommError(m_idComDev, &dwErrorFlags, &ComStat);

				//if(dwErrorFlags > 0);
					// CAN DISPLAY ERROR MESSAGE HERE
			}
		}
	}

	return (dwLength);
}


/////////////////////////////////////////////////////////////////////////////
//WriteCommBlock( LPSTR lpByte , DWORD dwBytesToWrite)
// - LPSTR lpByte			: Com Port로 보낼 데이터의 주소
// - DWORD dwBytesToWrite	: Com Port로 보낼 데이터의 크기

// 연결된 Com Port로 dwBytesToWrite의 크기만큼 데이터를 보낸다.
/////////////////////////////////////////////////////////////////////////////

BOOL CComm::WriteCommBlock(LPSTR lpByte, DWORD dwBytesToWrite)
{
	DWORD		dwErrorFlags;
	BOOL        fWriteStat;
	DWORD       dwBytesWritten;
	COMSTAT ComStat;

	DWORD	dwLength = dwBytesToWrite;

	m_bTxEmpty = FALSE;

	//데이터 크기만큼 데이터를 Com Port로 보낸다.
	fWriteStat = WriteFile(m_idComDev, lpByte, dwBytesToWrite, &dwBytesWritten, &m_OverLappedWrite);


	//if (dwBytesToWrite != dwBytesWritten)

	if (!fWriteStat) {

		if (GetLastError() == ERROR_IO_PENDING) {

			OutputDebugString(_T("\n\rIO Pending"));
			// 읽을 문자가 남아 있거나 전송할 문자가 남아 있을 경우 Overapped IO의
			// 특성에 따라 ERROR_IO_PENDING 에러 메시지가 전달된다.
			//timeouts에 정해준 시간만큼 기다려준다.
			while (!GetOverlappedResult(m_idComDev, &m_OverLappedWrite,
				&dwLength, TRUE))
			{
				DWORD dwError = GetLastError();

				if (dwError == ERROR_IO_INCOMPLETE) {
					// normal result if not finished
					continue;


				}
				else {
					// CAN DISPLAY ERROR MESSAGE HERE
					// an error occured, try to recover
					::ClearCommError(m_idComDev, &dwErrorFlags, &ComStat);

					if (dwErrorFlags > 0) {

						// CAN DISPLAY ERROR MESSAGE HERE
						AfxMessageBox(_T("ERROR"));
					}

					break;
				}
			}


		}
		else {

			// some other error occured
			dwLength = 0;

			ClearCommError(m_idComDev, &dwErrorFlags, &ComStat);


			if (dwErrorFlags > 0) {

				// CAN DISPLAY ERROR MESSAGE HERE				
				AfxMessageBox(_T("ERROR"));
			}
		}
	}

	return (TRUE);
}


/////////////////////////////////////////////////////////////////////////////
//DestroyComm()

//컴포트의 연결을 해제하고 IO 동기화 Event를 해제한다.
/////////////////////////////////////////////////////////////////////////////

BOOL CComm::DestroyComm()
{
	if (m_fConnected)
		CloseConnection();

	if (m_OverLappedWriteRead.hEvent != NULL)
		CloseHandle(m_OverLappedWriteRead.hEvent);

	if (m_OverLappedWrite.hEvent != NULL)
		CloseHandle(m_OverLappedWrite.hEvent);

	return (TRUE);
}


/////////////////////////////////////////////////////////////////////////////
//CloseConnection()

//Com Port 연결을 닫는다.
/////////////////////////////////////////////////////////////////////////////

BOOL CComm::CloseConnection()
{

	// set connected flag to FALSE

	m_fConnected = FALSE;

	// disable event notification and wait for thread
	// to halt

	SetCommMask(m_idComDev, 0);


	EscapeCommFunction(CLRDTR);
	EscapeCommFunction(CLRRTS);


	PurgeComm(m_idComDev, PURGE_TXABORT | PURGE_RXABORT |
		PURGE_TXCLEAR | PURGE_RXCLEAR);
	CloseHandle(m_idComDev);

	return (TRUE);

}



void CComm::EscapeCommFunction(DWORD dwFunc)
{
	// TODO: Add your control notification handler code here
	::EscapeCommFunction(m_idComDev, dwFunc);
}