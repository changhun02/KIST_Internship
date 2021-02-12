#include "pch.h"
#include "framework.h"
#include "Comm.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CComm, CObject)

// ����������� Table
BYTE _nFlow[] = { FC_NONE,FC_DTRDSR,FC_RTSCTS,FC_XONXOFF };

//��� ������ ũ��
int _nDataValues[] = { 7,8 };


BYTE _nStopBits[] = { ONESTOPBIT, TWOSTOPBITS };
//BOOL	m_bTxEmpty=TRUE;

UINT ReceMessage;



//Comm()
//��ü ������

CComm::CComm()
{
	//�ɹ������� �ʱ�ȭ�Ѵ�.
	m_idComDev = NULL;
	m_fConnected = FALSE;
	m_bTxEmpty = TRUE;
	m_OverLappedWriteRead.hEvent = NULL;
	m_OverLappedWrite.hEvent = NULL;
}



//Comm
//��ü �Ҹ���

CComm::~CComm()
{
	DestroyComm();
}


/////////////////////////////////////////////////////////////////////////////
// CComm message handlers

//CommWatchProc(LPVOID lpData)
// - LPVOID lpData : �����尡 ������ �� �Ѱܹ޴� ����

//����� �ϴ� ���μ��� �� ����Ÿ�� �������� �����ϴ�
//��ƾ ����ƾ�� OpenComPort �Լ� ����� ���ν����� �����
//OpenComPort �Լ� ����
/////////////////////////////////////////////////////////////////////////////

DWORD CommWatchProc(LPVOID lpData)
{
	DWORD       dwEvtMask;
	OVERLAPPED  os;
	CComm*      npComm = (CComm*)lpData;
	char		InData[COM_MAXBLOCK + 1];
	int			nLength;

	//idCommDev�� ����� Com Port�� ���� ���
	/*
	if (npComm == NULL || !npComm->IsKindOf(RUNTIME_CLASS(CComm)))
		return (DWORD)(-1);
	*/

	memset(&os, 0, sizeof(OVERLAPPED));


	//�̺�Ʈ �ڵ��� �����.
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



	//Com Port�� ������ �ִ� ���� �ݺ�
	while (npComm->m_fConnected) {
		dwEvtMask = 0;

		WaitCommEvent(npComm->m_idComDev, &dwEvtMask, NULL);


		if ((dwEvtMask & EV_RXCHAR) == EV_RXCHAR) {

			//�����͸� ���� �� ���� �ݺ�
			do
			{
				memset(InData, 0, 80);

				//���ŵ� �������� ũ�� ��´�.
				if (nLength = npComm->ReadCommBlock((LPSTR)InData, COM_MAXBLOCK)) {

					//���ŵ� �������� ũ�Ⱑ 0�� �ƴѰ��
					//���ŵ� ������ ���̸�ŭ �����͸� ��´�.
					npComm->SetReadData(InData, nLength);
				}
			} while (nLength > 0);

		}

	}

	//����̽� �ڵ��� �����Ѵ�.
	CloseHandle(os.hEvent);

	//�����带 �����Ѵ�.
	return(TRUE);
}


/////////////////////////////////////////////////////////////////////////////
//SetReadData(LPSTR data, int nLen)
// - LPSTR data	: �����Ͱ� ����� ������ �ּ�
// - int nLen	: ���� �������� ����

//nLenũ�� ��ŭ Com Port�� ���� ���� �����͸� �д´�.
/////////////////////////////////////////////////////////////////////////////

void CComm::SetReadData(LPSTR data, int nLen)
{
	//���� �����͸� �����Ѵ�.
	memcpy(abIn, data, nLen);

	//hWnd�� ��ϵ� ��ü���� �޼����� ȣ���Ѵ�.
	SendMessage(m_hwnd, WM_RECEIVEDATA, nLen, 0);
}



/////////////////////////////////////////////////////////////////////////////
//SetHwnd(HWND hwnd)
// - HWND hwnd : �޼����� ���޹��� ��ü�� hWnd 

