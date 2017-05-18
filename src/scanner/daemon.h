/**
* \file		daemon.h
* \brief	Implement concept of DAEMON.
*
* \author	Leonid Belyasnik (leonid.belyasnik@gmail.com)
* \date		08/05/2017
*/

#ifndef DAEMON_H
#define DAEMON_H

#pragma once
#ifndef _MSC_VER
#include <sys/types.h>
#include <sys/stat.h>
#include <cerrno>
#include <signal.h>
#include <syslog.h>
#include <functional>

namespace T1 {
	class CiDaemon;

	class HandlerManager
	{
	private:
		HandlerManager() {};

	public:
		static HandlerManager* get_instance();
		void register_handler(int signum, CiDaemon * handler);

	private:
		static void dispatch_signal(int signum, siginfo_t * info, void * context);

	private:
		static HandlerManager* the_manager;
		CiDaemon * handler_object;
	};

	typedef std::function<void(void)> ServerFn;
} // T1
#endif
namespace T1 {
	class CiDaemon
	{
#ifndef _MSC_VER
		pid_t PID;
		int errcode;
		int logLevel;
		char name[64];
		CiDaemon(const CiDaemon&) = delete;
		CiDaemon& operator=(const CiDaemon&) = delete;
		/* set up signal processing */
		int ConfigureSignalHandlers();
	private:
		friend class HandlerManager;
		void install_signal_handler(int signum)
		{
			(HandlerManager::get_instance())->register_handler(signum, this);
		}
		void receive_sig_function( int status );
		ServerFn ServerStop;
		ServerFn ServerReload;
	public:
		CiDaemon(int _log_level = LOG_INFO);
		~CiDaemon();
		inline bool IsNotExists() { return (errcode == ENOENT); }
		inline bool IsChild() { return (PID != -1); }
		void SetLogLevel(int _level);
		int Demonize();
		int Open();
		void Stop();
		void Restart();
		void Reload();
		void setServerStop(ServerFn fcb) { ServerStop = fcb; }
		void setServerReload(ServerFn fcb) { ServerReload = fcb; }
#endif
	};

} // T1

#endif //DAEMON_H

