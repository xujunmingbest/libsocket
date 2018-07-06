#include "libsocket.h"
#include <thread>
using namespace std;
bool CPP_Server::Open(int port) {

	WSAStartup(MAKEWORD(2, 2), &WsaData);
	SockS = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in SeverAddr;
	SeverAddr.sin_family = PF_INET;
	SeverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	SeverAddr.sin_port = htons(port);
	::bind(SockS, (SOCKADDR*)&SeverAddr, sizeof(SOCKADDR));

	if (::listen(SockS, 1) == 0)
	{
		cout << "服务程序已启动" << endl;
		return true;
	}
	else
	{
		cout << "服务程序启动失败，服务退出" << endl;
		return false;
	}
}

void CPP_Server::Acpt() {
	SOCKET SockC;
	sockaddr_in CliAddr;
	int Len = sizeof(SOCKADDR);
	while (1)
	{
		SockC = accept(SockS, (SOCKADDR*)&CliAddr, &Len);

		if (SockC == SOCKET_ERROR) return;

		thread t(sf, SockC);
		t.detach();
		//snprintf(UserAllInfo.Ip, 16, "%s", inet_ntoa(CliAddr.sin_addr));
		cout << inet_ntoa(CliAddr.sin_addr) << endl;
		cout << ntohs(CliAddr.sin_port) << endl;
		//UserAllInfo.port = ntohs(CliAddr.sin_port);
		//UserAllInfo.SockC = SockC;
		//ThreadMan->ManCreateThread((LPTHREAD_START_ROUTINE)CliFunEntry, &UserAllInfo);
	}
}


void CPP_Server::Destroy()
{
	if (SockS != INVALID_SOCKET) closesocket(SockS);
	SockS = INVALID_SOCKET;
	WSACleanup();
}
void CPP_Server::SetServfun(Servfun sft) {
	sf = sft;
}

string DataReceivingSending::R(SOCKET sockClient, int rl) {
	char *buff = new char[rl];
	int lena = 0;
	while (lena < rl) {
		int templ = recv(sockClient, buff + lena, rl - lena, 0);
		if (templ == 0 || templ == -1) {
			delete[]buff;
			return "";
		}
		lena += templ;
	}
	string r(buff, rl);
	delete[]buff;
	return r;
};

bool DataReceivingSending::Send(SOCKET sockClient, string &buff) {
	if (send(sockClient, buff.c_str(), buff.length(), 0) <= 0) return false;
	return true;
}

bool DataReceivingSending::Recv(SOCKET sockClient, int len, string &r) {
	r = R(sockClient, len);
	if (r.length() == 0) return false;
	return true;
}


bool protocolDemo::RecvFIRSTBYTES(SOCKET st, uint &len)
{
	string r;
	if (!Recv(st, 2, r)) return false;

	len = 0;
	len = (r.c_str()[0] & 0xff);
	len = len << 8;
	len += (r.c_str()[1] & 0xff);
	if (len != HEADFIRSTLEN) return false;
	return true;
}


bool protocolDemo::RecvSECONDBYTES(SOCKET st, uint &len) {
	string r;
	if (!Recv(st, 4, r)) return false;

	len = 0;
	len = (r.c_str()[0] & 0xff);
	len = len << 8;
	len += (r.c_str()[1] & 0xff);
	len = len << 8;
	len += (r.c_str()[2] & 0xff);
	len = len << 8;
	len += (r.c_str()[3] & 0xff);
	return true;
}
bool protocolDemo::RecvFIRST(SOCKET st, uint &len, string &first) {
	if (!Recv(st, len, first)) return false;
	return true;
}
bool protocolDemo::RecvSECOND(SOCKET st, uint &len, string &second)
{
	if (!Recv(st, len, second)) return false;
	return true;
}

string protocolDemo::GenerateFIRSTBYTES(uint ln) {
	char le[2];
	le[0] = (ln & 0xffff) >> 8;
	le[1] = ln & 0xff;
	return string(le, 2);

}
string protocolDemo::GenerateSECONDBYTES(uint ln) {
	char le[4];
	le[0] = (ln & 0xffffffff) >> 24;
	le[1] = (ln & 0xffffff) >> 16;
	le[2] = (ln & 0xffff) >> 8;
	le[3] = ln & 0xff;
	return string(le, 4);
}

string protocolDemo::GenerateErrRet(string &OriginHead, int errorCode) {
	if (OriginHead.length() != HEADFIRSTLEN) return "";
	string f = GenerateFIRSTBYTES(HEADFIRSTLEN);

	HeadDemo H;
	memcpy(&H, OriginHead.c_str(), HEADFIRSTLEN);
	snprintf(H.RequestType, sizeof(H.RequestType), "RETMSG");
	string f_s((char*)&H, HEADFIRSTLEN);

	string s_s = Ret_Msgs[errorCode];
	string s = GenerateSECONDBYTES(s_s.length());
	string fina = f + s + f_s + s_s;
	return fina;
}





bool SynchroShortConnection::Start(string Ip, string port) {
	//加载套接字  
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("Failed to load Winsock");
		return false;
	}

	SOCKADDR_IN addrSrv;
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(atoi(port.c_str()));
	addrSrv.sin_addr.S_un.S_addr = inet_addr(Ip.c_str());

	//创建套接字  
	sc = socket(AF_INET, SOCK_STREAM, 0);
	if (SOCKET_ERROR == sc) {
		printf("Socket() error: %d", WSAGetLastError());
		return false;
	}
	//向服务器发出连接请求  
	if (connect(sc, (struct  sockaddr*)&addrSrv, sizeof(addrSrv)) == INVALID_SOCKET) {
		printf("Connect failed %d", WSAGetLastError());
		return false;
	}
	return true;
};

void SynchroShortConnection::Destroy() {
	closesocket(sc);
	WSACleanup();
}

bool SynchroShortConnection::Connect(string Ip, string port) {
	int count = 0;
	while (!Start(Ip, port) && count <= 10)
	{
		Destroy();
		Sleep(1000);
		count++;
		if (count > 10)
		{
			return false;
		}
	}
	return true;
};

void SynchroShortConnection::SetTimeOut(int nNetTimeout) {
	setsockopt(sc, SOL_SOCKET, SO_RCVTIMEO, (char *)&nNetTimeout, sizeof(int));
}
