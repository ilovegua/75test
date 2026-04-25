#include "menu.hpp"
#include "../Core/settings.hpp"
#include "../Core/globals.hpp"
#include "../OS-ImGui/OS-ImGui.h"
#include "../Offsets.hpp"
#include "../Memory/Interface.hpp"
#include <string>
#include <vector>




static const char* KeyName(int vk)
{
    switch (vk) {
    case VK_RBUTTON:  return "RMB";
    case VK_LBUTTON:  return "LMB";
    case VK_MBUTTON:  return "MMB";
    case VK_XBUTTON1: return "MB4";
    case VK_XBUTTON2: return "MB5";
    case VK_LSHIFT:   return "LShift";
    case VK_RSHIFT:   return "RShift";
    case VK_LCONTROL: return "LCtrl";
    case VK_RCONTROL: return "RCtrl";
    case VK_LMENU:    return "LAlt";
    case VK_RMENU:    return "RAlt";
    case VK_CAPITAL:  return "CapsLock";
    case VK_F1:  return "F1";  case VK_F2:  return "F2";
    case VK_F3:  return "F3";  case VK_F4:  return "F4";
    case VK_F5:  return "F5";  case VK_F6:  return "F6";
    default: {
        static char buf[8]{};
        snprintf(buf, sizeof(buf), "0x%02X", vk);
        return buf;
    }
    }
}

static bool g_bindingKey = false;

static void KeybindButton(const char* label, int& key)
{
    char buf[48];
    if (g_bindingKey)
        snprintf(buf, sizeof(buf), "[ Press a key... ]");
    else
        snprintf(buf, sizeof(buf), "%s  [ %s ]", label, KeyName(key));

    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.13f,0.13f,0.17f,1.f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f,0.20f,0.28f,1.f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.25f,0.25f,0.35f,1.f));
    if (ImGui::Button(buf, {240.f, 28.f}))
        g_bindingKey = true;
    ImGui::PopStyleColor(3);

    if (g_bindingKey) {
        for (int k = 1; k < 256; k++) {
            if (GetAsyncKeyState(k) & 0x8000) {
                if (k != VK_ESCAPE) key = k;
                g_bindingKey = false;
                break;
            }
        }
    }
}


static void SectionHeader(const char* label)
{
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.45f, 0.75f, 1.f, 1.f));
    ImGui::Text("%s", label);
    ImGui::PopStyleColor();
    ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.25f, 0.35f, 0.55f, 1.f));
    ImGui::Separator();
    ImGui::PopStyleColor();
    ImGui::Spacing();
}




static void DrawESPTab()
{
    auto& s = g_settings;

    SectionHeader("Visibility");
    ImGui::Columns(2, "##espcols", false);
    ImGui::SetColumnWidth(0, 240.f);

    Gui.MyCheckBox("Dot",        &s.espDots);     ImGui::Spacing();
    Gui.MyCheckBox("Box",   &s.espBox);      ImGui::Spacing();
    Gui.MyCheckBox("Shitty Head Circle",    &s.espHeadCircle);ImGui::Spacing();
    Gui.MyCheckBox("Skeleton",       &s.espSkel);     ImGui::Spacing();
    Gui.MyCheckBox("Snaplines",      &s.espSnap);     ImGui::Spacing();

    ImGui::NextColumn();

    Gui.MyCheckBox("Shield Bar",     &s.espShield);   ImGui::Spacing();
    Gui.MyCheckBox("Distance",       &s.espDist);     ImGui::Spacing();
    Gui.MyCheckBox("Name Tag",       &s.espName);     ImGui::Spacing();
    Gui.MyCheckBox("Team Colors",    &s.espTeamCol);  ImGui::Spacing();
    Gui.MyCheckBox("Off-Screen",     &s.espOffArrow); ImGui::Spacing();

    ImGui::Columns(1);

    SectionHeader("Box Style");
    ImGui::SetNextItemWidth(220.f);
    const char* boxStyles[] = { "Full Box", "Cornered Box", "Gradient" };
    ImGui::Combo("##boxstyle", &s.boxStyle, boxStyles, 3);

    if (s.boxStyle == 2) {
        ImGui::Spacing();
        ImGui::Text("Gradient Transparency");
        ImGui::SetNextItemWidth(220.f);
        ImGui::SliderFloat("##galpha", &s.gradientAlpha, 0.05f, 1.f, "%.2f");
    }

    SectionHeader("Filters");
    Gui.MyCheckBox("Visible Check",  &s.espVisible);
    ImGui::Spacing();
    Gui.MyCheckBox("Max Distance",   &s.useMaxDist);
    if (s.useMaxDist) {
        ImGui::SameLine(200.f);
        ImGui::SetNextItemWidth(180.f);
        ImGui::SliderFloat("##maxdist", &s.maxDist, 10.f, 600.f, "%.0f m");
    }
    ImGui::Spacing();
}




