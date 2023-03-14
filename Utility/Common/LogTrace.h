#pragma once

#include <string>
#include <sstream>
#include <fstream>

void LogTrace(const char* function, int line)
{
	std::string sFuncton{ function };

	if (sFuncton.find("lambda_") != std::string::npos)
		sFuncton = "Lambda";
	
	std::ostringstream out;
	out << sFuncton << " > " << line;

	static std::ofstream file("zLogs.txt");
	file << out.str().c_str() << std::endl;
}

#define LOG_TRACE { LogTrace(__FUNCTION__, __LINE__); }