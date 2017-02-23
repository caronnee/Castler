#include "Filename.h"
#include <Windows.h>

std::string ExtractDirectory(const std::string & path)
{
	int t = path.find_last_of('\\');
	int tt = path.find_last_of('/');
	t = max(t, tt);
	if (t <= 0)
		return path;
	return path.substr(0, t);
}
std::string GetFullPath(const std::string & relativeName)
{
	if (relativeName[1] == ':')
		return relativeName;
	// find Exe directory
	char buffer[2048];
	GetModuleFileNameA(NULL, buffer, 2048);
	char * c = strrchr(buffer, '\\') + 1;
	*c = '\0';
	std::string str(buffer);
	str += relativeName;
	return str;
}

std::string ReplaceExt(const std::string & path, const char * ext)
{
	int t = path.find_last_of('.');
	if (t == std::string::npos)
		return path + ext;
	std::string s = path.substr(0, t);
	return s + ext;
}
std::string FindTitle(const std::string & path)
{
	int t = path.find_last_of('\\');
	int tt = path.find_last_of('/');
	t = max(t, tt);
	if (t <= 0)
		return path;
	int dt = path.find_last_of('.');
	t++;
	if (dt < t)
		return path.substr(t);
	return path.substr(t, dt - t);
}
