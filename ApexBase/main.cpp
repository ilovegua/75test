#include <iostream>
#include <thread>
#include <chrono>
#include <algorithm>
#include <array>
#include <cmath>
#define NOMINMAX
#include <Windows.h>
#include <ShlObj.h>
#include <filesystem>
#include <fstream>

#include "OS-ImGui/OS-ImGui.h"
#include "Core/globals.hpp"
#include "Core/settings.hpp"
#include "Entity.hpp"
#include "ESP/esp.hpp"
#include "Aimbot/aimbot.hpp"
#include "Menu/menu.hpp"
#include "Memory/driver.hpp"
#include "Memory/driver.h"
#include "Memory/map.h"

using clock_t2 = std::chrono::steady_clock;




DWORD64  GameBase  = 0;
HWND     hwnd      = 0;
Vector2  ScreenSize = { (float)GetSystemMetrics(SM_CXSCREEN), (float)GetSystemMetrics(SM_CYSCREEN) };

std::atomic<std::shared_ptr<std::vector<EntityWorld>>> g_pubWorld{
    std::make_shared<std::vector<EntityWorld>>() };
std::atomic<std::shared_ptr<std::vector<uint64_t>>> g_pubEntityPtrs{
    std::make_shared<std::vector<uint64_t>>() };
DebugInfo g_debug;




static const std::string DRIVER_PATH = "C:\\Windows\\System32\\Tasks\\kernel.sys";
static const std::string MAPPER_PATH = "C:\\Windows\\System32\\Tasks\\mapper.exe";

static bool LoadKernelDriver()
{
    if (kernel->initialize_handle()) return true;
    {
        std::ofstream f(DRIVER_PATH, std::ios::binary);
        if (!f.write(reinterpret_cast<const char*>(Driver), sizeof(Driver))) return false;
    }
    {
        std::ofstream f(MAPPER_PATH, std::ios::binary);
        if (!f.write(reinterpret_cast<const char*>(Mapper), sizeof(Mapper))) {
            std::filesystem::remove(DRIVER_PATH); return false;
        }
    }
    std::string cmd = MAPPER_PATH + " " + DRIVER_PATH;
    STARTUPINFOA si{ sizeof(si) };
    PROCESS_INFORMATION pi{};
    if (!CreateProcessA(nullptr, const_cast<LPSTR>(cmd.c_str()),
        nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
        std::filesystem::remove(DRIVER_PATH);
        std::filesystem::remove(MAPPER_PATH);
        return false;
    }
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess); CloseHandle(pi.hThread);
    std::filesystem::remove(DRIVER_PATH);
    std::filesystem::remove(MAPPER_PATH);
    if (!kernel->initialize_handle()) {
        char desktop[MAX_PATH]{};
        if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_DESKTOP, nullptr, 0, desktop))) {
            std::string drv = std::string(desktop) + "\\kernel.sys";
            std::string map = std::string(desktop) + "\\mapper.exe";
            { std::ofstream f(drv, std::ios::binary); f.write(reinterpret_cast<const char*>(Driver), sizeof(Driver)); }
            { std::ofstream f(map, std::ios::binary); f.write(reinterpret_cast<const char*>(Mapper), sizeof(Mapper)); }
        }
        return false;
    }
    return true;
}




static bool ReadBones(uint64_t ent, const Vector3& origin,
                      std::array<Vector3, BONE_COUNT>& out)
{
    uint64_t model = I::Read<uint64_t>(ent + 0x1000);
    if (!model) return false;
    uint64_t hdr = I::Read<uint64_t>(model + 0x8);
    if (!hdr) return false;
    uint16_t hitboxCache = I::Read<uint16_t>(hdr + 0x34);
    uint64_t hitboxArray = hdr + (uint64_t)((uint16_t)(hitboxCache & 0xFFFE) << (4 * (hitboxCache & 1)));
    if (!hitboxArray) return false;
    uint16_t indexCache  = I::Read<uint16_t>(hitboxArray + 0x4);
    int      hitboxIndex = (int)((uint16_t)(indexCache & 0xFFFE) << (4 * (indexCache & 1)));
    uint64_t boneArray   = I::Read<uint64_t>(ent + OFF_BoneArray);
    if (!boneArray) return false;
    bool any = false;
    for (int i = 0; i < BONE_COUNT; i++) {
        out[i] = {};  
        uint16_t boneIdx = I::Read<uint16_t>(hitboxArray + hitboxIndex + (i * 0x20));
        if (boneIdx > 255) continue;
        matrix3x4_t mat = I::Read<matrix3x4_t>(boneArray + boneIdx * sizeof(matrix3x4_t));
        Vector3 pos = { mat.m[0][3] + origin.x, mat.m[1][3] + origin.y, mat.m[2][3] + origin.z };
        
        if (pos.x == 0.f && pos.y == 0.f && pos.z == 0.f) continue;
        float dx = pos.x - origin.x, dy = pos.y - origin.y, dz = pos.z - origin.z;
        if (dx*dx + dy*dy + dz*dz > 250000.f) continue; 
        out[i] = pos;
        any = true;
    }
    return any;
}

