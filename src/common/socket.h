/**
* \file		socket.h
* \brief	Wrapper over the concept of SOCKET.
*
* \author	Leonid Belyasnik (leonid.belyasnik@gmail.com)
* \date		08/05/2017
*/

#ifndef SOCKET_H
#define SOCKET_H

#pragma once

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <iostream>

#ifndef _MSC_VER
#include <unistd.h>			 
#include <netdb.h>			 
#include <sys/types.h>       
#include <sys/socket.h>		 
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/shm.h> 
#include <sys/sem.h> 
#include <netinet/in.h>		 
#include <arpa/inet.h>		 
#include <sys/ioctl.h>
// Type used for raw data on this platform
typedef char raw_type;       
#else
#include <winsock2.h>		 
#include <Ws2tcpip.h>
#include <windows.h>
#include <io.h>
// Type used for raw data on this platform
typedef char raw_type;       
// Types to simplify portability, may not be necessary in the future.
typedef unsigned int uint32_t;
typedef unsigned short in_port_t;
#endif


#ifdef _MSC_VER
#define set_errno(e)	SetLastError((e))
#define isvalidsock(s)	((s) != SOCKET_ERROR)
#define bzero(b, n)		memset((b), 0, (n))
#define sleep(t)		Sleep((t) * 1000)
typedef int socklen_t;
#else
#define set_errno(e)	errno = (e)
#define isvalidsock(s)	((s) >= 0)
#define WSAEWOULDBLOCK	EWOULDBLOCK
typedef int SOCKET;
#define INVALID_SOCKET (SOCKET)(~0)
#endif

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif

#define CISOCKVERSION	1
/**
* \defgroup	IDOPERATION IDs operations.
* \brief	IDs operations.
*
*  @{
*/
#define OP_HELLO		10
#define OP_OK			11
#define OP_YES			12
#define OP_NO			13
#define OP_MSG			14
#define OP_COMMAND		15
#define OP_QUESTION		16
#define OP_ANSWER		17	
#define OP_SET			18
#define OP_GET			19
#define OP_ERR			100	
/** @} */

#pragma pack(push,1)
struct ConfirmationHeader
{
	uint32_t version;
	uint32_t operation;
	size_t size;
	uint8_t user[64];
	uint8_t data[256];
};
#pragma pack(pop)

namespace CITOOL {

	class SocketException
	{
	public:
		int err_code;
		std::string err_msg;
	public:
		SocketException(void) throw() : err_code(0), err_msg(0) {}
		SocketException(int _code) throw() : err_code(_code), err_msg(gai_strerror(_code)) {}
		SocketException(int _code, const std::string& _msg) : err_code(_code), err_msg(_msg) {}
		SocketException(const SocketException& _ex) : err_code(_ex.err_code), err_msg(_ex.err_msg) {}
		~SocketException(void) {}

		friend std::ostream& operator << (std::ostream&, SocketException&);
	};

	class Socket
	{
	protected:
		SOCKET m_sock;
		struct sockaddr_storage m_addr;
		int m_addr_len;
	private:
		int m_type;
		int m_protocol;
		int m_family;
		bool f_nonblocking;
	protected:
		void Clear();
		virtual void Bind() {}

		bool IsDebug();
		bool IsReuseAddr();
		bool IsKeepAlive();
		bool IsLinger();
		int GetLingerSeconds();
	public:
		Socket(void);
		Socket(const SOCKET &_s);
		Socket(const Socket &inf_sock);
		virtual ~Socket(void);
		void Create(const char* _port, const char* _host = NULL, int _family = AF_INET, int _type = SOCK_STREAM, int _protocol = IPPROTO_TCP);
		void SetAddr(struct sockaddr* addr, int addr_len);
		void operator()(const SOCKET &_s) { Clear(); m_sock = _s; }
		operator SOCKET() const { return GetSocket(); }

		virtual int ReadConfirmation(ConfirmationHeader& _header);
		virtual int SendConfirmation(const ConfirmationHeader& _header);
		virtual int SendConfirmation(uint32_t _operation, size_t _size, const char* _user = nullptr);
		virtual int Read(raw_type *_buf, size_t _size_buf);
		virtual int Send(const raw_type *_buf, size_t _size_buf);

		inline bool IsNonBlocking() { return f_nonblocking; }
		int GetSendBufSize();
		int GetReceiveBufSize();

		void SetNonBlocking(bool _value);
		void SetLinger(size_t _seconds);
		void SetSendBufSize(size_t _new_size);
		void SetReceiveBufSize(size_t _new_size);
		void SetKeepAlive(bool _value);
		void SetReuseAddr(bool _value);
		void SetDebug(bool _value);

		inline const SOCKET& GetSocket(void) const { return m_sock; }
		inline void SetSocket(const SOCKET &_s) { Clear(); m_sock = _s; }
		inline struct sockaddr_in* GetAddr(void) { return reinterpret_cast<struct sockaddr_in *>(&this->m_addr); }

		friend std::ostream& operator << (std::ostream&, Socket&);

		static int inline Init(void)
		{
#ifdef _MSC_VER
			WSADATA wsa_data;
			return WSAStartup(MAKEWORD(2, 2), &wsa_data);
#else
			return 0;
#endif
		}

		static int inline Quit(void)
		{
#ifdef _MSC_VER
			return WSACleanup();
#else
			return 0;
#endif
		}

		static int inline Shutdown(const SOCKET& _s)
		{
			int status = 0;
#ifdef _MSC_VER
			status = shutdown(_s, SD_BOTH);
#else
			status = shutdown(_s, SHUT_RDWR);
#endif
			return status;
		}

		static int inline Close(const SOCKET& _s)
		{
			int status = 0;

#ifdef _MSC_VER
			status = shutdown(_s, SD_BOTH);
			if (status == 0) { status = closesocket(_s); }
#else
			status = shutdown(_s, SHUT_RDWR);
			if (status == 0) { status = close(_s); }
#endif

			return status;
		}

		static int inline Terminate(const SOCKET& _s)
		{
			int status = 0;

#ifdef _MSC_VER
			status = closesocket(_s);
#else
			status = close(_s);
#endif

			return status;
		}
	};

	class SocketServer : public Socket
	{
		virtual void Bind();
	public:
		SocketServer(void) : Socket() {}
		virtual ~SocketServer(void) {}
		void Listen(const char* _port, const char* _host = NULL, const uint32_t _backlog = 0);
		bool Accept(Socket &_socket);
	};

	class SocketClient : public Socket
	{
		virtual void Bind();
	public:
		SocketClient(void) : Socket() {}
		SocketClient(const std::string& _osuser) : Socket() {}
		virtual ~SocketClient(void) {}
		void Connect(const char* _host, const char* _port);
	};

} // CITOOL

#endif // SOCKET_H
