#pragma once

#include <string>

std::string GetFullPath(const std::string & relativeName);

std::string ReplaceExt(const std::string & path, const char * ext);
std::string FindTitle(const std::string & path);