#include <iostream>
#include "cmdline.h"
#include "server.h"
#include "daemon.h"
#include "config.h"

#ifdef _USE_VLD
#include <vld.h>
#endif

using namespace CITOOL;
using namespace T1;

namespace {
    struct Result {
        int code;
        std::string message;

        Result() : code(0) {}

        operator int() { return code; }

        Result &operator=(int _code) {
            this->code = _code;
            return *this;
        }
    };

	class CmdThread 
	{
		std::thread &t;
	public:
		explicit CmdThread(std::thread &_t) : t(_t) {}

		~CmdThread()
		{
			if (t.joinable())
				t.join();
		}

		CmdThread(CmdThread const &) = delete;
		CmdThread &operator=(CmdThread const &) = delete;
	};

	void Command(Server &s) 
	{
		while (!s.is_terminate()) 
		{
			std::string s_command;
			getline(std::cin, s_command);
			if (s_command == ":quit") {
				s.Stop();
				std::cout << "Stopping ..." << std::endl;
			}
		}
		std::cout << "Stopped !!!" << std::endl;
	}

    static void show_usage() {
        std::cout << "Usage: scannerd [ <option> ]\n"
                  << "Options:\n"
                  << " Without option -\tStart as application.\n"
                  << " --help(-h)\t\t\tThis description.\n"
                  << " --stop(-x)\t\t\tStop server.\n"
                  << " --daemon(-d)\t\t\tStart as daemon.\n"
                  << std::endl;
    }

}

int main(int argc, char* argv[])
{
	Result res;

	if (CmdLine::option_exists(argv, argv + argc, "--help", "-h"))
	{
		show_usage();
		return res;
	}

    bool asdaemon = CmdLine::option_exists(argv, argv + argc, "--daemon", "-d");

#ifndef _MSC_VER
    if (!asdaemon && argc > 1)
	{
        CiDaemon Daemon;
        Daemon.SetLogLevel(LOG_DEBUG);
        Daemon.Open();

        if(CmdLine::option_exists(argv, argv + argc, "--stop", "-x")) 
		{
            Daemon.Stop();
            exit(EXIT_SUCCESS);
        }

        exit(EXIT_FAILURE);
    }
#else
	asdaemon = false;
#endif
    Server Scanner;

    try 
	{
		if (asdaemon)
		{
#ifndef _MSC_VER
			CiDaemon Daemon;
			Daemon.SetLogLevel(LOG_DEBUG);
			Daemon.setServerStop(std::bind(&Server::Stop, &Scanner));

			if (Daemon.Demonize() == 0)
			{
				std::string flog("/tmp/scannerd.log");
				Scanner.SetLog(CITOOL::levelValue("DEBUG"), flog);
				Scanner.Start();
			}
			else
			{
				return res;
			}
#endif
		}
		else
		{	
			std::thread tcom = std::thread(Command, std::ref(Scanner));
			CmdThread cmdt(tcom);
			std::string flog("scannerd.log");
			Scanner.SetLog(CITOOL::levelValue("DEBUG"), flog, true);
			Scanner.Start();
		}
    }
    catch (SocketException &ex) 
	{
        ERR(Scanner.LOG, ex.err_msg);
    }
    catch (...) 
	{
        ERR(Scanner.LOG, "Error: Server is crashed.");
		Scanner.terminate();
    }

	return res;
}

