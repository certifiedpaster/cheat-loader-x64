#pragma once
#include <inc.h>

namespace anti
{
	struct s_opcode
	{
		s_opcode(void* a1, int a2, bool a3, std::string a4)
		{
			region = a1;
			size = a2;
			critical = a3;
			name = a4;
			prep();
		}
		/*given*/
		std::string name;
		bool critical;
		int size;
		void* region;
		/*scan*/
		std::vector<int> original; /*original sequence of bytes*/
		/*helper*/
		void prep()
		{
			if (size != 0) /*set region*/
			{
				for (auto c = 0; c < size; c++)
				{
					const auto ptr = (uint8_t*)region + c;
					const auto opcode = ptr;
					const auto disasm_opcode = *opcode;
					original.push_back(disasm_opcode);
				}
			}
			else /*scan until we hit function end*/
			{
				auto last_result = int();
				auto cnt = 0;
				while (last_result != 0xc3)/*0xc3 ret*/
				{
					const auto ptr = (uint8_t*)region + cnt;
					const auto opcode = ptr;
					const auto opc = *opcode;
					original.push_back(opc);
					cnt++;
					last_result = opc;
				}
				size = cnt - 1;
				original.pop_back();
				/*delete 0xc3*/
			}
		}
	};
	class c_opcode
	{
	public:
		auto setup() -> bool;
		auto add(void* region, int region_size, bool critical, std::string name) -> bool;
		auto log(std::string str) -> void;
		static auto get_opcode(std::vector<s_opcode>::value_type& e)->std::vector<int>;
		void work();
	private:
		std::vector<s_opcode> guard_sections;
		std::vector<std::string> logs;
	};
	extern c_opcode* p_opcode;
}