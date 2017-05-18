/**
* \file		client.h
* \brief	Client operations.
*
* \author	Leonid Belyasnik (leonid.belyasnik@gmail.com)
* \date		08/05/2017
*/

#ifndef CLIENT_H
#define CLIENT_H

#pragma once

#include "socket.h"

/**
*	namespace T1
*	\brief Test 1
*/
namespace T1 {
	/**
	* \brief	Client connect class.
	*/
	class ConnectSocket : public CITOOL::SocketClient
	{
		std::string osuname;	///< OS username
	public:
		/**
		* Constructor.
		*/
		ConnectSocket() {}
		/**
		* Destructor.
		*/
		~ConnectSocket() {}
		/**
		* Handshake to server.
		*
		* \return	Successful answer.
		*/
		bool Hello();
		/**
		* Send request to server.
		*
		* \param [in]	path	Path to file.
		*
		* \return	Size answer header.
		*/
		int SendFile(const char* filepath);
		/**
		* Get OS username.
		*
		* \return	OS username.
		*/
		static std::string os_user();
	};

} // T1

#endif // CLIENT_H
