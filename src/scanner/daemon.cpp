#include "daemon.h"

#ifndef _MSC_VER

#include <fcntl.h>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <unistd.h>

#define OPEN_MAX_GUESS 256

const char *const cPIDFile = "/tmp/scannerd.pid";
const char *const clogPrefix= "scannerd";

using namespace T1;

HandlerManager* HandlerManager::the_manager;

HandlerManager* HandlerManager::get_instance()
{
    if (!the_manager)
        the_manager = new HandlerManager;

    return the_manager;
}

void HandlerManager::register_handler(int signum, CiDaemon * handler)
{
    struct sigaction act;

    memset(&act, 0, sizeof(struct sigaction));

    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = HandlerManager::dispatch_signal;

    handler_object = handler;
    sigaction(signum, &act, NULL);

}

void HandlerManager::dispatch_signal(int signum, siginfo_t * info, void * context)
{
    CiDaemon* d = get_instance()->handler_object;
	if (d)
	{
		d->receive_sig_function(signum);
	}
}

CiDaemon::CiDaemon(int _log_level) : PID(-1), errcode(0), logLevel(_log_level) 
{
    /* open the system log - here we are using the LOCAL0 facility */
    openlog(clogPrefix, LOG_PID|LOG_CONS|LOG_NDELAY|LOG_NOWAIT,LOG_LOCAL0);

    (void)setlogmask(LOG_UPTO(logLevel)); /* set logging level */
}

CiDaemon::~CiDaemon() 
{
    closelog();
}

int CiDaemon::Open()
{
    int fd, len;
    char pid_buf[16] = {0};
    if ((fd = open(cPIDFile, O_RDONLY)) < 0)
    {
        syslog(LOG_LOCAL0|LOG_ERR, "\n\nLock file not found. May be the server is not running?\n\n");
        return fd;
    }
    len = read(fd, pid_buf, 16);
    pid_buf[len] = 0;
    PID = atoi(pid_buf);

    return 0;
}

void CiDaemon::Stop()
{
    if (PID != -1)
        kill(PID, SIGQUIT);
}

void CiDaemon::Restart()
{
    Reload();
}

void CiDaemon::Reload()
{
    if (PID != -1)
        kill(PID, SIGHUP);
}

