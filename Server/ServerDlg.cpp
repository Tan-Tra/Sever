// ServerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Server.h"
#include "ServerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CServerDlg dialog


CServerDlg::CServerDlg(CWnd* pParent /*=NULL*/)
: CDialog(CServerDlg::IDD, pParent)
, m_msgString(_T(""))
, m_Manage(_T(""))
, m_Online(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_BOXCHAT, m_msgString);
	DDX_Text(pDX, IDC_MANAGE, m_Manage);
	DDX_Text(pDX, IDC_ONLINE, m_Online);
}

BEGIN_MESSAGE_MAP(CServerDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_SOCKET,SockMsg)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_LISTEN, &CServerDlg::OnBnClickedListen)
	ON_BN_CLICKED(IDC_CANCEL, &CServerDlg::OnBnClickedCancel)
	//ON_EN_CHANGE(IDC_BOXCHAT, &CServerDlg::OnEnChangeBoxchat)
END_MESSAGE_MAP()


// CServerDlg message handlers

BOOL CServerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

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

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CServerDlg::Split(CString src, CString des[3])
{
	int p1,p2,p3;

	p1=src.Find(_T("\r\n"),0);
	des[0]=src.Mid(0,p1);
	
	p2=src.Find(_T("\r\n"),p1+1);
	des[1] = src.Mid(p1 + 2, p2 - (p1 + 2));

	p3 = src.Find(_T("\r\n"), p2 + 1);
	des[2] = src.Mid(p2 + 2, p3 - (p2 + 2));
}

char* CServerDlg::ConvertToChar(const CString &s)
{
	int nSize = s.GetLength();
	char *pAnsiString = new char[nSize+1];
	memset(pAnsiString,0,nSize+1);
	wcstombs(pAnsiString, s, nSize+1);
	return pAnsiString;
}

void CServerDlg::mSend(SOCKET sk, CString Command)
{
	int Len=Command.GetLength();
	Len+=Len;
	PBYTE sendBuff = new BYTE[1000];
	memset(sendBuff,0,1000);
	memcpy(sendBuff,(PBYTE)(LPCTSTR)Command, Len);
	send(sk,(char*)&Len,sizeof(Len),0);
	send(sk,(char*)sendBuff,Len,0);
	delete sendBuff;
}

int CServerDlg::mRecv(SOCKET sk, CString &Command)
{
	PBYTE buffer = new BYTE[1000];
	memset(buffer,0, 1000);
	recv(sk ,(char*)&buffLength,sizeof(int),0);
	recv(sk,(char*)buffer,buffLength,0);
	TCHAR* ttc = (TCHAR*)buffer;
	Command = ttc;

	if(Command.GetLength()==0)
		return -1;
	return 0;

}

void CServerDlg::OnBnClickedListen()
{
	// TODO: Add your control notification handler code here
	loadData();
	UpdateData();

	sockServer =socket(AF_INET,SOCK_STREAM,0);
	serverAdd.sin_family=AF_INET;
	serverAdd.sin_port=htons(PORT);
	serverAdd.sin_addr.s_addr =htonl(INADDR_ANY);

	int err2=bind(sockServer,(SOCKADDR*)&serverAdd,sizeof (serverAdd));
	int err1=listen(sockServer,5);
	int err =WSAAsyncSelect(sockServer,m_hWnd,WM_SOCKET,FD_READ|FD_ACCEPT|FD_CLOSE);
	if (err)
		MessageBox((LPCTSTR)"Cant call WSAAsyncSelect");	
	GetDlgItem(IDC_LISTEN)->EnableWindow(FALSE);
	number_Socket=0;
	pSock = new SockName[200];
}

void CServerDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}

