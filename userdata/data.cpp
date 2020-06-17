#include <inc.h>
std::string user::c_data::regkey(const std::string& loc, const std::string& skey)
{
	HKEY key; TCHAR value[1024]; DWORD bufLen = 1024 * sizeof(TCHAR); long ret;
	ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, loc.c_str(), 0, KEY_QUERY_VALUE, &key);
	if (ret != ERROR_SUCCESS) return std::string();
	ret = RegQueryValueEx(key, skey.c_str(), 0, 0, (LPBYTE)value, &bufLen);
	RegCloseKey(key);
	if ((ret != ERROR_SUCCESS) || (bufLen > 1024 * sizeof(TCHAR))) return std::string();
	auto stringValue = std::string(value, (size_t)bufLen - 1);
	size_t i = stringValue.length();
	while (i > 0 && stringValue[i - 1] == '\0') --i;
	return stringValue.substr(0, i);
}
HKEY user::c_data::openkey(HKEY loc, LPCSTR key)
{
	HKEY hKey;
	LONG nError = RegOpenKeyEx(loc, key, NULL, KEY_ALL_ACCESS, &hKey);
	if (nError == ERROR_FILE_NOT_FOUND) nError = RegCreateKeyEx(loc, key, NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
	return hKey;
}
bool user::c_data::setup()
{
	VMProtectBegin("sup");
	auto tmp = std::string();
	tmp = this->regkey(stra("SOFTWARE\\something"), stra("user"));
	this->usr.name = tmp; tmp.clear();
	tmp = this->regkey(stra("SOFTWARE\\something"), stra("pass"));
	this->usr.pass = tmp; tmp.clear();
	auto key = this->openkey(HKEY_LOCAL_MACHINE, stra("SOFTWARE\\something"));
	RegSetValueExA(key, stra("ver"), 0, REG_SZ, (LPBYTE)ver.c_str(), ver.size()); RegCloseKey(key);
	this->hwid_clear = this->hwid_get(); /*save on setup check for mutation later*/
	for (auto c = 0; c < this->hwid_clear.size(); c++) this->hwid_clear[c] ^= 3;
	this->usr.hwid = this->hwid_clear;
	return 1;
	VMProtectEnd();
}
std::string user::c_data::hwid_get()
{
	VMProtectBegin("hwd");
	auto h = CreateFileW(strw(L"\\\\.\\PhysicalDrive0"), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (h == INVALID_HANDLE_VALUE) return {};
	std::unique_ptr<std::remove_pointer<HANDLE>::type, void(*)(HANDLE)> hDevice{ h, [](HANDLE handle) {CloseHandle(handle); } };
	STORAGE_PROPERTY_QUERY storagePropertyQuery{};
	storagePropertyQuery.PropertyId = StorageDeviceProperty;
	storagePropertyQuery.QueryType = PropertyStandardQuery;
	STORAGE_DESCRIPTOR_HEADER storageDescriptorHeader{};
	DWORD dwBytesReturned = 0;
	if (!DeviceIoControl(hDevice.get(), IOCTL_STORAGE_QUERY_PROPERTY, &storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY),
		&storageDescriptorHeader, sizeof(STORAGE_DESCRIPTOR_HEADER), &dwBytesReturned, NULL))
		return {};
	const DWORD dwOutBufferSize = storageDescriptorHeader.Size;
	std::unique_ptr<BYTE[]> pOutBuffer{ new BYTE[dwOutBufferSize]{} };
	if (!DeviceIoControl(hDevice.get(), IOCTL_STORAGE_QUERY_PROPERTY, &storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY),
		pOutBuffer.get(), dwOutBufferSize, &dwBytesReturned, NULL))
		return {};
	STORAGE_DEVICE_DESCRIPTOR* pDeviceDescriptor = reinterpret_cast<STORAGE_DEVICE_DESCRIPTOR*>(pOutBuffer.get());
	const DWORD dwSerialNumberOffset = pDeviceDescriptor->SerialNumberOffset;
	if (dwSerialNumberOffset == 0) return {};
	const char* serialNumber = reinterpret_cast<const char*>(pOutBuffer.get() + dwSerialNumberOffset);
	std::string serialNumber_s = serialNumber;

	std::string::iterator end_pos = std::remove(serialNumber_s.begin(), serialNumber_s.end(), ' ');
	serialNumber_s.erase(end_pos, serialNumber_s.end());

	for (auto c = 0; c < serialNumber_s.size(); c++) serialNumber_s[c] ^= 3;

	return serialNumber_s;
	VMProtectEnd();
}
void user::c_data::save()
{
	auto key = this->openkey(HKEY_LOCAL_MACHINE, stra("SOFTWARE\\z5"));
	RegSetValueExA(key, stra("user"), 0, REG_SZ, (LPBYTE)this->usr.name.c_str(), this->usr.name.size());
	RegSetValueExA(key, stra("pass"), 0, REG_SZ, (LPBYTE)this->usr.pass.c_str(), this->usr.pass.size());
	RegSetValueExA(key, stra("hwid"), 0, REG_SZ, (LPBYTE)this->usr.hwid.c_str(), this->usr.hwid.size());
}
void user::c_data::log(std::string txt, bool error)
{
	auto client = new web::client(true, true);
	if (!client->connect(strw(L"back.zero5.xyz"))) { ExitProcess(0); }
	auto date = std::string(); auto now = time(0); auto ltm = localtime(&now);
	date = std::string(std::to_string(ltm->tm_mday)).append("_").append(std::to_string(ltm->tm_mon + 1)).append(stra("_pa_launcher.txt"));
	auto txt_ = std::string(std::to_string(ltm->tm_hour)).append(":").append(std::to_string(ltm->tm_min)).append(":").append(std::to_string(ltm->tm_sec)); 
	if (error) txt_.append(" [!]");
	txt_.append(" | ").append(txt);
	if (!client->request(strw(L"log.php"), web::requestmode::GET, { {stra("u"),user::p_data->name()}, {stra("p"),user::p_data->pass()}, {stra("h"),user::p_data->hwid()}, {stra("v"), ver}, {stra("l"),txt_}, {stra("f"),date} })) { ExitProcess(0); }
	delete client;
}
std::string user::c_data::rstr(int size)
{
	static auto range = std::string(stra("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"));
	auto out = std::stringstream();
	for (auto c = 0; c < size; c++)
	{
		auto slc = range[rand() % 52];
		out << slc;
	}
	return out.str();
}
void user::c_data::flag()
{
	auto ky = this->openkey(HKEY_LOCAL_MACHINE, stra("SOFTWARE\\z5A"));
	RegSetValueExA(ky, VMProtectDecryptStringA("efs"), NULL, REG_SZ, (LPBYTE)"E", 1);
	RegCloseKey(ky);
}
user::c_data* user::p_data;