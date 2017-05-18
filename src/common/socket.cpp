#include "socket.h"

using namespace std;
using namespace CITOOL;

Socket::Socket(void) : m_sock(INVALID_SOCKET), m_type(SOCK_STREAM), m_protocol(0), f_nonblocking(false)
{
	m_addr_len = sizeof(struct sockaddr_storage);
	memset(&m_addr, 0, m_addr_len);
}

Socket::Socket(const SOCKET &_s) : Socket()
{
	m_sock = _s;
	struct sockaddr_storage new_addr; // address information of the connected
	socklen_t sin_size;
	sin_size = sizeof new_addr;
	struct sockaddr* _addr = reinterpret_cast<sockaddr*>(&new_addr);
	getpeername(m_sock, _addr, &sin_size);
}

Socket::~Socket(void) 
{ 
	if (INVALID_SOCKET != m_sock)
	{
		int status = 0;

#ifdef _MSC_VER
			status = shutdown(m_sock, SD_BOTH);
			if (status == 0) { status = closesocket(m_sock); }
#else
			status = shutdown(m_sock, SHUT_RDWR);
			if (status == 0) { status = close(m_sock); }
#endif
	}
}

void Socket::Clear()
{
	m_sock = INVALID_SOCKET;
	m_addr_len = sizeof(struct sockaddr_storage);
	memset(&m_addr, 0, sizeof m_addr);
}

Socket::Socket(const Socket &inf_sock) 
{
	Clear();
	m_sock = inf_sock.m_sock;
	memcpy(&m_addr, &inf_sock.m_addr, inf_sock.m_addr_len);
	m_addr_len = inf_sock.m_addr_len;
}

