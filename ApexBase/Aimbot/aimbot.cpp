#include "aimbot.hpp"
#include "../Core/settings.hpp"
#include "../Memory/Interface.hpp"
#include "../Offsets.hpp"
#include <cmath>
#include <thread>
#include <chrono>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static constexpr uint64_t OFF_HEADEYES   = 0x1FC4;
static constexpr uint64_t OFF_VIEWANGLES = 0x2600;

static Vector3 CalcAngle(const Vector3& src, const Vector3& dst)
{
    Vector3 delta = { dst.x - src.x, dst.y - src.y, dst.z - src.z };
    double hyp = sqrt(delta.x * delta.x + delta.y * delta.y);
    Vector3 angle;
    angle.x = (float)(atan2(-delta.z, hyp) * (180.0 / M_PI));
    angle.y = (float)(atan2(delta.y, delta.x) * (180.0 / M_PI));
    angle.z = 0.f;
    while (angle.y >  180.f) angle.y -= 360.f;
    while (angle.y < -180.f) angle.y += 360.f;
    return angle;
}

void DrawFovCircle()
{
    if (!g_settings.showFovCircle) return;
    ImGui::GetBackgroundDrawList()->AddCircle(
        { ScreenSize.x * 0.5f, ScreenSize.y * 0.5f },
        g_settings.aimbotFov,
        ImColor(255, 255, 255, 80), 64, 1.f);
}


void AimbotThread()
{
    using clock = std::chrono::steady_clock;
    constexpr auto INTERVAL = std::chrono::milliseconds(1);

    while (true) {
        auto start = clock::now();

        auto& s = g_settings;
        if (!s.aimbotEnabled || !(GetAsyncKeyState(s.aimbotKey) & 0x8000))
            goto sleep;

        {
            uint64_t localEnt = I::Read<uint64_t>(GameBase + OFF_LOCAL_PLAYER);
            if (!localEnt) goto sleep;

            Vector3 eyePos    = I::Read<Vector3>(localEnt + OFF_HEADEYES);
            Vector3 viewAngle = I::Read<Vector3>(localEnt + OFF_VIEWANGLES);

            auto world = g_pubWorld.load(std::memory_order_acquire);
            if (!world || world->empty()) goto sleep;

            float   bestFov   = s.aimbotFov * s.aimbotFov;
            Vector3 bestAngle = {};
            bool    found     = false;

            for (auto& e : *world) {
                if (s.aimbotKnocked && e.knocked)  continue;
                if (s.aimbotVisible && !e.visible) continue;

                Vector3 targetWorld;
                if      (s.aimbotBone == 0 && e.hasBones) targetWorld = e.bones[0];
                else if (s.aimbotBone == 1 && e.hasBones) targetWorld = e.bones[1];
                else if (s.aimbotBone == 2 && e.hasBones) targetWorld = e.bones[2];
                else                                       targetWorld = e.head;

                Vector3 targetAngle = CalcAngle(eyePos, targetWorld);

                float dx = targetAngle.x - viewAngle.x;
                float dy = targetAngle.y - viewAngle.y;
                while (dy >  180.f) dy -= 360.f;
                while (dy < -180.f) dy += 360.f;

                float fovDist = dx*dx + dy*dy;
                if (fovDist < bestFov) {
                    bestFov   = fovDist;
                    bestAngle = targetAngle;
                    found     = true;
                }
            }

            if (found) {
                float dx = bestAngle.x - viewAngle.x;
                float dy = bestAngle.y - viewAngle.y;
                while (dy >  180.f) dy -= 360.f;
                while (dy < -180.f) dy += 360.f;

                
                
                
                float factor = 1.f / s.aimbotSmooth;

                Vector3 newAngle = {
                    viewAngle.x + dx * factor,
                    viewAngle.y + dy * factor,
                    0.f
                };

                I::Write<Vector3>(localEnt + OFF_VIEWANGLES, newAngle);
            }
        }

        sleep:
        auto elapsed = clock::now() - start;
        if (elapsed < INTERVAL)
            std::this_thread::sleep_for(INTERVAL - elapsed);
    }
}


void RunAimbot(const Matrix&) {}
