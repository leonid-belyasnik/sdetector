/**
* \file		server.h
* \brief	Server operations.
*
* \author	Leonid Belyasnik (leonid.belyasnik@gmail.com)
* \date		08/05/2017
*/

#ifndef SERVER_H
#define SERVER_H

#pragma once

#include <string>
#include "usepool.h"
#include "socket.h"
#include "log.h"
#include "malware_list.h"

/**
*	namespace T1
*	\brief Test 1
*/
namespace T1 {

	class Server;
	/**
	* \brief	Client connect class.
	*/
	class ClientSocket : public CITOOL::Socket
	{
	public:
		Server* server;	///< Server object pointer.
	private:
		/**
		* Do not use default constructor.
		*/
		ClientSocket() = delete;
		/**
		* Do not use copy constructor.
		*/
		ClientSocket(const ClientSocket&) = delete;
	public:
		/**
		* Constructor.
		*
		* \param [in]	_server	Pointer to server object.
		*/
		ClientSocket(Server* _server) : CITOOL::Socket(), server(_server) {}
		/**
		* Destructor.
		*/
		~ClientSocket() {}
		/**
		* Handshake to client.
		*
		* \return	Successful answer.
		*/
		bool SendHello();
	};
	/**
	* \brief	Implemented server class.
	*/
	class Server : public CITOOL::UsePool
	{
		CITOOL::SocketServer _svr_socket;	///< Server socket.
	private:
		/**
		* Do not use copy constructor.
		*/
		Server(const Server&) = delete;
		/**
		* Static function of processing connect. Use for tasks in the pool.
		*
		* \param [in]	s	Pointer to client socket.
		*/
		static void process(std::shared_ptr<ClientSocket> s);
	protected:
		/**
		* Start server.
		*
		* \param [in]	c_port	Listen port.
		* \param [in]	c_host	Server host.
		*/
		void Run(const char* c_port, const char* c_host);
	public:
		/**
		* Constructor.
		*/
		Server();
		/**
		* Destructor.
		*/
		~Server();
		/**
		* Implement virtual function start.
		*/
		void start();
		/**
		* Start process.
		*/
		inline void Start() { start(); }
		/**
		* Stop process.
		*/
		void Stop();
		/**
		* Establish write to log.
		*
		* \param [in]	level	Log level.
		* \param [in]	c_host	Log file name.
		* \param [in]	f_to_console	Output log to console.
		*/
		void SetLog(int level, const std::string& filename, bool f_to_console = false);
	public:
		std::shared_ptr<CITOOL::Log> LOG;	///< Log object pointer.
		MalwareList MList;	///< List data for search.
	};

} // T1

#endif // SERVER_H