static void ParseName(uint64_t ent, char* out, int outLen)
{
    out[0] = '\0';
    uint64_t model = I::Read<uint64_t>(ent + OFF_StudioHdr);
    if (!model) return;
    uint64_t hdr = I::Read<uint64_t>(model + OFF_ModelName);
    if (!hdr) return;
    char path[128]{};
    kernel->read(hdr, path, sizeof(path) - 1);
    const char* last = strrchr(path, '/');
    const char* src  = last ? last + 1 : path;
    strncpy_s(out, outLen, src, outLen - 1);
    char* dot = strrchr(out, '.');
    if (dot) *dot = '\0';
}




static void EntityScanThread()
{
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
    constexpr auto INTERVAL = std::chrono::milliseconds(150);
    while (true) {
        auto start = clock_t2::now();
        auto list  = GetEntityList(GameBase);
        g_pubEntityPtrs.store(
            std::make_shared<std::vector<uint64_t>>(std::move(list)),
            std::memory_order_release);
        auto e = clock_t2::now() - start;
        if (e < INTERVAL) std::this_thread::sleep_for(INTERVAL - e);
    }
}




static void PlayerUpdateThread()
{
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
    constexpr auto INTERVAL = std::chrono::milliseconds(15);
    bool debugDone = false;

    while (true) {
        auto start = clock_t2::now();

        if (!debugDone) {
            uint64_t vr = I::Read<uint64_t>(GameBase + OFF_VIEW_RENDER);
            uint64_t vm = is_valid(vr) ? I::Read<uint64_t>(vr + OFF_VIEW_MATRIX) : 0;
            g_debug.mkPass        = (kernel->read_t<uint16_t>(GameBase) == 0x5A4D);
            g_debug.entityListPtr = I::Read<uint64_t>(GameBase + OFF_ENTITYLIST);
            g_debug.viewRenderPtr = vr;
            g_debug.viewMatrixPtr = vm;
            g_debug.dtb           = kernel->dtb;
            g_debug.gameBase      = GameBase;
            g_debug.ready         = true;
            debugDone = true;
        }

        uint64_t localEnt  = I::Read<uint64_t>(GameBase + OFF_LOCAL_PLAYER);
        Vector3  localPos  = localEnt ? I::Read<Vector3>(localEnt + OFF_VecAbsOrigin) : Vector3{};
        int      localTeam = localEnt ? I::Read<int>(localEnt + OFF_TeamId) : -1;

        auto& s = g_settings;
        bool needSkel   = s.espSkel || (s.aimbotEnabled && s.aimbotBone <= 2);
        bool needName   = s.espName;
        bool needVis    = s.espVisible || s.aimbotVisible;
        bool filterDist = s.useMaxDist;
        float maxDist   = s.maxDist;

        auto ptrs  = g_pubEntityPtrs.load(std::memory_order_acquire);
        auto frame = std::make_shared<std::vector<EntityWorld>>();
        frame->reserve(ptrs->size());

        for (uint64_t ent : *ptrs) {
            int lifeState = I::Read<int>(ent + OFF_LifeState);
            if (lifeState != 0 && lifeState != 1) continue; 

            int hp    = I::Read<int>(ent + OFF_iHealth);
            int maxhp = I::Read<int>(ent + OFF_iMaxHealth);
            if (hp < 0 || maxhp <= 0 || maxhp > 200) continue;
            if (hp == 0 && lifeState == 0) continue; 

            int teamId = I::Read<int>(ent + OFF_TeamId);
            if (teamId == localTeam && localTeam != -1) continue;

            int shield    = std::max(0, std::min(I::Read<int>(ent + OFF_Shield),    175));
            int maxShield = std::max(1, std::min(I::Read<int>(ent + OFF_MaxShield), 175));

            Vector3 feet = I::Read<Vector3>(ent + OFF_VecAbsOrigin);
            float dx = feet.x - localPos.x, dy = feet.y - localPos.y, dz = feet.z - localPos.z;
            float dist = sqrtf(dx*dx + dy*dy + dz*dz) / 39.37f;
            if (filterDist && dist > maxDist) continue;

            EntityWorld ew{};
            ew.feet      = feet;
            ew.head      = { feet.x, feet.y, feet.z + 72.f };
            ew.hp        = std::max(0, std::min(hp, 200));
            ew.maxHp     = maxhp;
            ew.shield    = shield;
            ew.maxShield = maxShield;
            ew.dist      = dist;
            ew.teamId    = teamId;
            ew.entPtr    = ent;
            ew.knocked   = (lifeState == 1);
            ew.hasBones  = false;
            ew.name[0]   = '\0';

            if (needVis) {
                float lastActive = I::Read<float>(ent + OFF_LastVisible);
                float curTime    = I::Read<float>(GameBase + 0x7A5E24);
                ew.visible = (curTime - lastActive) < 0.1f;
            }

            if (needSkel)
                ew.hasBones = ReadBones(ent, feet, ew.bones);

            if (needName)
                ParseName(ent, ew.name, sizeof(ew.name));

            frame->push_back(ew);
        }

        g_debug.entityCount = (int)frame->size();
        g_pubWorld.store(frame, std::memory_order_release);

        auto elapsed = clock_t2::now() - start;
        if (elapsed < INTERVAL) std::this_thread::sleep_for(INTERVAL - elapsed);
    }
}




