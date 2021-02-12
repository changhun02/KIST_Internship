#ifndef __COMM_H__
#define __COMM_H__

#include <Windows.h>
//Define

#define COM_MAXBLOCK     4095

// Flow control flags

#define FC_DTRDSR       0x01
#define FC_RTSCTS       0x02
#define FC_XONXOFF      0x04

// ascii definitions

#define ASCII_BEL       0x07
#define ASCII_BS        0x08
#define ASCII_LF        0x0A
#define ASCII_CR        0x0D
#define ASCII_XON       0x11
#define ASCII_XOFF      0x13
#define ASCII_STX		0x02
#define ASCII_ETX		0xFE

#define WM_RECEIVEDATA WM_USER+100

/////////////////////////////////////////////////////////////////////////////
// CComm window
#define ZERO_MEMORY(s)	::ZeroMemory(&s, sizeof(s))

// flow control
#define FC_DTRDSR	0x01
#define FC_RTSCTS	0x02
#define FC_XONXOFF	0x04
#define	FC_NONE		0x00

#define ASCII_XON	0x11
#define ASCII_XOFF	0x13

// registry stuff
#define CS_REGKEY_SETTINGS     _T("통신환경")
#define CS_REGENTRY_PORT       _T("PORT")
#define CS_REGENTRY_PARITY     _T("PARITY")
#define CS_REGENTRY_BAUD       _T("BAUD")
#define CS_REGENTRY_DATABITS   _T("DATABITS")
#define CS_REGENTRY_STOPBITS   _T("STOPBITS")
#define CS_REGENTRY_FLOW       _T("FLOW")

#pragma  pack(push,1)

typedef struct _TTYSTRUCT
{
	BYTE		byCommPort;  // zero based port - 3 or higher implies TELNET
	BYTE		byXonXoff;
	BYTE		byByteSize;
	BYTE		byFlowCtrl;
	BYTE		byParity;
	BYTE		byStopBits;
	DWORD	dwBaudRate;

} TTYSTRUCT, *LPTTYSTRUCT;

#pragma  pack(pop,1)

// 통신프로토콜 Table
extern BYTE _nFlow[4];

// 통신 데이타 사이즈 테이블
extern int _nDataValues[2];

// 통신 속도 Table
extern int _nBaudRates[12];

// 통신 정지 비트 Table
extern BYTE _nStopBits[2];

class CComm : public CObject
{

	DECLARE_DYNCREATE(CComm)

public:

	//맴버변수 선언

	HANDLE      m_idComDev;	//컴포트 디바이스 핸들
	BOOL        m_fConnected;	//컴포트가 연결되면 1로 설정
	BYTE        abIn[COM_MAXBLOCK + 1];	//컴포트에서 들어오는 데이타
	HWND		m_hwnd;	//메세지를 전달할 윈도우 플러그
	BOOL		m_bTxEmpty;	// TX용 데이터가 모두 송신 되었을 경우 TRUE로 전환된다.

// Construction
public:

	//객체 생성자
	CComm();
	/////////////////////////////////////////////////////////////////////////////
	//OpenCommPort(LPTTYSTRUCT lpTTY)
	// - LPTTYSTRUCT lpTTY : Com Port를 설정하기 위한 TTYSTRUCT 구조체의 주소값

	//TTY구조체에 저장된 데이터로 Com Port 연결을 시도한다.
	/////////////////////////////////////////////////////////////////////////////
	BOOL OpenCommPort(LPTTYSTRUCT lpTTY);



	/////////////////////////////////////////////////////////////////////////////
	//DestroyComm()

	//컴포트의 연결을 해제하고 IO 동기화 Event를 해제한다.
	/////////////////////////////////////////////////////////////////////////////
	BOOL DestroyComm();



	/////////////////////////////////////////////////////////////////////////////
	//ReadCommBlock(LPSTR lpszBlock, int nMaxLength )
	// - LPSTR lpszBlock		: 데이터가 저장된 버퍼의 주소
	// - nt nMaxLength		: 최대 데이터의 크기

	//Com Port로 부터 수신된 데이터의 크기를 반환한다.
	/////////////////////////////////////////////////////////////////////////////
	int  ReadCommBlock(LPSTR data, int len);



	/////////////////////////////////////////////////////////////////////////////
	//WriteCommBlock( LPSTR lpByte , DWORD dwBytesToWrite)
	// - LPSTR lpByte			: Com Port로 보낼 데이터의 주소
	// - DWORD dwBytesToWrite	: Com Port로 보낼 데이터의 크기

	// 연결된 Com Port로 dwBytesToWrite의 크기만큼 데이터를 보낸다.
	/////////////////////////////////////////////////////////////////////////////
	BOOL WriteCommBlock(LPSTR data, DWORD len);



	/////////////////////////////////////////////////////////////////////////////
	//SetupConnection(LPTTYSTRUCT lpTTY)
	// - LPTTYSTRUCT lpTTY : Com Port를 설정하기 위한 TTYSTRUCT 구조체의 주소값

	//Com Port연결의 통신규약을 설정한다.
	//SetupConnection 이전에 CreateComPort를 해주어야 한다.
	/////////////////////////////////////////////////////////////////////////////
	BOOL SetupConnection(LPTTYSTRUCT lpTTY);



	/////////////////////////////////////////////////////////////////////////////
	//CloseConnection()

	//Com Port 연결을 닫는다.
	/////////////////////////////////////////////////////////////////////////////
	BOOL CloseConnection();



	/////////////////////////////////////////////////////////////////////////////
	//SetReadData(LPSTR data, int nLen)
	// - LPSTR data	: 데이터가 저장된 버퍼의 주소
	// - int nLen	: 읽을 데이터의 길이

	//nLen크기 만큼 Com Port로 부터 받은 데이터를 읽는다.
	/////////////////////////////////////////////////////////////////////////////
	void SetReadData(LPSTR data, int nLen);



	/////////////////////////////////////////////////////////////////////////////
	//SetHwnd(HWND hwnd)
	// - HWND hwnd : 메세지를 전달받을 객체의 hWnd 

	//데이터를 받는 경우 메세지를 전달할 객체의 hwnd를 전달받아 등록한다.
	/////////////////////////////////////////////////////////////////////////////
	void SetHwnd(HWND hwnd);


	void EscapeCommFunction(DWORD dwFunc);


	// Attributes
public:

	//IO 동기화를 위한 구조체
	OVERLAPPED  m_OverLappedWrite, m_OverLappedWriteRead;

	// Operations
public:


	// Implementation
public:

	//객체 소멸자
	virtual ~CComm();


protected:
};


/////////////////////////////////////////////////////////////////////////////
#endif