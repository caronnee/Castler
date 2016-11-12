#include "Filename.h"
#include <Windows.h>

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

std::string FindTitle(const std::string & path)
{
	int t = path.find_last_of('\\');
	int tt = path.find_last_of('/');
	t = max(t, tt);
	if (t <= 0)
		return path;
	int dt = path.find_last_of('.');
	if (dt < t)
		return path.substr(t);
	return path.substr(t, dt - t - 1);
}
