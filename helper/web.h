#pragma once
#include <inc.h>
namespace web
{
	typedef enum 
	{
		GET,
		POST
	} requestmode;
	class client
	{
		int size;
		int stage;
		bool      usehttps;
		bool      insecure;
		HINTERNET hsession = nullptr;
		HINTERNET hconnect = nullptr;
		HINTERNET hrequest = nullptr;
	private:
		std::vector<byte> response;
	public:
		bool               connect(std::wstring page);
		bool               request(std::wstring file, web::requestmode mode, std::unordered_map<std::string, std::string> vars);
		std::vector<byte>  get();
		void			   set_max(int s, int st);
		static std::string tostring(std::vector<byte> in);
	public:
		client(bool usehttps = true, bool insecure = false);
		~client();
	};
}
