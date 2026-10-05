#include "Config.h"
#include <windows.h>
#include <shlobj.h>
#include <fstream>
#include <sstream>
#include "logger.h"
/// <summary>
/// 获取配置文件的完整路径。函数从当前用户的 AppData（使用 CSIDL_APPDATA）获取目录，并在其后追加文件名 aemeath_config.ini
/// </summary>
/// <returns>返回一个 std::string，包含用户的 AppData 目录和追加的文件名 aemeath_config.ini（由 SHGetFolderPathW 获取的路径）</returns>
std::string Config::GetConfigPath()
{
    CHAR appdata[MAX_PATH];
    SHGetFolderPath(nullptr, CSIDL_APPDATA, nullptr, 0, appdata);
    std::string path = appdata;
    path += "\\aemeath_config.ini";
    return path;
}

AppConfig Config::Load()
{
    AppConfig cfg;
    std::string temp_path = GetConfigPath();
    std::ifstream in(temp_path);
    if (!in.is_open()) return cfg;
    in.close();
    LPCSTR path = temp_path.c_str();

    CHAR buffer[256];
    cfg.windowX = GetPrivateProfileInt("Location", "window_x", 500, path);
    cfg.windowY = GetPrivateProfileInt("Location", "window_y", 500, path);

    cfg.scaleIndex = GetPrivateProfileInt("General", "scale_index", 3, path);
    cfg.transparencyIndex = GetPrivateProfileInt("General", "transparency_index", 0, path);
    GetPrivateProfileString("General", "click_through", "false", buffer, 256, path);
    cfg.clickThrough = (_stricmp(buffer, "true") == 0);
    GetPrivateProfileString("General", "defaultState", "true", buffer, 256, path);
    cfg.defaultState = (_stricmp(buffer, "true") == 0);

    GetPrivateProfileString("General", "follow_mouse", "false", buffer, 256, path);
    cfg.followMouse = (_stricmp(buffer, "true") == 0);

    cfg.petIdleIndex = GetPrivateProfileInt("General", "pet_idle_index", 4, path);

#ifdef _DEBUG
    std::stringstream ss;
    ss << "\n[Config::Load] Loaded config:\n"
        << "  window_x           = " << cfg.windowX << "\n"
        << "  window_y           = " << cfg.windowY << "\n"
        << "  scale_index        = " << cfg.scaleIndex << "\n"
        << "  transparency_index = " << cfg.transparencyIndex << "\n"
        << "  pet_idle_index     = " << cfg.petIdleIndex << "\n"
        << "  click_through      = " << (cfg.clickThrough ? "true" : "false") << "\n"
        << "  follow_mouse       = " << (cfg.followMouse ? "true" : "false") << "\n"
        << "  defaultState       = " << (cfg.defaultState ? "true" : "false");
    LOG_INFO(ss.str().c_str());
#endif  
    return cfg;
}

void Config::Save(const AppConfig& cfg)
{
    std::string temp_path = GetConfigPath();
    LPCSTR path = temp_path.c_str();
#ifdef _DEBUG
    std::stringstream ss;
    ss << "\n[Config::Save] Saving config:\n"
        << "  window_x           = " << cfg.windowX << "\n"
        << "  window_y           = " << cfg.windowY << "\n"
        << "  scale_index        = " << cfg.scaleIndex << "\n"
        << "  transparency_index = " << cfg.transparencyIndex << "\n"
        << "  pet_idle_index      = " << cfg.petIdleIndex << "\n"
        << "  click_through      = " << (cfg.clickThrough ? "true" : "false") << "\n"
        << "  follow_mouse       = " << (cfg.followMouse ? "true" : "false") << "\n"
        << "  defaultState       = " << (cfg.defaultState ? "true" : "false");
    LOG_INFO(ss.str().c_str());

#endif 

    // 写入 [Location] 节
    WritePrivateProfileString("Location", "window_x", std::to_string(cfg.windowX).c_str(), path);
    WritePrivateProfileString("Location", "window_y", std::to_string(cfg.windowY).c_str(), path);

    // 写入 [General] 节的整数
    WritePrivateProfileString("General", "scale_index", std::to_string(cfg.scaleIndex).c_str(), path);
    WritePrivateProfileString("General", "transparency_index", std::to_string(cfg.transparencyIndex).c_str(), path);
    WritePrivateProfileString("General", "pet_idle_index", std::to_string(cfg.petIdleIndex).c_str(), path);

    // 写入 [General] 节的布尔值
    WritePrivateProfileString("General", "click_through", cfg.clickThrough ? "true" : "false", path);
    WritePrivateProfileString("General", "defaultState", cfg.defaultState ? "true" : "false", path);
    WritePrivateProfileString("General", "follow_mouse", cfg.followMouse ? "true" : "false", path);
}