LRESULT CServerDlg::SockMsg(WPARAM wParam, LPARAM lParam)
{
	if (WSAGETSELECTERROR(lParam))
	{
		// Display the error and close the socket
		closesocket(wParam);
	}
	switch (WSAGETSELECTEVENT(lParam))
	{
	case FD_ACCEPT:
	{
		pSock[number_Socket].sockClient = accept(wParam, NULL, NULL);
		//GetDlgItem(IDC_LISTEN)->EnableWindow(FALSE);

	}break;
	case FD_READ:
	{

		int post = -1, dpos = -1;

		for (int i = 0; i < number_Socket; i++)
		{
			if (pSock[i].sockClient == wParam)
			{
				if (i < number_Socket)
					post = i;
			}
		}
		CString temp;
		if (mRecv(wParam, temp) < 0)
			break;
		Split(temp, strResult);
		int flag = _ttoi(strResult[0]);

		char* mes1 = ConvertToChar(strResult[1]);
		char* mes2 = ConvertToChar(strResult[2]);
		switch (flag)
		{
		case 1://Login
		{
			int t = 0;
			if (number_Socket > 0)
			{
				for (int i = 0; i < number_Socket; i++)
				{
					if (strcmp(mes1, pSock[i].Name) == 0)//Trung ten user
					{
						t = 1;
						break;
					}
				}
			}
			if (checkLogin(mes1, mes2) != 1)
			{
				t = 1;
			}

			if (t == 0)
			{
				strcpy(pSock[number_Socket].Name, mes1);
				Command = _T("1\r\n1\r\n");
				Command += pSock[number_Socket].Name;
				Command += _T("\r\n");
				m_msgString += strResult[1] + _T(" login\r\n");

				m_Online += pSock[number_Socket].Name;//cập nhật tên client vừa đăng nhập vào danh sách các client đang onl
				m_Online += _T("\r\n");
				number_Socket++;//tăng số lượng client
				UpdateData(FALSE);
				for (int i = 0; i < number_Socket; i++)
				{
					mSend(pSock[i].sockClient, Command);
				}
			}
			else
			{
				Command = _T("1\r\n0\r\n");
				Command += _T("\r\n");
				mSend(wParam, Command);
			}

			Command = _T("5\r\n");
			Command += m_Online;
			for (int i = 0; i < number_Socket; i++)
			{
				mSend(pSock[i].sockClient, Command);
			}
			UpdateData(FALSE);
			break;
		}
		case 2://Sign up
		{
			int post = -1;
			for (int i = 0; i < number_Socket; i++)
			{
				if (pSock[i].sockClient == wParam)
				{
					if (i < number_Socket)
						post = i;
				}
			}
			//kiểm tra xem tên đăng ký có tồn tại trong hệ thống hay chưa hay chưa
			if (checkSignUp(mes1) == 1)//tài khoản vừa yêu cầu khởi tạo đã tồn tại trong hệ thống
			{
				Command = _T("2\r\n0\r\n\r\n");
				mSend(wParam, Command);
				closesocket(wParam);

			}
			else
			{
				//thêm tài khoản mới vào database
				updateDatabase(mes1, mes2);
				Command = _T("2\r\n1\r\n\r\n");
				mSend(wParam, Command);
				m_msgString += strResult[1] + _T(" signup\r\n");

				//đăng nhập vào hệ thống
				//cập nhật các thông tin cần thiết như đăng nhập bình thường
				strcpy(pSock[number_Socket].Name, mes1);
				Command = _T("1\r\n1\r\n");
				Command += pSock[number_Socket].Name;
				Command += _T("\r\n");
				m_msgString += strResult[1] + _T(" login\r\n");

				m_Online += pSock[number_Socket].Name;//cập nhật tên client vừa đăng nhập vào danh sách các client đang onl
				m_Online += _T("\r\n");
				number_Socket++;//tăng số lượng client
				UpdateData(FALSE);
				for (int i = 0; i < number_Socket; i++)
				{
					mSend(pSock[i].sockClient, Command);
				}
			}

			UpdateData(FALSE);
		}break;



		case 3:
		{
			m_msgString += strResult[1] + _T("-  ");
			m_msgString += strResult[2] + _T("\r\n");
			for (int i = 0; i < number_Socket; i++)
			{
				mSend(pSock[i].sockClient, temp);
			}
			UpdateData(FALSE);
		}break;
		case 4:
		{
			{
				int post = -1;
				for (int i = 0; i < number_Socket; i++)
				{
					if (pSock[i].sockClient == wParam)
					{
						if (i < number_Socket)
							post = i;
					}
				}

				m_msgString += pSock[post].Name;
				m_msgString += _T(" logout\r\n");

				//gửi thông báo cho các Client khác rằng Client A nào đó đã log out
				Command = _T("3\r\n");
				Command += pSock[post].Name;
				Command += _T("\r\n");
				Command += _T(" logout\r\n");
				for (int i = 0; i < number_Socket; i++)
				{
					if (i != post)
						mSend(pSock[i].sockClient, Command);
				}

				//Xóa tên client ra khỏi danh sách client online
				CString temp(pSock[post].Name);
				//temp += _T("\r\n");
				int pos = m_Online.FindOneOf(temp);
				m_Online.Delete(pos, temp.GetLength() + 2);

				Command = _T("5\r\n");
				Command += m_Online;
				Command += _T("\r\n");
				for (int i = 0; i < number_Socket; i++)
				{
					if (i != pos)
						mSend(pSock[i].sockClient, Command);
				}

				closesocket(wParam);
				//loại bỏ thông tin socket đã đóng ra khỏi mảng
				for (int j = post; j < number_Socket - 1; j++)
				{
					pSock[j].sockClient = pSock[j + 1].sockClient;
					strcpy(pSock[j].Name, pSock[j + 1].Name);
				}
				number_Socket--;

				UpdateData(FALSE);
			}
		}break;
		}break;
	}
	case FD_CLOSE:
	{
		UpdateData();
		int post = -1;
		for (int i = 0; i < number_Socket; i++)
		{
			if (pSock[i].sockClient == wParam)
			{
				if (i < number_Socket)
					post = i;
			}
		}
		//lấy tên của socket kèm thông báo log out ra màn hình.
		m_msgString += pSock[post].Name;
		m_msgString += " logout\r\n";

		//gửi thông báo cho các Client khác rằng Client A nào đó đã log out
		Command = _T("3\r\n");
		Command += pSock[post].Name;
		Command += _T("\r\n");
		Command += _T(" logout\r\n");
		for (int i = 0; i < number_Socket; i++)
		{
			if (i != post)
				mSend(pSock[i].sockClient, Command);
		}
		//Xóa tên client ra khỏi danh sách client online
		CString temp(pSock[post].Name);
		int pos = m_Online.FindOneOf(temp);
		m_Online.Delete(pos, temp.GetLength() + 2);

		Command = _T("5\r\n");
		Command += m_Online;
		Command += _T("\r\n");
		for (int i = 0; i < number_Socket; i++)
		{
			if (i != pos)
				mSend(pSock[i].sockClient, Command);
		}
		closesocket(wParam);
		for (int j = post; j < number_Socket - 1; j++)
		{
			pSock[j].sockClient = pSock[j + 1].sockClient;
			strcpy(pSock[j].Name, pSock[j + 1].Name);
		}
		number_Socket--;
		UpdateData(FALSE);
	}break;

	}
	return 0;
	
}
	
	


