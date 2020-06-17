#pragma once
#include <inc.h>

namespace user
{
	struct s_user
	{
		std::string name;
		std::string pass;
		std::string hwid;
		int         enc_key;
	};
	class c_data
	{
	private:
		std::string hwid_clear;
		s_user		usr;
		std::string regkey(const std::string& loc, const std::string& key);
		HKEY        openkey(HKEY loc, LPCSTR key);
	public:
		bool		setup();
		std::string hwid_get();
		std::string name() { return this->usr.name; }
		std::string pass() { return this->usr.pass; }
		std::string hwid() { return this->usr.hwid; }
		int			encr() { return this->usr.enc_key; }
		void set_name(std::string in) { this->usr.name = in; }
		void set_pass(std::string in) { this->usr.pass = in; }
		void set_hwid(std::string in) { this->usr.hwid = in; }
		void set_encr(std::string in) { this->usr.enc_key = std::stoi(in); };
		void save();
		void log(std::string txt, bool error=false);
		std::string rstr(int size);
		void flag();
	};
	extern c_data* p_data;
}