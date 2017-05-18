#include <stdio.h>
#include <iostream>
#include "common.h"
#include "config.h"
#include "client.h"

#ifdef _USE_VLD
#include <vld.h>
#endif

using namespace CITOOL;
using namespace T1;

int main(int argc, char* argv[])
{
	int res = 0;

	const char filename[] = TESTFILE;

	Socket::Init();

	try
	{
		ConnectSocket connect_socket;
		connect_socket.Connect(SVRHOST, SVRPORT);

		if (!connect_socket.Hello())
			throw SocketException(-1, "Hello fail !!!");

		std::string osuname = ConnectSocket::os_user();
		if (osuname.empty() || osuname.length() > 64)
			osuname = "Unknown";

		ConfirmationHeader header;
		memset(&header, 0, sizeof(ConfirmationHeader));
		header.operation = OP_QUESTION;
		header.size = 0;
		header.version = CISOCKVERSION;
		memcpy(header.user, osuname.c_str(), osuname.length());
		memcpy(header.data, filename, strlen(filename));
		if (connect_socket.SendConfirmation(header) > 0)
		{
			memset(&header, 0, sizeof(ConfirmationHeader));
			while (connect_socket.ReadConfirmation(header) > 0)
			{
				if (header.operation != OP_ANSWER)
				{
					if (header.operation == OP_ERR)
						throw SocketException(-3, "Server returned ERROR !!!");
					else
						throw SocketException(-4, "Server not answered !!!");
				}

				if (header.size == 0)
				{
					std::cout << "Server returned 0" << std::endl;
					break;
				}

				const char* cguid = reinterpret_cast<const char*>(&header.data);
				if (!cguid)
					throw SocketException(-5, "Incorrect GUID");

				std::cout << "Server found GUID: " << cguid << " in position " << header.size << std::endl;

				memset(&header, 0, sizeof(ConfirmationHeader));
			}
		}
		else
		{
			throw SocketException(-2, "SendConfirmation fail !!!");
		}
	}
	catch (SocketException& ex)
	{
		std::cerr << "ERROR " << ex.err_code << ": " << ((ex.err_msg.empty()) ? "" : ex.err_msg.c_str()) << std::endl;
		res = ex.err_code;
	}

	Socket::Quit();

    return res;
}

