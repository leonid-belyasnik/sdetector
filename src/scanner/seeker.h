/**
* \file		seeker.h
* \brief	Search in file malwares.
*
* \note		Use Boyer–Moore search algorithm.
* \ref		https://ru.wikipedia.org/wiki/%D0%90%D0%BB%D0%B3%D0%BE%D1%80%D0%B8%D1%82%D0%BC_%D0%91%D0%BE%D0%B9%D0%B5%D1%80%D0%B0_%E2%80%94_%D0%9C%D1%83%D1%80%D0%B0
* \author	Leonid Belyasnik (leonid.belyasnik@gmail.com)
* \date		12/05/2017
*/

#ifndef SEEKER_H
#define SEEKER_H

#pragma once

#include <string>
#include <vector>
#include "mmfile.h"

/**
*	namespace T1
*	\brief Test 1
*/
namespace T1 {
	/**
	* \brief	Dataset for search.
	*/
	struct SeekData
	{
		std::vector<int> vn_suffshift;	///< Suffshift for Boyer-Moore search algorithm.
		std::vector<uint8_t> vb_trace;	///< Desired byte array.
		std::string s_guid;	///< Malware GUID.
	};
	/**
	* \brief	Seeker class.
	*/
	class BMSeeker : public CITOOL::MMFile
	{
	public:
		/**
		* Constructor.
		*/
		BMSeeker() : CITOOL::MMFile() {}
		/**
		* Constructor.
		*
		* \param [in]	filename	Path to file.
		*/
		BMSeeker(const char* filename) : CITOOL::MMFile(filename) {}
		/**
		* Find in file.
		*
		* \param [in]	seek_set	Dataset for search.
		*
		* \return	Position in file or -1 when not found.
		*/
		int find(const SeekData& seek_set);
		/**
		* Compute suffshift for Boyer-Moore search algorithm.
		*
		* \param [in]	s	Byte array pointer. Malware signature.
		* \param [in]	m	Size of byte array.
		*
		* \return	Suffshift array for search algorithm.
		*/
		static std::vector<int> compute_suffshift(const uint8_t* s, int m);
	};

} // T1

#endif // SEEKER_H