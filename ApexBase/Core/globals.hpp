#pragma once
#include <atomic>
#include <memory>
#include <vector>
#include <array>
#include <cstdint>
#include <Windows.h>
#include "../Structs.hpp"
#include "../Offsets.hpp"




struct matrix3x4_t { float m[3][4]; };

static constexpr int BONE_COUNT      = 19;
static constexpr int SKEL_PAIR_COUNT = 16;
static constexpr int SKEL_PAIRS[SKEL_PAIR_COUNT][2] = {
    {1,2},{2,3},{3,4},{4,5},
    {1,6},{6,7},{7,8},
    {1,9},{9,10},{10,11},
    {5,12},{12,13},{13,14},
    {5,16},{16,17},{17,18},
};




struct EntityWorld {
    Vector3  feet, head;
    int      hp, maxHp;
    int      shield, maxShield;
    float    dist;
    int      teamId;
    bool     visible;
    bool     knocked;
    bool     hasBones;
    std::array<Vector3, BONE_COUNT> bones;
    char     name[64];
    uint64_t entPtr;
};




struct DebugInfo {
    bool     mkPass        = false;
    uint64_t entityListPtr = 0;
    uint64_t viewRenderPtr = 0;
    uint64_t viewMatrixPtr = 0;
    uint64_t dtb           = 0;
    uint64_t gameBase      = 0;
    int      entityCount   = 0;
    int      boxCount      = 0;
    bool     ready         = false;
};




extern DWORD64  GameBase;
extern HWND     hwnd;
extern Vector2  ScreenSize;

extern std::atomic<std::shared_ptr<std::vector<EntityWorld>>> g_pubWorld;
extern std::atomic<std::shared_ptr<std::vector<uint64_t>>>    g_pubEntityPtrs;
extern DebugInfo g_debug;
