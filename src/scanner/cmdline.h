/**
* \file		cmdline.h
* \brief	Use command line options.
*
* \author	Leonid Belyasnik (leonid.belyasnik@gmail.com)
* \date		08/05/2017
*/

#ifndef CMDLINE_H
#define CMDLINE_H

#pragma once
#include <cstdio>
#include <cstring>
#include <iostream>

namespace CmdLine {
	char* get_option(char ** begin, char ** end, const std::string & option, const std::string & s_option);
	bool option_exists(char** begin, char** end, const std::string& option, const std::string & s_option);
}

#endif // CMDLINE_H
