#include "util.h"

#include <iostream>
#include <fstream>

std::string lang::fsutil::readTextFile(const std::string& path) noexcept
{
	std::ifstream file(path);
	return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());;
}