static void DrawAimbotTab()
{
    auto& s = g_settings;

    SectionHeader("General");
    Gui.MyCheckBox("Enable niggabot",   &s.aimbotEnabled);  ImGui::Spacing();
    Gui.MyCheckBox("Visible Only",    &s.aimbotVisible);  ImGui::Spacing();
    Gui.MyCheckBox("Ignore Knocked",  &s.aimbotKnocked);  ImGui::Spacing();
    Gui.MyCheckBox("Show FOV Circle", &s.showFovCircle);  ImGui::Spacing();

    SectionHeader("Settings");

    ImGui::Text("FOV Radius");
    ImGui::SetNextItemWidth(260.f);
    Gui.SliderScalarEx2("##fov", ImGuiDataType_Float, &s.aimbotFov,
        []{ static float mn=10.f;  return &mn; }(),
        []{ static float mx=300.f; return &mx; }(), "%.0f px", 0);
    ImGui::Spacing();

    ImGui::Text("Smoothness");
    ImGui::SetNextItemWidth(260.f);
    Gui.SliderScalarEx2("##smooth", ImGuiDataType_Float, &s.aimbotSmooth,
        []{ static float mn=1.f;  return &mn; }(),
        []{ static float mx=30.f; return &mx; }(), "%.1f", 0);
    ImGui::Spacing();

    ImGui::Text("Target Bone");
    ImGui::SetNextItemWidth(260.f);
    const char* bones[] = { "Head", "Neck", "Chest" };
    ImGui::Combo("##bone", &s.aimbotBone, bones, 3);

    SectionHeader("Hotkey");
    KeybindButton("Aimbot Key", s.aimbotKey);
    ImGui::Spacing();
}




static void DrawConfigTab()
{
    static char newName[64] = "default";
    static std::vector<std::string> configs;
    static bool refreshed = false;
    if (!refreshed) { configs = ListConfigs(); refreshed = true; }

    SectionHeader("Save Config");
    ImGui::Text("Name:");
    ImGui::SetNextItemWidth(240.f);
    ImGui::InputText("##cfgname", newName, sizeof(newName));
    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.15f,0.35f,0.15f,1.f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f,0.45f,0.20f,1.f));
    if (ImGui::Button("  Save  ", {110.f, 28.f})) {
        SaveConfig(newName);
        configs = ListConfigs();
    }
    ImGui::PopStyleColor(2);
    ImGui::SameLine();
    if (ImGui::Button("  Refresh  ", {110.f, 28.f}))
        configs = ListConfigs();

    SectionHeader("Load Config");

    if (configs.empty()) {
        ImGui::TextDisabled("No configs found.");
    } else {
        for (auto& cfg : configs) {
            ImGui::PushID(cfg.c_str());
            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.13f,0.20f,0.35f,1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.18f,0.28f,0.48f,1.f));
            if (ImGui::Button("Load", {70.f, 24.f}))
                LoadConfig(cfg);
            ImGui::PopStyleColor(2);
            ImGui::SameLine();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("%s", cfg.c_str());
            ImGui::PopID();
            ImGui::Spacing();
        }
    }
    ImGui::Spacing();
}




static void DrawDebugTab()
{
    if (!g_debug.ready) { ImGui::TextDisabled("Waiting for first game tick..."); return; }

    SectionHeader("Driver / Process");
    ImGui::Text("GameBase   0x%llX", g_debug.gameBase);
    ImGui::Text("DTB (CR3)  0x%llX", g_debug.dtb);
    ImGui::Text("MK Check   %s",     g_debug.mkPass ? "PASS" : "FAIL");

    SectionHeader("Offsets");
    ImGui::Text("EntityList  OFF=0x%llX  ->  0x%llX", OFF_ENTITYLIST,  g_debug.entityListPtr);
    ImGui::Text("ViewRender  OFF=0x%llX  ->  0x%llX", OFF_VIEW_RENDER, g_debug.viewRenderPtr);
    ImGui::Text("ViewMatrix  OFF=0x%llX  ->  0x%llX", OFF_VIEW_MATRIX, g_debug.viewMatrixPtr);

    SectionHeader("Entities");
    ImGui::Text("Tracked: %d   Boxed: %d", g_debug.entityCount, g_debug.boxCount);
    ImGui::Spacing();

    bool ok = is_valid(g_debug.entityListPtr) &&
              is_valid(g_debug.viewRenderPtr)  &&
              is_valid(g_debug.viewMatrixPtr);
    ImGui::TextColored(ok ? ImVec4{0.2f,1.f,0.2f,1.f} : ImVec4{1.f,0.3f,0.3f,1.f},
                       ok ? "  Offsets r working nigga" : "  offsets not working nigga");
    ImGui::Spacing();
}




