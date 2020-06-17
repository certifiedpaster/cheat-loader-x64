#pragma once
#include <inc.h>

namespace game
{
	typedef struct _MANUAL_INJECT
	{
		LPVOID  ImageBase;
		PIMAGE_NT_HEADERS64 NtHeaders;
		PIMAGE_BASE_RELOCATION BaseRelocation;
		PIMAGE_IMPORT_DESCRIPTOR ImportDirectory;
		decltype(LoadLibraryA)* fnLoadLibraryA;
		decltype(GetProcAddress)* fnGetProcAddress;
	} MANUAL_INJECT, * PMANUAL_INJECT;
	typedef BOOL(__stdcall* dllmain)(HMODULE, DWORD, LPVOID);
	static DWORD __stdcall LibraryLoader(LPVOID Memory)
	{
		auto mject = (MANUAL_INJECT*)Memory;
		auto image_brealoc = mject->BaseRelocation;
		auto delta = (ULONGLONG)((ULONGLONG)mject->ImageBase - mject->NtHeaders->OptionalHeader.ImageBase);

		while (image_brealoc->VirtualAddress)
		{
			if (image_brealoc->SizeOfBlock >= sizeof(IMAGE_BASE_RELOCATION))
			{
				int count = (image_brealoc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
				auto list = (PWORD)(image_brealoc + 1);
				for (int i = 0; i < count; i++)
				{
					if (list[i])
					{
						auto ptr = (PULONGLONG)((ULONGLONG)mject->ImageBase + (image_brealoc->VirtualAddress + (list[i] & 0xFFF)));
						*ptr += delta;
					}
				}
			}
			image_brealoc = (PIMAGE_BASE_RELOCATION)((ULONGLONG)image_brealoc + image_brealoc->SizeOfBlock);
		}

		auto proc_importdir = mject->ImportDirectory;
		while (proc_importdir->Characteristics)
		{
			auto orig_first_thunk = (PIMAGE_THUNK_DATA64)((ULONGLONG)mject->ImageBase + proc_importdir->OriginalFirstThunk);
			auto first_thunk = (PIMAGE_THUNK_DATA64)((ULONGLONG)mject->ImageBase + proc_importdir->FirstThunk);

			auto hModule = mject->fnLoadLibraryA((LPCSTR)mject->ImageBase + proc_importdir->Name);

			if (!hModule)
				return FALSE;

			while (orig_first_thunk->u1.AddressOfData)
			{
				if (orig_first_thunk->u1.Ordinal & IMAGE_ORDINAL_FLAG)
				{
					auto Function = (ULONGLONG)mject->fnGetProcAddress(hModule,
						(LPCSTR)(orig_first_thunk->u1.Ordinal & 0xFFFF));

					if (!Function)
						return FALSE;

					first_thunk->u1.Function = Function;
				}
				else
				{
					auto import_by_name = (PIMAGE_IMPORT_BY_NAME)((ULONGLONG)mject->ImageBase + orig_first_thunk->u1.AddressOfData);
					auto Function = (ULONGLONG)mject->fnGetProcAddress(hModule, (LPCSTR)import_by_name->Name);

					if (!Function) return FALSE;

					first_thunk->u1.Function = Function;
				}
				orig_first_thunk++;
				first_thunk++;
			}
			proc_importdir++;
		}

		if (mject->NtHeaders->OptionalHeader.AddressOfEntryPoint != 0)
		{
			auto EntryPoint = (dllmain)((ULONGLONG)mject->ImageBase + mject->NtHeaders->OptionalHeader.AddressOfEntryPoint);
			return EntryPoint((HMODULE)mject->ImageBase, DLL_PROCESS_ATTACH, NULL);
		}

		return TRUE;
	}
	static DWORD __stdcall stub()
	{
		return 0;
	}
	class c_game
	{
	private:
		DWORD bdo_pid;
		DWORD pid(std::string exe);
	public:
		bool running();
		bool inject(std::vector<uint8_t> file);
	};
	extern c_game* p_game;
}