void Socket::Create(const char* _port, const char* _host, int _family, int _type, int _protocol)
{
	m_type = _type;
	m_protocol = _protocol;
	m_family = _family;

	struct addrinfo *res;
	struct addrinfo hints;
	int status;
	struct addrinfo *p;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = _family;
	hints.ai_socktype = _type;
	hints.ai_protocol = _protocol;

	if (!_host)
	{
		hints.ai_flags = AI_PASSIVE; 
	}

	if ((status = getaddrinfo(_host, _port, &hints, &res)) != 0 ) 
	{
		throw SocketException(status);
	}

	for (p = res; p != NULL; p = p->ai_next) 
	{
		if ((m_sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == INVALID_SOCKET) 
		{
			continue;
		}
		
		SetAddr(p->ai_addr, (int)p->ai_addrlen);
		try
		{
			Bind();
		}
		catch (SocketException&)
		{
			Socket::Close(*this);
			continue;
		}
		break;
	}

	freeaddrinfo(res);

	if (p == NULL) 
	{
		Clear();
		throw SocketException(-1, "The creation and binding of the socket failed");
	}
}

void Socket::SetAddr(struct sockaddr* addr, int addr_len)
{
	memset(&m_addr, 0, sizeof(struct sockaddr_storage));
	memcpy(&m_addr, addr, addr_len);
	m_addr_len = addr_len;
}

int Socket::SendConfirmation(const ConfirmationHeader& _header)
{
	int result = send(m_sock, reinterpret_cast<const char*>(&_header), sizeof(ConfirmationHeader), 0);

	if (result == -1)
	{
		throw SocketException(errno, "send");
	}

	if (result != sizeof(ConfirmationHeader))
		throw SocketException(-1, "Sending header - been fail");

	return result;
}

int Socket::SendConfirmation(uint32_t _operation, size_t _size, const char* _user)
{
	ConfirmationHeader header;
	header.version = CISOCKVERSION;
	header.operation = _operation;
	header.size = _size;
	if (_user) 
	{
		size_t l = strlen(_user);
		if (l > 64)
			l = 64;

		memcpy(&header.user, _user, l);
	} 
	else
	{
		memset(&header.user, 0, sizeof(header.user));
	}
	memset(&header.data, 0, sizeof(header.data));

	int result = send(m_sock, reinterpret_cast<const char*>(&header), sizeof(ConfirmationHeader), 0);
	
	if (result == -1)
	{
		throw SocketException(errno, "send");
	}

	if (result != sizeof(ConfirmationHeader))
		throw SocketException(-1, "Sending header - been fail");

	return result;
}

int Socket::ReadConfirmation(ConfirmationHeader& _header)
{
	int result;
	int sz_h = sizeof(ConfirmationHeader);
	char* buf = new char[sz_h];
	result = recv(m_sock, buf, sz_h, 0);
	if (result == -1 || result == 0)
	{
		delete [] buf;
		throw SocketException(errno);
	}

	if (result != (int)sz_h)
	{
		delete [] buf;
		throw SocketException(-1, "Read incorrect header");
	}

	ConfirmationHeader& h = reinterpret_cast<ConfirmationHeader&>(*buf);
	_header.operation = h.operation;
	_header.size = h.size;
	_header.version = h.version;
	memcpy(&_header.user, &h.user, sizeof(h.user));
	memcpy(&_header.data, &h.data, sizeof(h.data));

	delete [] buf;

	return result;
}

int Socket::Read(raw_type *_buf, size_t _size_buf)
{
  int cnt;
  int rc;
  
  cnt = (int)_size_buf;
  
  while(cnt > 0)
  {
	rc = recv(m_sock, _buf, cnt, 0);
	if (rc < 0)					//Is read error?
	{
	  if(errno == EINTR)		//The call is terminated?
		continue;				//To repeat the reading
		
	  return -1;				//Return error code
	}
	if (rc == 0)				//The end of the message?
	  return int(_size_buf - cnt);	//Return incomplete counter ...
	  
	_buf += rc;
	cnt -= rc;
  }
  
  return (int)_size_buf;
}

int Socket::Send(const raw_type *_buf, size_t _size_buf)
{
	size_t total = 0;
	int n = 0;

	while(total < _size_buf)
	{
	  n = send(m_sock, _buf + total, int(_size_buf - total), 0);
	  if(n == -1)
		break;

	  total += n;
	}

	return int(n == -1 ? -1 : total);
}

void Socket::SetNonBlocking(bool _value)
{
	if (_value)
	{
		if (IsNonBlocking())
			return;

		f_nonblocking = true;
	}
	else
	{
		if (!IsNonBlocking())
			return;

		f_nonblocking = false;
	}
	
	u_long toggle = (_value) ? 1 : 0;
#ifdef _MSC_VER
	  if (ioctlsocket(m_sock, FIONBIO, &toggle) == -1)
	  {
		  throw SocketException(errno);
	  }
#else
	  //fcntl(sock, F_SETFL, O_NONBLOCK);
	  if (ioctl(m_sock, FIONBIO, (char *)&toggle) == -1)
	  {
		  throw SocketException(errno);
	  }
#endif
}

void Socket::SetLinger(size_t _seconds)
{
	struct linger _value;
	if ( _seconds > 0 )
	{
		_value.l_linger = (u_short)_seconds;
		_value.l_onoff = 1;
	}
	else 
	{
		_value.l_onoff = 0;
	}

	if (setsockopt(m_sock, SOL_SOCKET, SO_LINGER, (char*)&_value, sizeof(struct linger)) == -1) 
	{
		throw SocketException(errno);
	}
}

void Socket::SetSendBufSize(size_t _new_size)
{
	if (setsockopt(m_sock, SOL_SOCKET, SO_SNDBUF, (char*)&_new_size, sizeof(_new_size)) == -1) 
	{
		throw SocketException(errno);
	}
}

void Socket::SetReceiveBufSize(size_t _new_size)
{
	if (setsockopt(m_sock, SOL_SOCKET, SO_RCVBUF, (char*)&_new_size, sizeof(_new_size)) == -1) 
	{
		throw SocketException(errno);
	}
}

void Socket::SetKeepAlive(bool _value)
{
	int toggle = (_value) ? 1 : 0;
	if (setsockopt(m_sock, SOL_SOCKET, SO_KEEPALIVE, (char*)&toggle, sizeof(int)) == -1) 
	{
		throw SocketException(errno);
	}
}

void Socket::SetReuseAddr(bool _value)
{
	int toggle = (_value) ? 1 : 0;
	if (setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, (char*)&toggle, sizeof(int)) == -1) 
	{
		throw SocketException(errno);
	}
}

void Socket::SetDebug(bool _value)
{
	int toggle = (_value) ? 1 : 0;
	if (setsockopt(m_sock, SOL_SOCKET, SO_DEBUG, (char*)&toggle, sizeof(int)) == -1) 
	{
		throw SocketException(errno);
	}
}

bool Socket::IsDebug()
{
	int _value;
	socklen_t value_size = sizeof(_value);
	if (getsockopt(m_sock, SOL_SOCKET, SO_DEBUG, (char *)&_value, &value_size) == -1)
		return false;

	return (_value != 0);
}

bool Socket::IsReuseAddr()
{
	int _value;
	socklen_t value_size = sizeof(_value);
	if (getsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&_value, &value_size) == -1)
		return false;

	return (_value != 0);
}

bool Socket::IsKeepAlive()
{
	int _value;
	socklen_t value_size = sizeof(_value);
	if (getsockopt(m_sock, SOL_SOCKET, SO_KEEPALIVE, (char *)&_value, &value_size) == -1)
		return false;

	return (_value != 0);
}

bool Socket::IsLinger()
{
	struct linger _value;
	socklen_t value_size = sizeof(struct linger);
	if (getsockopt(m_sock, SOL_SOCKET, SO_LINGER, (char *)&_value, &value_size) == -1)
		return false;

	return (_value.l_onoff != 0);
}

