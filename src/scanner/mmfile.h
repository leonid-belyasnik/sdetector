/**
* \file		mmfile.h
* \brief	Implement concept of Mapped file to memory.
*
* \note		Does not support too large files ...
*
* \author	Leonid Belyasnik (leonid.belyasnik@gmail.com)
* \date		12/05/2017
*/

#ifndef MMFILE_H
#define MMFILE_H

#pragma once

#include <stdint.h>

namespace CITOOL {

#ifdef _MSC_VER
  typedef void* HandleType;
#else
  typedef int   HandleType;
#endif
	
	class MMFile
	{
		HandleType h_file;		///< file handle
#ifdef _MSC_VER
		void* h_mmfile;			///< handle to memory mapping
#endif
		void* p_mmdata;			///< pointer to the file contents mapped into memory					
		uint64_t n_filesize;	///< file size	

	public:
		MMFile();
		MMFile(const char* filename);
		virtual ~MMFile();

		/// open file
		bool open(const char* filename);

		/// close file
		void close();

		/// raw access
		const uint8_t* data() const;

		/// true, if file successfully opened
		inline bool is_opened() const { return (nullptr != p_mmdata); }

		/// get file size
		inline uint64_t size() const { return n_filesize; }

	private:
		/// non-copiable object
		MMFile(const MMFile&) = delete;
		MMFile& operator=(const MMFile&) = delete;
	};

} // CITOOL

#endif // MMFILE_H