//�����͸� �޴� ��� �޼����� ������ ��ü�� hwnd�� ���޹޾� ����Ѵ�.
/////////////////////////////////////////////////////////////////////////////

void CComm::SetHwnd(HWND hwnd)
{
	//hWnd�� ����Ѵ�.
	m_hwnd = hwnd;
}


/////////////////////////////////////////////////////////////////////////////
//OpenCommPort(LPTTYSTRUCT lpTTY)
// - LPTTYSTRUCT lpTTY : Com Port�� �����ϱ� ���� TTYSTRUCT ����ü�� �ּҰ�

//TTY����ü�� ����� �����ͷ� Com Port ������ �õ��Ѵ�.
/////////////////////////////////////////////////////////////////////////////

BOOL CComm::OpenCommPort(LPTTYSTRUCT lpTTY)
{
	char       szPort[15];
	BOOL       bRetVal;
	COMMTIMEOUTS  CommTimeOuts;

	//overlapped ����ü �ʱ�ȭ
	m_OverLappedWrite.Offset = 0;
	m_OverLappedWrite.OffsetHigh = 0;
	m_OverLappedWriteRead.Offset = 0;
	m_OverLappedWriteRead.OffsetHigh = 0;

	//IO����ȭ�� ���� ����ü�� Event�ڵ��� �����Ѵ�.
	//Read Event����, �ڵ� ȹ��
	m_OverLappedWriteRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	//Read Event������ ���� �� ��� FALSE�� Return�ϰ� ����
	if (m_OverLappedWriteRead.hEvent == NULL) return FALSE;


	//Write Event����, �ڵ� ȹ��
	m_OverLappedWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	//Write Event������ ���� �� ��� Read Event�� �����ϰ� FALSE�� Return�� �� ����
	if (NULL == m_OverLappedWrite.hEvent) {
		CloseHandle(m_OverLappedWriteRead.hEvent);
		return FALSE;
	}

	//CreateFile�Լ��� ������ Com Port�̸��� ����.
	wsprintf(szPort, _T("\\\\.\\COM%d"), lpTTY->byCommPort);


	//File�� ���� ����̽� �ڵ��� ���´�.
	//Window�� �ֺ���ġ�� File�������� �����͸� �ְ� �޴´�.
	//���� �ֺ���ġ�� ��Ʈ���ϰų� �����͸� �ְ�ޱ� ���ؼ� File�� �����ϰ� �ڵ��� ���´�.

	//File������ �õ�
	if ((m_idComDev =
		CreateFile(szPort, GENERIC_READ | GENERIC_WRITE,
			0,                    // exclusive access
			NULL,                 // no security attrs
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL |
			FILE_FLAG_OVERLAPPED, // overlapped I/O(����)
			NULL)) == (HANDLE)-1)
		//������ ��� FALSE�� Return�ϰ� ����
		return (FALSE);

	//������ ���
	else {

		//Com Port�� ��� ������ ũ�⸦ Charũ��� �����Ѵ�.
		SetCommMask(m_idComDev, EV_RXCHAR);
		SetupComm(m_idComDev, 4096, 4096);

		//Open�ϴ� Com Port�� ���۸� �ʱ�ȭ�Ѵ�.
		PurgeComm(m_idComDev, PURGE_TXABORT | PURGE_RXABORT |
			PURGE_TXCLEAR | PURGE_RXCLEAR);


		//Com ��� �Ѱ�ð� ����
		CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFFF;
		CommTimeOuts.ReadTotalTimeoutMultiplier = 0;
		CommTimeOuts.ReadTotalTimeoutConstant = 1000;
		CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
		CommTimeOuts.WriteTotalTimeoutConstant = 1000;
		SetCommTimeouts(m_idComDev, &CommTimeOuts);
	}

	//Com Port ������ �õ��Ѵ�.
	//������ ��� 1
	//������ ��� 0�� Return�Ѵ�.
	bRetVal = SetupConnection(lpTTY);

	//���ῡ ������ ���
	if (bRetVal) {
		//�÷��� ������ TRUE�� ����
		m_fConnected = TRUE;

		//������ ������ Ȯ���ϰ� ó���ϴ� �����带 �����Ѵ�.
		//�����尡 ������ �����Ͱ� ���������� Ȯ���ϰ� �޼����� ȣ���ϰ� �ȴ�.
		AfxBeginThread((AFX_THREADPROC)CommWatchProc, (LPVOID)this);


		//���ῡ ������ ���
	}
	else {
		//�÷��� ������ FALSE�� ����
		m_fConnected = FALSE;

		//Open�� File�� �����Ѵ�.
		CloseHandle(m_idComDev);
	}

	//Open �������θ� Return�Ѵ�.
	//������ ��� 1
	//������ ��� 0
	return (bRetVal);
}


