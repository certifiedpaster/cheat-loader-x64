#include <inc.h>
void watcher()
{
	VMProtectBeginVirtualization("wtcher_vr");
	while (1)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(250));
		if (VMProtectIsDebuggerPresent(1) || !VMProtectIsProtected() || VMProtectIsVirtualMachinePresent() || IsDebuggerPresent() || !VMProtectIsValidImageCRC()) { user::p_data->log(stra("user is running a debugger!")); ExitProcess(0); }
		anti::p_opcode->work();
		anti::p_proc->work();
	}
	VMProtectEnd();
}

/*
needed 
https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads
*/

int main()
{
	SetConsoleTitleA(" ");
	print("loading");
	user::p_data = new user::c_data();
	if (!user::p_data->setup()) { ExitProcess(0); }
	game::p_game = new game::c_game();
	anti::p_proc = new anti::c_proc();
	anti::p_opcode = new anti::c_opcode();
	if (game::p_game->running()) { user::p_data->log(stra("game already running forcing exit"),1); ExitProcess(0); }
	anti::p_opcode->setup();
	anti::p_opcode->add((void*)main, 0, 1, stra("mn"));
	anti::p_opcode->add((void*)DeleteFileA, 0, 1, stra("delfilea"));
	anti::p_opcode->add((void*)IsDebuggerPresent, 0, 1, stra("isdeb"));
	anti::p_opcode->add((void*)watcher, 0, 1, stra("wathrd"));
	anti::p_opcode->add((void*)LoadLibraryA, 0, 1, stra("llib"));
	anti::p_opcode->add((void*)VirtualProtectEx, 0, 1, stra("vproex"));
	anti::p_opcode->add((void*)OpenProcess, 0, 1, stra("openpr"));
	anti::p_opcode->add((void*)ExitProcess, 0, 1, stra("expro"));
	anti::p_opcode->add((void*)CreateThread, 0, 1, stra("cthrd"));
	auto hnd = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)watcher, 0, 0, 0);
	anti::p_proc->stealththrd(hnd);
	auto client = new web::client(true, true);
	srand((int)&client + time(NULL));
	if (!client->connect(strw(L"back.something.com"))) { ExitProcess(0); }
	/*look if user has never logged in before*/
	if (user::p_data->name().empty())
	{
		auto tmp = std::string();
		print(std::string("user hardware-id: ").append(user::p_data->hwid_get().c_str()).c_str());
		std::cout << stra("--- user:"); std::cin >> tmp;
		if (tmp.empty()) ExitProcess(0); user::p_data->set_name(tmp); tmp.clear();
		std::cout << stra("--- pass:"); std::cin >> tmp;
		if (tmp.empty()) ExitProcess(0); user::p_data->set_pass(tmp); tmp.clear();
		user::p_data->save(); user::p_data->log(stra(std::string("---").c_str())); user::p_data->log(stra(std::string("new session started").c_str()));  user::p_data->log(stra(std::string("user saved details u:").append(user::p_data->name()).append(" p:").append(user::p_data->pass()).append(" h:").append(user::p_data->hwid()).append(" h:").append(user::p_data->hwid_get()).append(" v:").append(ver).c_str()));
	}
	else
	{
		user::p_data->log(stra(std::string("---").c_str()));
		user::p_data->log(stra(std::string("new session started").c_str()));
	}
	/*login and checking*/
	if (!client->request(strw(L"gate.php"), web::requestmode::GET, { { stra("user"), user::p_data->name() }, { stra("pass"), user::p_data->pass() }, { stra("hwid"), user::p_data->hwid() }, { stra("ver"), ver } })) { ExitProcess(0); }
	user::p_data->log(stra(std::string("user attempting auth u:").append(user::p_data->name()).append(" p:").append(user::p_data->pass()).append(" h:").append(user::p_data->hwid()).append(" h:").append(user::p_data->hwid_get()).append(" v:").append(ver).c_str()));
	auto res = client->tostring(client->get());
	if (std::strstr(res.c_str(), stra("wrong")))
	{
		auto err = res.find("0");
		auto nstr = res.erase(0, err);
		print(std::string("auth returned ").append(nstr).c_str());
		user::p_data->log(stra(std::string("user failed auth reason ").append(nstr).c_str()),1);
		auto a = getchar(); ExitProcess(0);
	}
	if (res.empty()) { ExitProcess(0); } user::p_data->set_encr(res); user::p_data->log(stra(std::string("user auth competed rk:").append(std::to_string(user::p_data->encr())).c_str()));

	/*requesting generation & downloading & unpacking the image itself*/
	user::p_data->log(stra("user requesting image"));
	if (!client->request(strw(L"image.php"), web::requestmode::GET, { {stra("user"),user::p_data->name()}, {stra("pass"),user::p_data->pass()}, {stra("hwid"),user::p_data->hwid()}, {stra("ver"),ver} })) { ExitProcess(0); }
	auto image = client->get();	
	user::p_data->log(stra(std::string("user recieved image with size ").append(std::to_string(image.size())).c_str()));

	std::thread f_dec_worker([&image] { auto key = user::p_data->encr(); for (auto c = 0; c < image.size(); c++) { image[c] -= 1; image[c] ^= key; } });
	f_dec_worker.join();/*wait for exec to finish*/
	user::p_data->log(stra(std::string("user unpacked image with size ").append(std::to_string(image.size())).c_str()));

	print("waiting for <game>");
	user::p_data->log(stra("waiting for game"));

	while (!game::p_game->running()) { Sleep(500); }

	print("game detected");
	user::p_data->log(stra("game found"));
	user::p_data->flag();

	/*injection*/
	if (!game::p_game->inject(image)) { user::p_data->log(stra("user failed to inject image")); ExitProcess(0); }
	
	user::p_data->log(stra("finished"));

	/*snip*/
}