void DrawSpectatorList()
{
    if (!g_settings.showSpectators) return;
    constexpr uint64_t OFF_OBSERVERLIST = 0x644f158;
    constexpr int MAX_SPECS = 8;

    std::vector<std::string> specs;
    for (int i = 0; i < MAX_SPECS; i++) {
        uint64_t obs = I::Read<uint64_t>(GameBase + OFF_OBSERVERLIST + i * 0x8);
        if (!is_valid(obs)) continue;
        char name[64]{};
        uint64_t model = I::Read<uint64_t>(obs + 0x1000);
        if (model) {
            uint64_t hdr = I::Read<uint64_t>(model + 0x8);
            if (hdr) {
                char path[128]{};
                kernel->read(hdr, path, sizeof(path) - 1);
                const char* last = strrchr(path, '/');
                const char* src  = last ? last + 1 : path;
                strncpy_s(name, src, sizeof(name) - 1);
                char* dot = strrchr(name, '.');
                if (dot) *dot = '\0';
            }
        }
        if (name[0] != '\0') specs.push_back(name);
    }
    if (specs.empty()) return;

    ImGui::SetNextWindowPos({ ScreenSize.x - 170.f, 40.f }, ImGuiCond_Always);
    ImGui::SetNextWindowSize({ 160.f, 0.f }, ImGuiCond_Always);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.07f,0.07f,0.09f,0.88f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.f);
    ImGui::Begin("##specs", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove     | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoInputs);
    ImGui::TextColored({0.35f,0.75f,1.f,1.f}, "Spectators  (%d)", (int)specs.size());
    ImGui::Separator();
    ImGui::Spacing();
    for (auto& n : specs) {
        ImGui::TextColored({0.85f,0.85f,0.85f,1.f}, "  %s", n.c_str());
    }
    ImGui::Spacing();
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}




void DrawMenu()
{
    if (!g_settings.showMenu) return;

    
    ImGui::SetNextWindowSize({ 580.f, 520.f }, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos ({ 120.f, 80.f  }, ImGuiCond_FirstUseEver);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding,    8.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize,  1.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,     ImVec2(16.f, 16.f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,       ImVec2(10.f, 10.f));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,      ImVec2(8.f,  5.f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg,  ImVec4(0.06f, 0.06f, 0.08f, 0.97f));
    ImGui::PushStyleColor(ImGuiCol_Border,    ImVec4(0.18f, 0.22f, 0.32f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg,   ImVec4(0.10f, 0.10f, 0.14f, 1.f));

    ImGui::Begin("##apex", nullptr,
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoScrollbar);

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(5);

    
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.09f, 0.09f, 0.13f, 1.f));
    ImGui::BeginChild("##hdr", { 0.f, 44.f }, false);
    {
        
        ImVec2 p = ImGui::GetWindowPos();
        ImGui::GetWindowDrawList()->AddRectFilled(
            p, { p.x + ImGui::GetWindowWidth(), p.y + 2.f },
            IM_COL32(80, 140, 255, 255));

        float tw = ImGui::CalcTextSize("APEX EXTERNAL").x;
        ImGui::SetCursorPos({ (ImGui::GetWindowWidth() - tw) * 0.5f, 13.f });
        ImGui::TextColored({ 0.40f, 0.78f, 1.f, 1.f }, "APEX EXTERNAL");
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();

    ImGui::Spacing();
    ImGui::Spacing();

    
    ImGui::PushStyleColor(ImGuiCol_Tab,             ImVec4(0.09f, 0.09f, 0.12f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_TabActive,       ImVec4(0.14f, 0.22f, 0.40f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_TabHovered,      ImVec4(0.12f, 0.18f, 0.30f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_TabUnfocused,    ImVec4(0.09f, 0.09f, 0.12f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_TabUnfocusedActive, ImVec4(0.12f,0.18f,0.30f,1.f));
    ImGui::PushStyleVar(ImGuiStyleVar_TabRounding, 4.f);

    if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton)) {
        if (ImGui::BeginTabItem("  NIGESP  "))    { DrawESPTab();    ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("  Aimbot  ")) { DrawAimbotTab(); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("  Config  ")) { DrawConfigTab(); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("  Debuge  "))  { DrawDebugTab();  ImGui::EndTabItem(); }
        ImGui::EndTabBar();
    }

    ImGui::PopStyleVar(1);
    ImGui::PopStyleColor(5);
    ImGui::End();
}