int Socket::GetLingerSeconds()
{
	struct linger _value;
	socklen_t value_size = sizeof(struct linger);
	if (getsockopt(m_sock, SOL_SOCKET, SO_LINGER, (char *)&_value, &value_size) == -1)
		return -1;

	return _value.l_linger;
}

int Socket::GetSendBufSize()
{
	int _value;
	socklen_t value_size = sizeof(_value);
	if (getsockopt(m_sock, SOL_SOCKET, SO_SNDBUF, (char *)&_value, &value_size) == -1)
		return -1;

	return _value;
}

int Socket::GetReceiveBufSize()
{
	int _value;
	socklen_t value_size = sizeof(_value);
	if (getsockopt(m_sock, SOL_SOCKET, SO_RCVBUF, (char *)&_value, &value_size) == -1)
		return -1;

	return _value;
}

void SocketServer::Bind()
{
#ifdef __linux__
	int yes = 1;
#else
	char yes = '1';
#endif

	if (setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) 
	{
		throw SocketException(errno, "Bind: setsockopt");
	}

	if (bind(m_sock, reinterpret_cast<struct sockaddr*>(&m_addr), m_addr_len) == -1) 
	{
		throw SocketException(errno, "BindLocal: bind");
	}
}

void SocketServer::Listen(const char* _port, const char* _host, const uint32_t _backlog)
{
	Create(_port, _host);
	if (listen(m_sock, _backlog) == -1)
		throw SocketException(errno, "Listen");
}

bool SocketServer::Accept(Socket &_socket) 
{
	SOCKET new_fd = INVALID_SOCKET;
	struct sockaddr_storage their_addr; 
	socklen_t sin_size;
	sin_size = sizeof their_addr;
	struct sockaddr* client_addr = reinterpret_cast<sockaddr*>(&their_addr);

	new_fd = accept(m_sock, client_addr, &sin_size);
	if (new_fd == INVALID_SOCKET || errno == EINTR) // Error or interrupted ...
	{
		return false;
	}

	_socket(new_fd); 
	
	_socket.SetAddr(client_addr, sin_size);
	return true;
}

void SocketClient::Bind()
{
	if (connect(m_sock, reinterpret_cast<struct sockaddr*>(&m_addr), m_addr_len) == -1) 
	{
		throw SocketException(-1, "client: failed to connect");
	}
}

void SocketClient::Connect(const char* _host, const char* _port)
{
	Create(_port, _host); 
}

ostream& CITOOL::operator << (ostream& io, CITOOL::Socket& s)
{
	char ipstr[INET6_ADDRSTRLEN];
	void *addr;
	const char *ipver;

	int port;

	if (s.m_addr.ss_family == AF_INET) 
	{ // IPv4
		struct sockaddr_in *ipv4 = (struct sockaddr_in *)&s.m_addr;
		addr = &(ipv4->sin_addr);
		ipver = "IPv4";
		port = ntohs(ipv4->sin_port);
	} else 
	{ // IPv6
		struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)&s.m_addr;
		addr = &(ipv6->sin6_addr);
		ipver = "IPv6";
		port = ntohs(ipv6->sin6_port);
	}
	// перевести IP в строку 
	inet_ntop(s.m_addr.ss_family, addr, ipstr, sizeof ipstr);

	char host[NI_MAXHOST];
	getnameinfo((struct sockaddr *)&s.m_addr, s.m_addr_len, host, sizeof(host), NULL, 0, NI_NUMERICHOST);

	io << "\n--------------- Summary of socket settings -------------------" << endl;
	io << "   Socket Id:		" << s.m_sock << endl;
	io << "   " << ipver << ":		" << ipstr << endl;
	io << "   host:		" << host << endl;
	io << "   port:		" << port << endl;
	io << "   debug:		" << (s.IsDebug() ? "true" : "false" ) << endl;
	io << "   reuse addr:		" << (s.IsReuseAddr() ? "true" : "false" ) << endl;
	io << "   keep alive:		" << (s.IsKeepAlive() ? "true" : "false" ) << endl;
	io << "   send buf size:	" << s.GetSendBufSize() << endl;
	io << "   recv bug size:	" << s.GetReceiveBufSize() << endl;
	io << "   nonblocking:		" << (s.IsNonBlocking() ? "true" : "false" ) << endl;
	io << "   linger on:		" << (s.IsLinger() ? "true" : "false" ) << endl;
	io << "   linger seconds:	" << s.GetLingerSeconds() << endl;
	io << "----------- End of Summary of socket settings ----------------" << endl;
	return io;
}

ostream& CITOOL::operator << (ostream& io, SocketException& ex)
{
	io << "--------------- ERROR -------------------" << endl;
	io << ex.err_code << " : " << ex.err_msg.c_str() << endl;
	io << "-----------------------------------------" << endl;
	return io;
}
