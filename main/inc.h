#pragma once
#define _CRT_SECURE_NO_WARNINGS
/*sdl-sdk*/
#include <Windows.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <array>
#include <filesystem>
#include <winhttp.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <Shlwapi.h>
#include <Winternl.h>
#include <comdef.h>
#include <functional>
#include <WbemCli.h>
#include <Wbemidl.h>
#include <ctime>  
/*lib*/
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "ntdll.lib")
#pragma comment(lib, "Winhttp.lib")
/*security*/
#include "..\\vmp\VMProtectSDK.h"
/*macro*/
#include <macro.h>
/*self*/
//#include "..\\service\Service.h"		/*service management*/
#include "..\\helper\web.h"				/*connection and download management*/
#include "..\\userdata\data.h"			/*container and gatherer for user releated data - hwid-login*/
//#include "..\\driver\exploit.h"			/*exploitation resources*/
//#include "..\\driver\loader.h"			/*functionality to load the driver and unload stealthy*/
#include "..\\game\game.h"				/*injection and scanning for the game*/
#include "..\\antis\proc.h"				/*proactively scans through procs to find disallowed tools*/
#include "..\\antis\proactive_func.h"	/*proactively scan self for shady stuff and interactions*/