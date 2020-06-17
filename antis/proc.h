#pragma once
#include <process.h>

namespace anti
{
	struct s_handle_info
	{
		unsigned long pid;
		HWND		  hndl;
	};
	struct s_process_info
	{
		s_process_info(WCHAR a1[MAX_PATH], const DWORD a2, TCHAR a3[260], const char a4[MAX_PATH])
		{
			std::wstring ws(a1);
			const std::string str(ws.begin(), ws.end());
			const std::string str2(a3);
			proc_id = a2;
			exe_name = str;
			full_path = str2;
			title = a4;
		}
		std::string exe_name;
		std::string full_path;
		std::string title;
		DWORD proc_id;
	};

	class c_proc
	{
	private:
		PROCESS_INFORMATION open(const int pid);		
		HWND				findmain(unsigned long pid);
		std::vector< s_process_info> get();
		void				scan(std::vector<std::basic_string<char>>& blacklist_procname,
								 std::vector<std::basic_string<char>>& blacklist_title,
								 std::vector<s_process_info>::value_type obj, uint32_t sum) const;
	public:
		bool				ismain(const HWND hndl);
		void				work();
		bool				stealththrd(HANDLE hThread);

	};
	extern c_proc* p_proc;
}