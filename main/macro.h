#pragma once
#include <inc.h>
/*vmp*/
#define stra(x) VMProtectDecryptStringA(x)
#define strw(x) VMProtectDecryptStringW(x)
#define ver std::string(stra("2.7"))
#define print(x, ...) std::cout << stra("([]) ") << stra(x) << std::endl;