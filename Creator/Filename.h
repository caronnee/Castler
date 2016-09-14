#pragma once

#include <Windows.h>
#include <string>

std::string GetFullPath(const std::string & relativeName);

std::string FindTitle(const std::string & path);