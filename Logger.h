#pragma once
#include <string>
#include <windows.h>


namespace Logger
{
	void Log(std::ostream& os, const std::string& message);
};