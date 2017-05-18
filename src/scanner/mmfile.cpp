#include "mmfile.h"

#ifdef _MSC_VER
#include <windows.h>
#else
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#endif

using namespace CITOOL;

MMFile::MMFile()
:	h_file(0),
#ifdef _MSC_VER
	h_mmfile(0),
#endif
	p_mmdata(nullptr),
	n_filesize(0)
{
}

MMFile::MMFile(const char* filename)
:	h_file(0),
#ifdef _MSC_VER
	h_mmfile(0),
#endif
	p_mmdata(nullptr),
	n_filesize(0)
{
	open(filename);
}

MMFile::~MMFile()
{
	close();
}

/// open file
bool MMFile::open(const char* filename)
{
	if (is_opened())
		return false;

	h_file = 0;
	n_filesize = 0;
#ifdef _MSC_VER
	h_mmfile = 0;
#endif
	p_mmdata = nullptr;

#ifdef _MSC_VER
	// open file
	h_file = ::CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (h_file == INVALID_HANDLE_VALUE)
		return false;

	// file size
	LARGE_INTEGER result;
	if (!GetFileSizeEx(h_file, &result))
		return false;

	n_filesize = static_cast<uint64_t>(result.QuadPart);

	// convert to mapped mode
	h_mmfile = ::CreateFileMapping(h_file, nullptr, PAGE_READONLY, 0, 0, nullptr);
  
	if (!h_mmfile)
		return false;

#else
	// open file
	h_file = ::open(filename, O_RDONLY | O_LARGEFILE);
	if (h_file == -1)
	{
		h_file = 0;
		return false;
	}

	// file size
	struct stat64 statInfo;
	if (fstat64(h_file, &statInfo) < 0)
		return false;

	n_filesize = statInfo.st_size;
#endif

#ifdef _MSC_VER
	// get memory address
	p_mmdata = ::MapViewOfFile(h_mmfile, FILE_MAP_READ, 0, 0, n_filesize);

	if (nullptr == p_mmdata)
	{
		return false;
	}
#else
	// mapping
	p_mmdata = ::mmap64(nullptr, n_filesize, PROT_READ, MAP_SHARED, h_file, 0);
	if (p_mmdata == MAP_FAILED)
	{
		p_mmdata = nullptr;
		return false;
	}

	::madvise(p_mmdata, n_filesize, MADV_NORMAL);
#endif

	if (!p_mmdata)
		return false;

	return true;
}

/// close file
void MMFile::close()
{
	// clean pointer
	if (p_mmdata)
	{
		// unmap data
#ifdef _MSC_VER
		::UnmapViewOfFile(p_mmdata);
#else
		::munmap(p_mmdata, n_filesize);
#endif
		p_mmdata = nullptr;
	}

#ifdef _MSC_VER
	if (h_mmfile)
	{
		::CloseHandle(h_mmfile);
		h_mmfile = 0;
	}
#endif

	// close file handle
	if (h_file)
	{
#ifdef _MSC_VER
		::CloseHandle(h_file);
#else
		::close(h_file);
#endif
		h_file = 0;
	}

	n_filesize = 0;
}

/// raw access
const uint8_t* MMFile::data() const
{
	return (const uint8_t*)p_mmdata;
}




