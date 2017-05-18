#include "server.h"
#include "common.h"

using namespace std;
using namespace CITOOL;
using namespace T1;

bool ClientSocket::SendHello()
{
	DEBUG(server->LOG, "NEW CLIENT HELLO --------------------v");
	DEBUG(server->LOG, *this);

	Socket::SendConfirmation(OP_HELLO, 0);

	ConfirmationHeader oph;
	memset(&oph, 0, sizeof(ConfirmationHeader));
	Socket::ReadConfirmation(oph);
	if (oph.operation != OP_OK)
		return false;

	return true;
}

Server::Server()
{
	Socket::Init();
	LOG = shared_ptr<CITOOL::Log>(new CITOOL::Log);
}

Server::~Server()
{
	Socket::Quit();
}

void Server::SetLog(int level, const string& filename, bool f_to_console)
{
	LOG->Clear();

	LOG->addLogger(new CITOOL::ToFile(level, filename));

	if (f_to_console)
		LOG->addLogger(new CITOOL::ToConsole(level));
}

void Server::start()
{
	if (!MList.is_exists(MLFILE))
	{
		WARNING(LOG, "Not found file malware_list");
		WARNING(LOG, "Server won't be started");
		terminate();
		return;
	}
	MList.load(MLFILE);

	if (MList.is_empty())
	{
		WARNING(LOG, "Failed to load information from file malware_list");
		WARNING(LOG, "Server won't be started");
		terminate();
		return;
	}
    Run(SVRPORT, SVRHOST);
}

void Server::Run(const char* c_port, const char* c_host)
{
	_svr_socket.Listen(c_port, c_host);
    set_stop(false);
	INFO(LOG, string_format("RUN: %s:%s", c_host, c_port));
	DEBUG(LOG, "MAIN SERVER SOCKET -------------------v");
	DEBUG(LOG, _svr_socket);

	start_pool();

	while (!is_terminate())
	{
        auto esocket = shared_ptr<ClientSocket>(new ClientSocket(this));

        if (!_svr_socket.Accept(*esocket.get()))
		{
			break;
		}

		enqueue(bind(&Server::process, move(esocket)));
	}
}

void Server::process(shared_ptr<ClientSocket> s)
{
	auto this_id = std::this_thread::get_id();

	try
	{
		if (!s)
			throw SocketException(-300, string_format("[%d] Undefined socket", this_id));

		if (!s->SendHello())
			throw SocketException(-301, string_format("[%d] HELLO fail", this_id));

		ConfirmationHeader header;
		memset(&header, 0, sizeof(ConfirmationHeader));
        bool fbad = false;
		while (s->ReadConfirmation(header) > 0 || !fbad)
		{
			if (header.operation == OP_QUESTION)
			{
				const char* cuname = reinterpret_cast<const char*>(&header.user);
				if (!cuname)
					throw SocketException(-302, string_format("[%d] Incorrect user name", this_id));

				const char* cfilename = reinterpret_cast<const char*>(&header.data);
				if (!cfilename)
					throw SocketException(-303, string_format("[%d] [%s] Incorrect path to file", this_id, cuname));

				string spref = string_format("[%d] [%s] ", this_id, cuname);
				INFO(s->server->LOG, string_format("[%d] [%s] PROCESS: %s", this_id, cuname, cfilename));

				BMSeeker mm_seeker(cfilename);

				if (!mm_seeker.is_opened())
				{
					throw SocketException(-304, string_format("[%d] [%s] Failed to open file: %s", this_id, cuname, cfilename));
				}

				for (int i = 0; i < s->server->MList.size(); ++i)
				{
					const SeekData sd = s->server->MList[i];

					int pos = mm_seeker.find(sd);
					if (pos == -1)
						continue;

					ConfirmationHeader send_header;
					memset(&send_header, 0, sizeof(ConfirmationHeader));
					send_header.operation = OP_ANSWER;
					send_header.size = pos;
					send_header.version = CISOCKVERSION;
					memcpy(&send_header.data, sd.s_guid.c_str(), sd.s_guid.length());
					s->SendConfirmation(send_header);
				}
				
				s->SendConfirmation(OP_ANSWER, 0);
			}
			else
			{
				fbad = true;
				s->SendConfirmation(OP_ERR, 0);
			}

			memset(&header, 0, sizeof(ConfirmationHeader));
		}
	}
	catch (SocketException& ex)
	{
		if (ex.err_code != 0)
		{
			ERR(s->server->LOG, ex);
			s->SendConfirmation(OP_ERR, 0);
		}
	}
    catch (...){}

    DEBUG(s->server->LOG, string_format("[%d] Close connect", this_id));
    s.reset();
}

void Server::Stop()
{
	INFO(LOG, "STOP");

    Socket::Shutdown((SOCKET)_svr_socket);
    DEBUG(LOG, "Shutdown server");

	stop();

	DEBUG(LOG, "Finish all threads");
	Socket::Terminate((SOCKET)_svr_socket);
	DEBUG(LOG, "Terminate sockets");
}
