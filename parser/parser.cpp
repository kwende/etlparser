//Turns the DEFINE_GUID for EventTraceGuid into a const.
#define INITGUID
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>  // For inet_ntoa function
#include <windows.h>
#include <stdio.h>
#include <comdef.h>
#include <guiddef.h>
#include <wbemidl.h>
#include <wmistr.h>
#include <evntrace.h>
#include <tdh.h>
#include <iostream>
#include <ws2tcpip.h>
#include <fstream>
#include <map>
#include <string>
#include <iphlpapi.h>
#include <map>
#include <algorithm>
#include <filesystem>

std::map<int, int> counter;

void WINAPI OnEvent(PEVENT_RECORD pEventRecord)
{
	int id = pEventRecord->EventHeader.EventDescriptor.Id;
	auto result = counter.find(id); 
	if (result == counter.end())
	{
		counter.insert(std::pair<int, int>(id, 1)); 
		//std::cout << "new: " << id << ",";
	}
	else
	{
		counter[id]++;

		//std::cout << id << ":" << counter[id] << ","; 
	}
}

std::vector<std::pair<int, int>> processFile(std::string filePath)
{
	// https://github.com/piotrsmolinski/wireshark/blob/7fdaac735a6a4fb08c9e60ab27d67ad126a2dab1/extcap/etl.c#L287

	EVENT_TRACE_LOGFILEA traceSession;
	::ZeroMemory(&traceSession, sizeof(traceSession));

	// The following defines the session......
	// 
	// this thing has the ability to read from ETL files as well as trace real time. We want real time. 
	traceSession.ProcessTraceMode = PROCESS_TRACE_MODE_EVENT_RECORD;
	// this is either the session name, or the path of the log file we're reading. 
	traceSession.LogFileName = (LPSTR)filePath.c_str();
	// this is the callback. 
	traceSession.EventRecordCallback = (PEVENT_RECORD_CALLBACK)OnEvent;

	auto traceHandle = OpenTraceA(&traceSession);

	if (traceHandle != INVALID_PROCESSTRACE_HANDLE)
	{
		counter.clear(); 
		auto err = ProcessTrace(&traceHandle, 1, 0, 0);
		if (err == ERROR_SUCCESS)
		{
			std::vector<std::pair<int, int>> pairs(counter.begin(), counter.end());

			std::sort(pairs.begin(), pairs.end(), [](const auto& a, const auto& b)
				{
					return a.second > b.second;
				});

			return pairs; 
		}
	}
	else
	{
		HRESULT hr = GetLastError();
		printf("%d\n", hr); 
	}

	return std::vector<std::pair<int, int>>(); 
}

std::map<std::string, std::vector<std::pair<int, int>>> ProcessDirectory(std::string directory)
{
	std::map<std::string, std::vector<std::pair<int, int>>> result;

	for (auto file : std::filesystem::directory_iterator(directory))
	{
		auto filePath = file.path().string(); 
		auto fileResults = processFile(filePath);

		result.insert(std::pair<std::string, std::vector<std::pair<int, int>>>(filePath, fileResults));
	}

	return result;
}

void ProcessResults(std::map<std::string, std::vector<std::pair<int, int>>> results, std::string outputDir)
{
	if (!outputDir.ends_with('\\'))
	{
		outputDir += "\\"; 
	}

	for (auto result : results)
	{
		std::string fileName = result.first; 
		fileName = fileName.substr(fileName.find_last_of('\\') + 1); 

		std::ofstream fout(outputDir + fileName + ".csv");

		for (auto row : result.second)
		{
			fout << row.first << "," << row.second << std::endl; 
		}

		fout.flush(); 
		fout.close(); 
	}
}


int main()
{
	std::string badPath = "C:\\Users\\ben\\Desktop\\ETW work\\311-bad-gpu\\writeDir"; 
	std::string goodPath = "C:\\Users\\ben\\Desktop\\ETW work\\537-GPU-Good\\writeDir"; 

	auto badDirResults = ProcessDirectory(badPath); 
	auto goodDirResults = ProcessDirectory(goodPath);

	ProcessResults(badDirResults, "C:\\Users\\ben\\Desktop\\ETW work\\311-bad-gpu\\processed"); 
	ProcessResults(goodDirResults, "C:\\Users\\ben\\Desktop\\ETW work\\537-GPU-Good\\processed");
}