/////////////////////////////////////////////////////////////////////////////
//SetupConnection(LPTTYSTRUCT lpTTY)
// - LPTTYSTRUCT lpTTY : Com Port�� �����ϱ� ���� TTYSTRUCT ����ü�� �ּҰ�

//Com Port������ ��űԾ��� �����Ѵ�.
//SetupConnection ������ CreateComPort�� ���־�� �Ѵ�.
/////////////////////////////////////////////////////////////////////////////

BOOL CComm::SetupConnection(LPTTYSTRUCT lpTTY)
{
	BOOL       bRetVal;
	DCB        dcb;

	dcb.DCBlength = sizeof(DCB);

	//���� Com Port�� ���¸� �о�´�.
	GetCommState(m_idComDev, &dcb);

	//Com Port�� ��żӵ�, ������ ũ��, �з�Ƽ, �����Ʈ ���� ����
	dcb.BaudRate = lpTTY->dwBaudRate;
	dcb.ByteSize = lpTTY->byByteSize;
	dcb.Parity = lpTTY->byParity;
	dcb.StopBits = lpTTY->byStopBits;

	//�ϵ���� �帧��� �����Ѵ�.
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

	//������ dcb���� Com Port�� ����Ѵ�.
	bRetVal = SetCommState(m_idComDev, &dcb); //����� Dcb ����

	return (bRetVal);
}


/////////////////////////////////////////////////////////////////////////////
//ReadCommBlock(LPSTR lpszBlock, int nMaxLength )
// - LPSTR lpszBlock		: �����Ͱ� ����� ������ �ּ�
// - nt nMaxLength		: �ִ� �������� ũ��

//Com Port�� ���� ���ŵ� �������� ũ�⸦ ��ȯ�Ѵ�.
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

		//ũ�⸸ŭ �����͸� Com Port�κ��� �д´�.
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
// - LPSTR lpByte			: Com Port�� ���� �������� �ּ�
// - DWORD dwBytesToWrite	: Com Port�� ���� �������� ũ��

// ����� Com Port�� dwBytesToWrite�� ũ�⸸ŭ �����͸� ������.
/////////////////////////////////////////////////////////////////////////////

BOOL CComm::WriteCommBlock(LPSTR lpByte, DWORD dwBytesToWrite)
{
	DWORD		dwErrorFlags;
	BOOL        fWriteStat;
	DWORD       dwBytesWritten;
	COMSTAT ComStat;

	DWORD	dwLength = dwBytesToWrite;

	m_bTxEmpty = FALSE;

	//������ ũ�⸸ŭ �����͸� Com Port�� ������.
	fWriteStat = WriteFile(m_idComDev, lpByte, dwBytesToWrite, &dwBytesWritten, &m_OverLappedWrite);


	//if (dwBytesToWrite != dwBytesWritten)

	if (!fWriteStat) {

		if (GetLastError() == ERROR_IO_PENDING) {

			OutputDebugString(_T("\n\rIO Pending"));
			// ���� ���ڰ� ���� �ְų� ������ ���ڰ� ���� ���� ��� Overapped IO��
			// Ư���� ���� ERROR_IO_PENDING ���� �޽����� ���޵ȴ�.
			//timeouts�� ������ �ð���ŭ ��ٷ��ش�.
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

//����Ʈ�� ������ �����ϰ� IO ����ȭ Event�� �����Ѵ�.
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

//Com Port ������ �ݴ´�.
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