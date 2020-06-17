#include <inc.h>
auto anti::c_opcode::setup() -> bool
{
	return true;
}
/**
 * \brief adds memory region to be guarded
 * \param region region that needs to be protected
 * \param region_size if (region==0) (automatic function size) else (custom region size)
 * \param critical when true will force the process to close
 * \param name debug pseudo name
 */
auto anti::c_opcode::add(void* region, int region_size, bool critical, std::string name) -> bool
{
	/*getting the actual function if we in the rttable*/
	auto gb = [&](int add) -> const uint8_t
	{
		const auto ptr = (uint8_t*)region + add;
		const auto opcode = ptr;
		const auto opc = *opcode;
		return opc;
	};

	VMProtectBeginUltra("opca");

	unsigned long long res = 0;
	auto first_bt = gb(0x0);

	if (first_bt == 0xe9)
	{
		auto last_read = 0;
		auto mempad = 0;
		std::stringstream adr_content;
		while (last_read != 0xe9)
		{
			mempad++;
			last_read = gb(mempad);

			if (last_read != 0xe9) adr_content << std::hex << last_read;
		}
		/*convert*/
		auto final_adr = (DWORD64)region + 0x5;

		std::stringstream reorg;
		//pls just do it in another way I just did this because REEEEEE
		reorg << adr_content.str().at(2);
		reorg << adr_content.str().at(3);
		reorg << adr_content.str().at(0);
		reorg << adr_content.str().at(1);

		auto relastoi = std::stoul(reorg.str(), nullptr, 16);
		auto funcadr = final_adr + relastoi;
		res = funcadr;		
	}
	else res = (unsigned long long)region;
	this->guard_sections.emplace_back((void*)res, region_size, critical, name);

	//std::cout << stra("([]) guarding function 0x") << std::hex << res << std::endl;

	VMProtectEnd();

	return true;
}

auto anti::c_opcode::log(const std::string str) -> void
{
	this->logs.push_back(str);
}

auto anti::c_opcode::get_opcode(std::vector<anti::s_opcode>::value_type& e) -> std::vector<int>
{
	auto cur_opcode = std::vector<int>();
	for (auto c = 0; c < e.size; c++)
	{
		const auto ptr = (uint8_t*)e.region + c;
		const auto opcode = ptr;
		const auto disasm_opcode = *opcode;
		cur_opcode.push_back(disasm_opcode);
	}
	return cur_opcode;
}

void anti::c_opcode::work()
{
	VMProtectBeginUltra("opcw");
	if (this->guard_sections.empty()) return;
	for (auto e : this->guard_sections)
	{
		auto cur = get_opcode(e);
		for (auto f = 0; f < e.size; f++)
		{
			const auto cur_ = cur.at(f);
			const auto ori_ = e.original.at(f);
			if (cur_ != ori_)
			{
				const auto ptr = (uint8_t*)e.region + f;
				if (e.critical)
				{
					user::p_data->log(stra(std::string("integrity was damaged in critical area: ").append(e.name).c_str()), 1);
					ExitProcess(0);
				}
				user::p_data->log(stra(std::string("integrity was damaged in non-critical area: ").append(e.name).c_str()), 1);
				WriteProcessMemory(GetCurrentProcess(), (void*)ptr, &ori_, 1, nullptr);
			}
		}
	}
	VMProtectEnd();
}

anti::c_opcode* anti::p_opcode;