int CServerDlg::checkLogin(char* name, char* pass)
{
	fstream file;
	char* namefile = "Accounts.txt";
	file.open(namefile);
	if (file.fail())
	{
		MessageBox(_T("Cant not connect to database to Login"), _T("ERROR"), MB_ICONERROR);
		return 0;
	}
	string ten, mk;
	while (!file.eof())
	{
		file >> ten >> mk;
		if (strcmp(name, ten.c_str()) == 0 && strcmp(pass, mk.c_str()) == 0)
		{
			file.close();
			return 1;
		}

	}
	file.close();
	return 0;
}

int CServerDlg::checkSignUp(char* name)
{
	fstream file;
	char* namefile = "Accounts.txt";
	file.open(namefile);
	if (file.fail())
	{
		MessageBox(_T("Cant not connect to database to Signup"), _T("ERROR"), MB_ICONERROR);
		return 0;
	}
	string ten, mk;
	while (!file.eof())
	{
		file >> ten >> mk;
		if (strcmp(name, ten.c_str()) == 0)
		{
			file.close();
			return 1;
		}

	}
	file.close();
	return 0;
}

int CServerDlg::updateDatabase(char* name, char* pass)
{
	char* namefile = "Accounts.txt";
	FILE* file;
	file = fopen(namefile, "a");
	if (!file)

	{
		MessageBox(_T("Cant not connect to database to Update"),_T("ERROR"),MB_ICONERROR);
		return 0;
	}

	fprintf(file, "%s",name);
	fprintf(file, " ");
	fprintf(file, "%s",pass);
	fprintf(file, "\n");
	fclose(file);
	return 1;
}

int CServerDlg::loadData()
{
	char* namefile = "Accounts.txt";
	fstream file;
	file.open(namefile, ios::in);

	if (file.fail())
	{
		MessageBox(_T("Cant not connect to database to load Manage Clients"), _T("ERROR"), MB_ICONERROR);
		return 0;
	}
	string ten, mk;
	CString tmp;
	while (!file.eof())
	{
		file >> ten >> mk;
		
		CString name(ten.c_str());
		if (name != tmp)
		{
			m_Manage += name;
			m_Manage += _T("\r\n");
		}
		tmp = name;

	}
	UpdateData(FALSE);
	file.close();
	return 1;
}




