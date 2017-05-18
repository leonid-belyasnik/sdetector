/**
* \file		log.h
* \brief	Use write to log.
*
* \author	Leonid Belyasnik (leonid.belyasnik@gmail.com)
* \date		08/05/2017
*/

#ifndef LOG_H
#define LOG_H

#pragma once
#include <string>
#include <ctime>
#include <locale>
#include <vector>
#include <sstream>

#include "logto.h"

template<typename... Args>
std::string string_format(const char*  format, Args... args)
{
	size_t size = 1 + std::snprintf(nullptr, 0, format, args...); 
	std::unique_ptr<char[]> buf(new char[size]);
	std::snprintf(buf.get(), size, format, args...);
	return std::string(buf.get(), buf.get() + size - 1); 
}

#define DEBUG(logger, msg)													\
logger->debug(                                                              \
    static_cast<std::ostringstream&>(                                       \
        std::ostringstream().flush() << msg									\
    ).str()                                                                 \
);

#define INFO(logger, msg)													\
logger->info(                                                               \
    static_cast<std::ostringstream&>(                                       \
        std::ostringstream().flush() << msg									\
    ).str()                                                                 \
);

#define WARNING(logger, msg)												\
logger->warning(                                                            \
    static_cast<std::ostringstream&>(                                       \
        std::ostringstream().flush() << msg			                        \
    ).str()                                                                 \
);

#define ERR(logger, msg)													\
logger->error(                                                              \
    static_cast<std::ostringstream&>(                                       \
        std::ostringstream().flush() << msg << " ["                         \
            << __FILE__ << ":" << __LINE__ << ']'                           \
    ).str()                                                                 \
);

#ifdef _MSC_VER
#include <conio.h>
#define stricmp	_stricmp
#else
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif

namespace CITOOL {

    inline std::string current_date_time(std::string format) 
	{
		struct tm tm_snapshot;
		std::time_t time = std::time(NULL);
#ifdef _MSC_VER
		localtime_s(&tm_snapshot, &time);
#else
		localtime_r(&time, &tm_snapshot);
#endif  
        char buf[100];
        int len = (int)std::strftime(buf, 100, format.c_str(), &tm_snapshot);
        std::string stime(buf, len);
        return stime;
    }

    inline int levelValue(const std::string label)
    {
        if (label.empty())
            return DEBUG_;

        if (stricmp(label.c_str(), "DEBUG") == 0)
            return DEBUG_;

        if (stricmp(label.c_str(), "INFO") == 0)
            return INFO_;

        if (stricmp(label.c_str(), "WARNING") == 0)
            return WARNING_;

        if (stricmp(label.c_str(), "ERROR") == 0)
            return ERROR_;

        if (stricmp(label.c_str(), "FATAL") == 0)
            return FATAL_;

        return DEBUG_;
    }

    class Log 
	{
        protected:
            std::vector<LogTo*> loggers;
            std::string log_format;
            int log_level;
        public:
			Log() : loggers(), log_format("%Y-%m-%d %X"), log_level(INFO_) {}
            ~Log()
			{
                for (unsigned int i = 0; i < loggers.size(); i++) 
				{
                    delete loggers[i];
                }
            }

            size_t Count()
			{
                return loggers.size();
            }

            void Clear()
            {
                for (unsigned int i = 0; i < loggers.size(); i++) 
				{
                    delete loggers[i];
                }
            }

            void addLogger(LogTo* logger) 
			{
                loggers.push_back(logger);
            }

            void setLogLevel(int level) 
			{
                switch (level)
				{
                case DEBUG_:
                case INFO_:
                case WARNING_:
                case ERROR_:
                case FATAL_:
                    log_level = level;
                    break;
                default:
                    throw std::invalid_argument("Invalid level");
                }
            }

            void setLogFormat(std::string format) 
			{
                log_format = format;
            }

            bool isInformative(int lvl) 
			{
                return lvl == DEBUG_ || lvl == INFO_;
            }

            std::string levelLabel(int lvl) 
			{
                switch (lvl) {
                case DEBUG_:
                    return "DEBUG";
                case INFO_:
                    return "INFO";
                case WARNING_:
                    return "WARNING";
                case ERROR_:
                    return "ERROR";
                case FATAL_:
                    return "FATAL";
                }

                throw std::invalid_argument("Invalid level");
            }

            std::string formatMessage(int level, std::string msg) 
			{
                std::stringstream ss;

                ss << current_date_time(log_format) << ' ';
                ss << '[' << levelLabel(level) << ']' << ' ' << msg;

                return ss.str();
            }

            void operator()(std::string const& msg, const char* func, const char* file, int line) 
			{
                std::string fmessage = msg;
				fmessage.append(" in function: ").append(func).append(" ").append(file).append(":" + line);

                log(INFO_, fmessage);
            }

            void operator()(int level, std::string const& msg) 
			{
                log(level, msg);
            }

            void operator()(const char* msg)
			{
                log(msg);
            }

            void operator()(std::string const& msg)
			{
                log(msg);
            }

            void log(int level, std::string msg) 
			{
                for (unsigned int i = 0; i < loggers.size(); i++) 
				{
                    loggers[i]->write(level, formatMessage(level, msg));
                }
            }

            void log(std::string msg)
			{
                log(INFO_, msg);
            }

            void log(const char* msg)
			{
                log(INFO_, std::string(msg));
            }

            void debug(const char* msg)
			{
                log(DEBUG_, msg);
            }

            void debug(std::string msg)
			{
                log(DEBUG_, msg);
            }

            void info(const char* msg)
			{
                log(INFO_, msg);
            }

            void info(std::string m) 
			{
                log(INFO_, m);
            }

            void warning(const char* msg)
			{
                log(WARNING_, msg);
            }

            void warning(std::string msg)
			{
                log(WARNING_, msg);
            }

            void error(const char* msg)
			{
                log(ERROR_, msg);
            }

            void error(std::string msg)
			{
                log(ERROR_, msg);
            }

            void fatal(const char* msg)
			{
                log(FATAL_, msg);
            }

            void fatal(std::string msg)
			{
                log(FATAL_, msg);
            }
    };

} // CITOOL

#endif // LOG_H
