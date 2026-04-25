#include "settings.hpp"
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <Windows.h>

Settings g_settings;

static std::string ConfigDir()
{
    char buf[MAX_PATH]{};
    GetModuleFileNameA(nullptr, buf, MAX_PATH);
    std::filesystem::path p(buf);
    auto dir = p.parent_path() / "configs";
    std::filesystem::create_directories(dir);
    return dir.string();
}

static std::string ConfigPath(const std::string& name)
{
    return ConfigDir() + "\\" + name + ".cfg";
}


void SaveConfig(const std::string& name)
{
    std::ofstream f(ConfigPath(name));
    if (!f) return;

    auto b = [&](const char* k, bool v)  { f << k << "=" << (v ? 1 : 0) << "\n"; };
    auto fl= [&](const char* k, float v) { f << k << "=" << v << "\n"; };
    auto i = [&](const char* k, int v)   { f << k << "=" << v << "\n"; };

    b("espDots",      g_settings.espDots);
    b("espBox",       g_settings.espBox);
    b("espShield",    g_settings.espShield);
    b("espDist",      g_settings.espDist);
    b("espSnap",      g_settings.espSnap);
    b("espSkel",      g_settings.espSkel);
    b("espName",      g_settings.espName);
    b("espTeamCol",   g_settings.espTeamCol);
    b("espVisible",   g_settings.espVisible);
    b("espOffArrow",  g_settings.espOffArrow);
    b("useMaxDist",   g_settings.useMaxDist);
    fl("maxDist",     g_settings.maxDist);
    i("boxStyle",     g_settings.boxStyle);
    fl("gradientAlpha",g_settings.gradientAlpha);
    b("espHeadCircle",g_settings.espHeadCircle);
    b("aimbotEnabled",g_settings.aimbotEnabled);
    b("aimbotVisible",g_settings.aimbotVisible);
    b("aimbotKnocked",g_settings.aimbotKnocked);
    b("showFovCircle",g_settings.showFovCircle);
    fl("aimbotFov",   g_settings.aimbotFov);
    fl("aimbotSmooth",g_settings.aimbotSmooth);
    i("aimbotKey",    g_settings.aimbotKey);
    i("aimbotBone",   g_settings.aimbotBone);
    b("showSpectators",g_settings.showSpectators);
}

bool LoadConfig(const std::string& name)
{
    std::ifstream f(ConfigPath(name));
    if (!f) return false;

    std::string line;
    while (std::getline(f, line)) {
        auto eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);

        auto b  = [&](const char* k, bool& v)  { if (key == k) v = (val == "1"); };
        auto fl = [&](const char* k, float& v) { if (key == k) v = std::stof(val); };
        auto i  = [&](const char* k, int& v)   { if (key == k) v = std::stoi(val); };

        b("espDots",      g_settings.espDots);
        b("espBox",       g_settings.espBox);
        b("espShield",    g_settings.espShield);
        b("espDist",      g_settings.espDist);
        b("espSnap",      g_settings.espSnap);
        b("espSkel",      g_settings.espSkel);
        b("espName",      g_settings.espName);
        b("espTeamCol",   g_settings.espTeamCol);
        b("espVisible",   g_settings.espVisible);
        b("espOffArrow",  g_settings.espOffArrow);
        b("useMaxDist",   g_settings.useMaxDist);
        fl("maxDist",     g_settings.maxDist);
        i("boxStyle",     g_settings.boxStyle);
        fl("gradientAlpha",g_settings.gradientAlpha);
        b("espHeadCircle",g_settings.espHeadCircle);
        b("aimbotEnabled",g_settings.aimbotEnabled);
        b("aimbotVisible",g_settings.aimbotVisible);
        b("aimbotKnocked",g_settings.aimbotKnocked);
        b("showFovCircle",g_settings.showFovCircle);
        fl("aimbotFov",   g_settings.aimbotFov);
        fl("aimbotSmooth",g_settings.aimbotSmooth);
        i("aimbotKey",    g_settings.aimbotKey);
        i("aimbotBone",   g_settings.aimbotBone);
        b("showSpectators",g_settings.showSpectators);
    }
    return true;
}

std::vector<std::string> ListConfigs()
{
    std::vector<std::string> out;
    for (auto& entry : std::filesystem::directory_iterator(ConfigDir())) {
        if (entry.path().extension() == ".cfg")
            out.push_back(entry.path().stem().string());
    }
    return out;
}
