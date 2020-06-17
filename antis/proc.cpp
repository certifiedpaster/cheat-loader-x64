#include <inc.h>
PROCESS_INFORMATION anti::c_proc::open(const int pid)
{
	auto info = PROCESS_INFORMATION();
	if (!pid) { return {}; }
	info.hProcess = OpenProcess(PROCESS_ALL_ACCESS | PROCESS_VM_READ, 0, pid);
	if (info.hProcess) return info;
}
bool anti::c_proc::ismain(const HWND hndl)
{
	return GetWindow(hndl, GW_OWNER) == HWND(nullptr) && IsWindowVisible(hndl);
}
BOOL CALLBACK enum_window(HWND hndl, LPARAM param)
{
	auto& data = *(anti::s_handle_info*)param;
	unsigned long pid;
	GetWindowThreadProcessId(hndl, &pid);
	if (data.pid != pid || !anti::p_proc->ismain(hndl)) return 1;
	data.hndl = hndl;
	return 0;
}
HWND anti::c_proc::findmain(unsigned long pid)
{
	anti::s_handle_info info;
	info.pid = pid; info.hndl = nullptr;
	EnumWindows(enum_window, (LPARAM)&info);
	return info.hndl;
}
std::vector<anti::s_process_info> anti::c_proc::get()
{
	auto list = std::vector<s_process_info>();
	const auto handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	auto pentry32 = PROCESSENTRY32W();
	if (handle == INVALID_HANDLE_VALUE) return {};
	pentry32.dwSize = sizeof(PROCESSENTRY32W);
	if (Process32FirstW(handle, &pentry32))
	{
		TCHAR filename[MAX_PATH];
		auto pehandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pentry32.th32ProcessID);
		GetModuleFileNameEx(pehandle, nullptr, filename, MAX_PATH);
		CloseHandle(pehandle);

		auto wnd = this->findmain(pentry32.th32ProcessID);
		char wnd_title[MAX_PATH];
		if (wnd) GetWindowTextA(wnd, wnd_title, sizeof(wnd_title));

		list.emplace_back(pentry32.szExeFile, pentry32.th32ProcessID, filename, wnd_title);
		while (Process32NextW(handle, &pentry32))
		{
			auto pehandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pentry32.th32ProcessID);
			GetModuleFileNameEx(pehandle, nullptr, filename, MAX_PATH);
			CloseHandle(pehandle);

			wnd = this->findmain(pentry32.th32ProcessID);
			if (wnd) GetWindowTextA(wnd, wnd_title, sizeof(wnd_title));

			list.emplace_back(pentry32.szExeFile, pentry32.th32ProcessID, filename, wnd_title);
		}
	}
	return list;
}
void anti::c_proc::scan(std::vector<std::basic_string<char>>& blacklist_procname,
	std::vector<std::basic_string<char>>& blacklist_title,
	std::vector<s_process_info>::value_type obj,
	uint32_t sum) const
{
	for (auto bobj : blacklist_procname)
	{
		if (strstr(obj.exe_name.c_str(), bobj.c_str()))
		{
			user::p_data->log(stra(std::string("forbidden tool open ").append(obj.full_path).append(" | ").append(obj.title).c_str()), 1);
			ExitProcess(0);
		}
	}
	for (auto bobj : blacklist_title)
	{
		if (strstr(obj.title.c_str(), bobj.c_str()))
		{
			user::p_data->log(stra(std::string("forbidden tool open ").append(obj.full_path).append(" | ").append(obj.title).c_str()), 1);
			ExitProcess(0);
		}
	}
}
void anti::c_proc::work()
{
	VMProtectBeginMutation("proc");
	auto procs = this->get(); if (procs.empty()) { user::p_data->log(stra("proc list is empty - possibly hooked?"), 1); ExitProcess(0); }
	static auto blacklist_procname = std::vector<std::string>() = { VMProtectDecryptStringA("CrySearch") , VMProtectDecryptStringA("x64dbg") , VMProtectDecryptStringA("pe-sieve"), VMProtectDecryptStringA("PowerTool"), VMProtectDecryptStringA("windbg"), VMProtectDecryptStringA("DebugView"), stra("Process Hacker") };
	static auto blacklist_title = std::vector<std::string>() = { VMProtectDecryptStringA("Cheat Engine") , VMProtectDecryptStringA("Cheat - Engine") , VMProtectDecryptStringA("CrySearch") , VMProtectDecryptStringA("x64dbg") , VMProtectDecryptStringA("ollydbg") , VMProtectDecryptStringA("PE Tools"), VMProtectDecryptStringA("PowerTool"), VMProtectDecryptStringA("DbgView"), stra("Dbgview"),  stra("\"\DESKTOP"), stra("(local)") };
	for (auto obj : procs) this->scan(blacklist_procname, blacklist_title, obj, 0);	
	VMProtectEnd();
}
bool anti::c_proc::stealththrd(HANDLE hThread)
{
	typedef NTSTATUS(NTAPI* pNtSetInformationThread)(HANDLE, UINT, PVOID, ULONG);
	NTSTATUS Status;

	pNtSetInformationThread NtSIT = (pNtSetInformationThread)GetProcAddress(GetModuleHandle(TEXT(stra("ntdll.dll"))),stra("NtSetInformationThread"));

	if (NtSIT == NULL) return false;
	if (hThread == NULL) Status = NtSIT(GetCurrentThread(),0x11,0, 0);
	else Status = NtSIT(hThread, 0x11, 0, 0);
	if (Status != 0x00000000) return false;
	else return true;
}
anti::c_proc* anti::p_proc;