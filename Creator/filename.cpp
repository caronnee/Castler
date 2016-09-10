#include "Filename.h"

std::string GetFullPath(const std::string & relativeName)
{
	// find Exe directory
	char buffer[2048];
	GetModuleFileNameA(NULL, buffer, 2048);
	char * c = strrchr(buffer, '\\') + 1;
	*c = '\0';
	std::string str(buffer);
	str += relativeName;
	return str;
}