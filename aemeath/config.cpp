#include "Config.h"
#include <windows.h>
#include <shlobj.h>
#include <fstream>
#include <sstream>
#include "logger.h"
/// <summary>
/// 获取配置文件的完整路径。函数从当前用户的 AppData（使用 CSIDL_APPDATA）获取目录，并在其后追加文件名 aemeath_config.ini。
/// </summary>
/// <returns>返回一个 std::wstring，包含用户的 AppData 目录和追加的文件名 aemeath_config.ini（由 SHGetFolderPathW 获取的路径）。</returns>
std::wstring Config::GetConfigPath()
{
    wchar_t appdata[MAX_PATH];
    SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, appdata);
    std::wstring path = appdata;
    path += L"\\aemeath_config.ini";
    return path;
}

AppConfig Config::Load()
{
    AppConfig cfg;
    std::wstring temp_path = GetConfigPath();
    std::ifstream in(temp_path);
    if (!in.is_open()) return cfg;
    in.close();
    LPCWSTR path = temp_path.c_str();

    WCHAR buffer[256];
    cfg.windowX = GetPrivateProfileIntW(L"Location", L"window_x", 500, path);
    cfg.windowY = GetPrivateProfileIntW(L"Location", L"window_y", 500, path);

    cfg.scaleIndex = GetPrivateProfileIntW(L"General", L"scale_index", 3, path);
    cfg.transparencyIndex = GetPrivateProfileIntW(L"General", L"transparency_index", 0, path);
    GetPrivateProfileStringW(L"General", L"auto_startup", L"false", buffer, 256, path);
    cfg.autoStartup = (_wcsicmp(buffer, L"true") == 0);
    GetPrivateProfileStringW(L"General", L"click_through", L"false", buffer, 256, path);
    cfg.clickThrough = (_wcsicmp(buffer, L"true") == 0);
    GetPrivateProfileStringW(L"General", L"defaultState", L"true", buffer, 256, path);
    cfg.defaultState = (_wcsicmp(buffer, L"true") == 0);

    GetPrivateProfileStringW(L"General", L"follow_mouse", L"false", buffer, 256, path);
    cfg.followMouse = (_wcsicmp(buffer, L"true") == 0);

    cfg.petIdleIndex = GetPrivateProfileIntW(L"General", L"pet_idle_index", 4, path);

#ifdef _DEBUG
    std::stringstream ss;
    ss << "\n[Config::Load] Loaded config:\n"
        << "  window_x           = " << cfg.windowX << "\n"
        << "  window_y           = " << cfg.windowY << "\n"
        << "  scale_index        = " << cfg.scaleIndex << "\n"
        << "  transparency_index = " << cfg.transparencyIndex << "\n"
        << "  pet_idle_index     = " << cfg.petIdleIndex << "\n"
        << "  auto_startup       = " << (cfg.autoStartup ? "true" : "false") << "\n"
        << "  click_through      = " << (cfg.clickThrough ? "true" : "false") << "\n"
        << "  follow_mouse       = " << (cfg.followMouse ? "true" : "false") << "\n"
        << "  defaultState       = " << (cfg.defaultState ? "true" : "false");
    LOG_INFO(ss.str().c_str());
#endif  
    return cfg;
}

void Config::Save(const AppConfig& cfg)
{
    std::wstring temp_path = GetConfigPath();
    LPCWSTR path = temp_path.c_str();
#ifdef _DEBUG
    std::stringstream ss;
    ss << "\n[Config::Save] Saving config:\n"
        << "  window_x           = " << cfg.windowX << "\n"
        << "  window_y           = " << cfg.windowY << "\n"
        << "  scale_index        = " << cfg.scaleIndex << "\n"
        << "  transparency_index = " << cfg.transparencyIndex << "\n"
        << "  pet_idle_index      = " << cfg.petIdleIndex << "\n"
        << "  auto_startup       = " << (cfg.autoStartup ? "true" : "false") << "\n"
        << "  click_through      = " << (cfg.clickThrough ? "true" : "false") << "\n"
        << "  follow_mouse       = " << (cfg.followMouse ? "true" : "false") << "\n"
        << "  defaultState       = " << (cfg.defaultState ? "true" : "false");
    LOG_INFO(ss.str().c_str());

#endif 

    // 写入 [Location] 节
    WritePrivateProfileStringW(L"Location", L"window_x", std::to_wstring(cfg.windowX).c_str(), path);
    WritePrivateProfileStringW(L"Location", L"window_y", std::to_wstring(cfg.windowY).c_str(), path);

    // 写入 [General] 节的整数
    WritePrivateProfileStringW(L"General", L"scale_index", std::to_wstring(cfg.scaleIndex).c_str(), path);
    WritePrivateProfileStringW(L"General", L"transparency_index", std::to_wstring(cfg.transparencyIndex).c_str(), path);
    WritePrivateProfileStringW(L"General", L"pet_idle_index", std::to_wstring(cfg.petIdleIndex).c_str(), path);

    // 写入 [General] 节的布尔值
    WritePrivateProfileStringW(L"General", L"auto_startup", cfg.autoStartup ? L"true" : L"false", path);
    WritePrivateProfileStringW(L"General", L"click_through", cfg.clickThrough ? L"true" : L"false", path);
    WritePrivateProfileStringW(L"General", L"defaultState", cfg.defaultState ? L"true" : L"false", path);
    WritePrivateProfileStringW(L"General", L"follow_mouse", cfg.followMouse ? L"true" : L"false", path);
}