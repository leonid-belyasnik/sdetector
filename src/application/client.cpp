#include "client.h"
#include "common.h"
#include <memory>
#ifndef _MSC_VER
#include <pwd.h>
#endif

using namespace CITOOL;
using namespace T1;

bool ConnectSocket::Hello()
{
	ConfirmationHeader header;
	memset(&header, 0, sizeof(ConfirmationHeader));
	Socket::ReadConfirmation(header);
	if (header.operation != OP_HELLO)
	{
		throw SocketException(-1, "HELLO FAIL");
	}

	Socket::SendConfirmation(OP_OK, 0);

	osuname = ConnectSocket::os_user();
	if (osuname.empty() || osuname.length() > 64)
		osuname = "Unknown";

	return true;
}

std::string ConnectSocket::os_user()
{
#ifdef _MSC_VER
	char user_name[256] = { 0 };
	DWORD user_name_size = sizeof(user_name);
	if (::GetUserNameA(user_name, &user_name_size))
		return std::string(user_name);
#else
	struct passwd *pw = getpwuid(getuid());
	if (pw)
		return std::string(pw->pw_name);
#endif

	return std::string("");
}

int ConnectSocket::SendFile(const char* filepath)
{
	ConfirmationHeader header;
	memset(&header, 0, sizeof(ConfirmationHeader));
	header.operation = OP_QUESTION;
	header.size = 0;
	header.version = CISOCKVERSION;
	memcpy(header.user, osuname.c_str(), osuname.length());
	memcpy(header.data, filepath, strlen(filepath));

	return SendConfirmation(header);
}
