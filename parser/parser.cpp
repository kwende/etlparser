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
#include <vector>
#include <algorithm>

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

int main()
{
	// https://github.com/piotrsmolinski/wireshark/blob/7fdaac735a6a4fb08c9e60ab27d67ad126a2dab1/extcap/etl.c#L287

	EVENT_TRACE_LOGFILEA traceSession;
	::ZeroMemory(&traceSession, sizeof(traceSession));

	// The following defines the session......
	// 
	// this thing has the ability to read from ETL files as well as trace real time. We want real time. 
	traceSession.ProcessTraceMode = PROCESS_TRACE_MODE_EVENT_RECORD;
	// this is either the session name, or the path of the log file we're reading. 
	traceSession.LogFileName = (LPSTR)"C:\\Users\\ben\\Desktop\\recorings\\bad\\Intel-GFX-Info_000001.etl";
	// this is the callback. 
	traceSession.EventRecordCallback = (PEVENT_RECORD_CALLBACK)OnEvent;

	// now we're going to initialize structures to start the tracing session. 
	//
	// we're going to heap alloc this, so lets get the size. should be the size of the structure + my name. 
	// the extra char at the end allows for null termination. 
	ULONG tracePropertiesBufferSize = sizeof(EVENT_TRACE_PROPERTIES) + sizeof(traceSession.LogFileName) + sizeof(CHAR);
	// heap alloc on local heap. 
	PEVENT_TRACE_PROPERTIES ptrEventTraceProps = (PEVENT_TRACE_PROPERTIES)::LocalAlloc(LPTR, tracePropertiesBufferSize);
	if (ptrEventTraceProps)
	{
		::ZeroMemory(ptrEventTraceProps, tracePropertiesBufferSize);

		// WNODE information. 
		// https://learn.microsoft.com/en-us/windows/win32/etw/wnode-header
		ptrEventTraceProps->Wnode.BufferSize = tracePropertiesBufferSize;
		ptrEventTraceProps->Wnode.ClientContext = 2;
		ptrEventTraceProps->Wnode.Flags = WNODE_FLAG_TRACED_GUID;

		// EVENT_TRACE_PROPERTIES information. 
		// https://learn.microsoft.com/en-us/windows/win32/api/evntrace/ns-evntrace-event_trace_properties
		// https://learn.microsoft.com/en-us/windows/win32/etw/logging-mode-constants
		ptrEventTraceProps->LogFileMode = EVENT_TRACE_REAL_TIME_MODE;
		ptrEventTraceProps->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);

		// start the tracing session. 
		//TRACEHANDLE startTraceHandle;
		//auto response = ::StartTraceA(&startTraceHandle, traceSession.LogFileName, ptrEventTraceProps);
		//if (response == ERROR_SUCCESS)
		//{
		//	::StopTraceA(startTraceHandle, traceSession.LogFileName, ptrEventTraceProps);
		//}
		auto traceHandle = OpenTraceA(&traceSession); 
		if (traceHandle != INVALID_PROCESSTRACE_HANDLE)
		{
			auto err = ProcessTrace(&traceHandle, 1, 0, 0); 
			if (err == ERROR_SUCCESS)
			{
				std::vector<std::pair<int,int>> pairs(counter.begin(), counter.end()); 

				std::sort(pairs.begin(), pairs.end(), [](const auto& a, const auto& b)
					{
						return a.second > b.second; 
					}); 

				for (auto it : pairs)
				{
					std::cout << it.first << ":" << it.second << std::endl;
				}

				getchar(); 
			}
		}
	}
}
