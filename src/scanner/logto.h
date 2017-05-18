/**
* \file		logto.h
* \brief	Log's direction.
*
* \author	Leonid Belyasnik (leonid.belyasnik@gmail.com)
* \date		08/05/2017
*/

#ifndef LOGTO_H
#define LOGTO_H

#pragma once
#include <mutex>
#include <iostream>
#include <fstream>
#include <ios>
#include <string>

namespace CITOOL {

	enum LogLevel 
	{
		DEBUG_ = 0,
		INFO_,
		WARNING_,
		ERROR_,
		FATAL_
	};

    class LogTo 
	{
        protected:
            int threshold;
            std::mutex mutex;
        public:
			LogTo(int th) : threshold(th), mutex() {}
			LogTo() : LogTo(INFO_) {}
            virtual ~LogTo() {}
            virtual void write(int level, std::string msg) = 0;
    };

    class ToConsole : public LogTo
	{
        public:
			ToConsole(int threshold) : LogTo(threshold) {}
            ~ToConsole() {}

            void write(int level, std::string msg) 
			{
                if (threshold > level) 
				{
                    return;
                }

                std::unique_lock<std::mutex> lk(mutex);
                std::cout << msg << std::endl;
            }
    };

    class ToFile : public LogTo
	{
        protected:
            std::ofstream ofs;
        public:
			ToFile(int threshold, std::string filename) : LogTo(threshold), ofs()
			{
                ofs.open(filename, std::ofstream::out | std::ofstream::app);
            }

            ~ToFile()
			{
                ofs.close();
            }

            void write(int level, std::string msg)
			{
                if (level < threshold)
				{
                    return;
                }

                std::unique_lock<std::mutex> lk(mutex);
                ofs << msg << std::endl;
            }
    };

} // CITOOL

#endif // LOGTO_H
