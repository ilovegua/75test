#pragma once
#include <string>
#include <vector>
#include <algorithm>
#define NOMINMAX
#include <Windows.h>




struct Settings {
    
    bool showMenu    = false;

    
    bool espDots      = false;
    bool espBox       = false;
    bool espShield    = false;
    bool espDist      = false;
    bool espSnap      = false;
    bool espSkel      = false;
    bool espName      = false;
    bool espTeamCol   = false;
    bool espVisible   = false;
    bool espOffArrow  = false;
    bool espHeadCircle= false;
    bool useMaxDist   = false;
    float maxDist     = 300.f;

    
    int   boxStyle        = 0;
    float gradientAlpha   = 0.35f;  

    
    bool  aimbotEnabled  = false;
    bool  aimbotVisible  = false;  
    bool  aimbotKnocked  = false;  
    bool  showFovCircle  = false;
    float aimbotFov      = 80.f;
    float aimbotSmooth   = 8.f;
    int   aimbotKey      = VK_RBUTTON; 
    int   aimbotBone     = 0;          

    
    bool showSpectators  = false;
};

extern Settings g_settings;


void SaveConfig(const std::string& name);
bool LoadConfig(const std::string& name);
std::vector<std::string> ListConfigs();
