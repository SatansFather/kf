#include "file_utility.h"
 
#include <fstream>

TVector<char> GetFileData(const KString& path, bool binary /*= true*/)
{
	std::ios::openmode mode = std::ios::in | std::ios::ate;
	if (binary) mode |= std::ios::binary;

	std::ifstream file(path.Get(), mode);
	if (!file.is_open()) return TVector<char>();

	// get size and reset cursor
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	// copy file to buffer
	TVector<char> buffer;
	buffer.resize(size);
	K_ASSERT(file.read(buffer.data(), size), "could not read " + path);

	file.close();

	return std::move(buffer);
}