static void RenderFrame()
{
    if (GetAsyncKeyState(VK_INSERT) & 1)
        g_settings.showMenu = !g_settings.showMenu;

    
    uint64_t vr = I::Read<uint64_t>(GameBase + OFF_VIEW_RENDER);
    uint64_t vm = is_valid(vr) ? I::Read<uint64_t>(vr + OFF_VIEW_MATRIX) : 0;
    if (is_valid(vm)) {
        Matrix m = I::Read<Matrix>(vm);
        DrawESP(m);
        RunAimbot(m);
    }

    DrawFovCircle();
    DrawSpectatorList();
    DrawMenu();
}




void main()
{
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    SetConsoleTitleA("this shit is detected nigga");

    std::cout << "[*] Loading driver...\n";
    if (!LoadKernelDriver()) { std::cout << "[-] Driver failed.\n"; system("pause"); exit(1); }
    std::cout << "[+] Driver OK\n";

    std::cout << "[*] Waiting for Apex...\n";
    uint32_t PID = 0;
    while (!PID) { PID = kernel->get_process_pid(L"r5apex_dx12.exe"); if (!PID) Sleep(2000); }
    std::cout << "[+] PID: " << PID << "\n";

    if (!kernel->attach(PID)) { std::cout << "[-] Attach failed.\n"; system("pause"); exit(1); }
    std::cout << "[+] DTB: 0x" << std::hex << kernel->dtb << std::dec << "\n";

    GameBase = kernel->get_image_base(nullptr);
    if (!GameBase) { std::cout << "[-] Base failed.\n"; system("pause"); exit(1); }
    std::cout << "[+] Base: 0x" << std::hex << GameBase << std::dec << "\n";

    while (!hwnd) { hwnd = FindWindowA("Respawn001", "Apex Legends"); if (!hwnd) Sleep(2000); }
    std::cout << "[+] HWND found. Starting...\n";

    std::thread(EntityScanThread).detach();
    std::thread(PlayerUpdateThread).detach();
    std::thread(AimbotThread).detach();

    try {
        Gui.AttachAnotherWindow("Apex Legends", "Respawn001", RenderFrame);
    }
    catch (OSImGui::OSException& e) {
        std::cout << "[-] " << e.what() << "\n";
    }
}