int CiDaemon::Demonize()
{
    char pid_buf[16];
    int curPID;

    chdir("/");

    /* try to grab the lock file */
    int lockFD = open(cPIDFile, O_RDWR | O_CREAT | O_EXCL, 0644);
    if (lockFD == -1 && errno == EEXIST) 
	{
        /* Perhaps the lock file already exists. Try to open it */
        FILE *lfp = fopen(cPIDFile, "r");

        if (lfp == 0)
        {
            syslog(LOG_LOCAL0|LOG_ERR,"\n\nCan't get lockfile\n\n");
            return -1;
        }

        char *lfs = fgets(pid_buf, 16, lfp);

        if (lfs != 0) 
		{
            if (pid_buf[strlen(pid_buf) - 1] == '\n')
                pid_buf[strlen(pid_buf) - 1] = 0;

            unsigned long lockPID = strtoul(pid_buf, (char **) 0, 10);

            /* see if that process is running. Signal 0 in kill(2) doesn't
               send a signal, but still performs error checking */
            int killResult = kill((__pid_t)lockPID, 0);

            if (killResult == 0) 
			{
                syslog(LOG_LOCAL0|LOG_ERR, 
					   "\n\nERROR\n\nA lock file %s has been detected. It appears it is owned\nby the (active) process with PID %ld.\n\n",
                       cPIDFile, lockPID);
            } 
			else 
			{
                errcode = errno;
                if (errcode == ESRCH) /* non-existent process */
                {
                    syslog(LOG_LOCAL0|LOG_ERR,
						   "\n\nERROR\n\nA lock file %s has been detected. It appears it is owned\nby the process with PID %ld,"
						   " which is now defunct. Delete the lock file\nand try again.\n\n",
                           cPIDFile, lockPID);
                } 
				else 
				{
                    syslog(LOG_LOCAL0|LOG_ERR,"\n\nCould not acquire exclusive lock on lock file\n\n");
                }
            }
        } 
		else 
		{
            errcode = errno;
            syslog(LOG_LOCAL0|LOG_ERR,"\n\nCould not read lock file\n\n");
        }

        fclose(lfp);
        return -1;
    }
    else if (lockFD == -1 && errno != EEXIST)
    {
        errcode = errno;
        syslog(LOG_LOCAL0|LOG_ERR,"\n\nDo not exists directory for PID file\n\n");
        return -1;
    }
    /* acquired access to the lock file - set a lock on it */
    struct flock exclusiveLock;
    exclusiveLock.l_type = F_WRLCK; /* exclusive write lock */
    exclusiveLock.l_whence = SEEK_SET; /* use start and len */
    exclusiveLock.l_len = exclusiveLock.l_start = 0; /* whole file */
    exclusiveLock.l_pid = 0; 
    int lockResult = fcntl(lockFD, F_SETLK, &exclusiveLock);
    if (lockResult < 0) /* can't get a lock */
    {
        close(lockFD);
        syslog(LOG_LOCAL0|LOG_ERR, "\n\nCan't get lockfile\n\n");
        return -1;
    }

    /* move ourselves into the background and become a daemon. */
    curPID = fork();
    switch (curPID)
    {
            case 0: /* we are the child process */
                break;

            case -1: /* error - fork failing */
                syslog(LOG_LOCAL0|LOG_ERR, "Error: initial fork failed: %s\n", strerror(errno));
                errcode = errno;
                return -1;
                break;

            default: /* we are the parent, so exit */
                return -1;
                break;
    }

    /* make the process a session and process group leader. This simplifies
        job control if we are spawning child servers, and starts work on
		detaching us from a controlling TTY	*/
    if (setsid() < 0)
        return -1;

    /* ignore SIGHUP as this signal is sent when session leader terminates */
    signal(SIGHUP, SIG_IGN);

    /* fork again to let session group leader exit. Now we can't
       have a controlling TTY. */
    curPID = fork();

    switch (curPID) /* return codes as before */
    {
            case 0:
                break;

            case -1:
                errcode = errno;
                return -1;
                break;

            default:
                return -1;
                break;
    }

    /* log PID to lock file */
    /* truncate just in case file already existed */
    if (ftruncate(lockFD, 0) < 0)
        return -1;

    /* store our PID. Then we can kill the daemon with
	kill `cat <lockfile>` where <lockfile> is the path to our
	lockfile */
    char pidStr[16];
    memset(pidStr, 0, 16);
    sprintf(pidStr,"%ld\n", (long)getpid());
    if (write(lockFD, pidStr, strlen(pidStr)) == -1) 
	{
        errcode = errno;
        return -1;
    }

    PID = lockFD; /* save lock file descriptor */
    /* close open file descriptors */
    int numFiles = (int)sysconf(_SC_OPEN_MAX); /* how many file descriptors? */

    if (numFiles < 0) /* sysconf has returned an indeterminate value */
        numFiles = OPEN_MAX_GUESS;

    for (int i = numFiles-1; i >= 0; --i) /* close all open files except lock */
    {
        if (i != lockFD ) /* don't close the lock file! */
            close(i);
    }
   
    umask(0);

	/* stdin/out/err to /dev/null */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);


    /* put server into its own process group. If this process now spawns
       child processes, a signal sent to the parent will be propagated
       to the children */
    setpgrp();

    ConfigureSignalHandlers();

    return 0;
}

void CiDaemon::SetLogLevel(int _level)
{
    logLevel = _level;
    (void)setlogmask(LOG_UPTO(logLevel)); /* set logging level */
}

int CiDaemon::ConfigureSignalHandlers() /* set up signal processing */
{
    install_signal_handler(SIGQUIT);
    install_signal_handler(SIGTERM);
    install_signal_handler(SIGUSR1);
    install_signal_handler(SIGHUP);

    return 0;
}

void CiDaemon::receive_sig_function( int status )
{
    syslog(LOG_LOCAL0|LOG_INFO,"SIGNAL: %s !!!!",strsignal(status));
    switch (status)
    {
        case SIGQUIT:
            if (PID != -1)
            {
                close(PID);
                unlink(cPIDFile);
                PID = -1;
            }

            if (ServerStop != nullptr) 
			{
                syslog(LOG_LOCAL0|LOG_INFO, "STOP SERVER !!!!");
                ServerStop();
                ServerStop = nullptr;
            }

            _exit(0);
            break;
        case SIGUSR1:
            syslog(LOG_LOCAL0|LOG_INFO, "caught SIGUSR1 - soft shutdown");

            if (ServerStop != nullptr) 
			{
                syslog(LOG_LOCAL0|LOG_INFO, "STOP SERVER !!!!");
                ServerStop();
            }
            break;
        case SIGHUP:
			if (ServerReload != nullptr)
			{
				ServerReload();
			}
            break;
        default:
#ifdef _GNU_SOURCE
            syslog(LOG_LOCAL0|LOG_INFO, "caught signal: %s - exiting", strsignal(status));
#else
            syslog(LOG_LOCAL0|LOG_INFO, "caught signal: %d - exiting", status);
#endif
    }
}

#endif // not _MSC_VER