#include "esp.hpp"
#include "../Core/settings.hpp"
#include <algorithm>

ImColor TeamColor(int teamId)
{
    static const ImColor palette[] = {
        ImColor(255, 80,  80,  255),
        ImColor(80,  180, 255, 255),
        ImColor(80,  255, 120, 255),
        ImColor(255, 200, 60,  255),
        ImColor(200, 80,  255, 255),
        ImColor(255, 140, 40,  255),
        ImColor(60,  230, 220, 255),
        ImColor(255, 100, 180, 255),
    };
    return palette[abs(teamId) % 8];
}


static void DrawFullBox(ImDrawList* dl, float x, float y, float w, float h, ImColor col)
{
    dl->AddRect({ x-1, y-1 }, { x+w+1, y+h+1 }, ImColor(0,0,0,180), 0.f, 0, 3.f);
    dl->AddRect({ x,   y   }, { x+w,   y+h   }, col, 0.f, 0, 1.5f);
}

static void DrawCorneredBox(ImDrawList* dl, float x, float y, float w, float h, ImColor col)
{
    float cw = w / 3.f, ch = h / 3.f;
    ImColor out(0, 0, 0, 160);
    float t = 1.5f;
    auto ln = [&](ImVec2 a, ImVec2 b, ImColor c, float th){ dl->AddLine(a,b,c,th); };
    
    ln({x,      y},   {x,      y+ch},  out,t+1); ln({x,      y},   {x+cw,  y},    out,t+1);
    ln({x+w,    y},   {x+w,    y+ch},  out,t+1); ln({x+w-cw, y},   {x+w,   y},    out,t+1);
    ln({x,      y+h}, {x,      y+h-ch},out,t+1); ln({x,      y+h}, {x+cw,  y+h},  out,t+1);
    ln({x+w,    y+h}, {x+w,    y+h-ch},out,t+1); ln({x+w-cw, y+h}, {x+w,   y+h},  out,t+1);
    
    ln({x,      y},   {x,      y+ch},  col,t); ln({x,      y},   {x+cw,  y},    col,t);
    ln({x+w,    y},   {x+w,    y+ch},  col,t); ln({x+w-cw, y},   {x+w,   y},    col,t);
    ln({x,      y+h}, {x,      y+h-ch},col,t); ln({x,      y+h}, {x+cw,  y+h},  col,t);
    ln({x+w,    y+h}, {x+w,    y+h-ch},col,t); ln({x+w-cw, y+h}, {x+w,   y+h},  col,t);
}

static void DrawGradientBox(ImDrawList* dl, float x, float y, float w, float h, float alpha)
{
    
    int a = (int)(alpha * 255.f);
    ImU32 topCol = IM_COL32(140, 60, 255, a);
    ImU32 botCol = IM_COL32(40,  120, 255, a);
    dl->AddRectFilledMultiColor(
        { x, y }, { x+w, y+h },
        topCol, topCol, botCol, botCol);
    
    dl->AddRect({ x-1, y-1 }, { x+w+1, y+h+1 }, IM_COL32(0,0,0,160), 0.f, 0, 2.f);
    dl->AddRect({ x,   y   }, { x+w,   y+h   }, IM_COL32(160, 80, 255, 200), 0.f, 0, 1.f);
}


static void DrawOffscreenArrow(ImDrawList* dl, float cx, float cy,
                                float tx, float ty, ImColor col)
{
    float dx = tx - cx, dy = ty - cy;
    float angle = atan2f(dy, dx);
    float hw = ScreenSize.x * 0.5f - 40.f;
    float hh = ScreenSize.y * 0.5f - 40.f;
    float ax = cx, ay = cy;
    if (fabsf(dx) * hh > fabsf(dy) * hw) {
        float s = (dx > 0 ? hw : -hw) / fabsf(dx);
        ax = cx + dx*s; ay = cy + dy*s;
    } else {
        float s = (dy > 0 ? hh : -hh) / fabsf(dy);
        ax = cx + dx*s; ay = cy + dy*s;
    }
    float sz = 10.f, ca = cosf(angle), sa = sinf(angle);
    ImVec2 tip  = { ax+ca*sz,              ay+sa*sz };
    ImVec2 left = { ax-ca*sz*.5f+sa*sz*.5f, ay-sa*sz*.5f-ca*sz*.5f };
    ImVec2 right= { ax-ca*sz*.5f-sa*sz*.5f, ay-sa*sz*.5f+ca*sz*.5f };
    dl->AddTriangleFilled(tip, left, right, col);
    dl->AddTriangle(tip, left, right, ImColor(0,0,0,180), 1.5f);
}


void DrawESP(const Matrix& m)
{
    auto& s    = g_settings;
    auto  world = g_pubWorld.load(std::memory_order_acquire);
    ImDrawList* dl = ImGui::GetBackgroundDrawList();

    float cx = ScreenSize.x * 0.5f;
    float cy = ScreenSize.y * 0.5f;
    int   boxCount = 0;

    for (auto& e : *world) {
        float hpPct = (float)e.hp / (float)e.maxHp;
        float shPct = (float)e.shield / (float)e.maxShield;
        float cr = (1.f - hpPct) * 255.f, cg = hpPct * 255.f;
        ImColor hpCol((int)cr, (int)cg, 0, 230);
        ImColor shCol(100, 180, 255, 230);
        ImColor boxCol = s.espTeamCol ? TeamColor(e.teamId) : ImColor(220, 220, 220, 255);
        if (s.espVisible && e.visible) boxCol = ImColor(255, 60, 60, 255);
        if (e.knocked)                 boxCol = ImColor(150, 150, 150, 180);

        Vector3 feetS = _WorldToScreen(e.feet, m, ScreenSize);
        Vector3 headS = _WorldToScreen(e.head, m, ScreenSize);
        
        bool behindCamera = feetS.z <= 0.f;
        bool onScreen = !behindCamera && headS.z > 0.f &&
                        feetS.x > -50.f && feetS.x < ScreenSize.x + 50.f &&
                        feetS.y > -50.f && feetS.y < ScreenSize.y + 50.f;

        float boxH = onScreen ? fabsf(feetS.y - headS.y) : 0.f;
        float boxW = boxH * 0.55f;
        float bx   = feetS.x - boxW * 0.5f;
        float by   = onScreen ? (headS.y < feetS.y ? headS.y : feetS.y) : 0.f;

        if (s.espOffArrow && behindCamera) {
            DrawOffscreenArrow(dl, cx, cy, e.feet.x, e.feet.y, boxCol);
            continue;
        }
        if (!onScreen || boxH < 5.f) continue;

        
        if (s.espSnap)
            dl->AddLine({ cx, ScreenSize.y }, { feetS.x, feetS.y }, ImColor(220,220,220,100), 1.f);

        
        if (s.espDots) {
            dl->AddCircleFilled({ feetS.x, feetS.y }, 4.f, hpCol);
            dl->AddCircle      ({ feetS.x, feetS.y }, 4.5f, ImColor(0,0,0,180), 0, 1.2f);
        }

        
        if (s.espBox) {
            if      (s.boxStyle == 0) DrawFullBox    (dl, bx, by, boxW, boxH, boxCol);
            else if (s.boxStyle == 1) DrawCorneredBox(dl, bx, by, boxW, boxH, boxCol);
            else if (s.boxStyle == 2) DrawGradientBox(dl, bx, by, boxW, boxH, s.gradientAlpha);
            boxCount++;
        }

        
        if (s.espBox || s.espShield) {
            float barX = bx - 6.f, hbH = boxH * hpPct;
            dl->AddRectFilled({barX-1, by-1},        {barX+3, by+boxH+1}, ImColor(0,0,0,200));
            dl->AddRectFilled({barX,   by+boxH-hbH}, {barX+2, by+boxH},   hpCol);
        }

        
        if (s.espShield) {
            float barX2 = bx + boxW + 4.f, sbH = boxH * shPct;
            dl->AddRectFilled({barX2-1, by-1},        {barX2+3, by+boxH+1}, ImColor(0,0,0,200));
            dl->AddRectFilled({barX2,   by+boxH-sbH}, {barX2+2, by+boxH},   shCol);
        }

        
        if (s.espHeadCircle) {
            Vector3 headWorld = (e.hasBones) ? e.bones[0] : e.head;
            Vector3 hs = _WorldToScreen(headWorld, m, ScreenSize);
            if (hs.z > 0.f) {
                
                float radius = (boxH * 0.12f);
                radius = (radius < 3.f) ? 3.f : radius;
                dl->AddCircle({ hs.x, hs.y }, radius + 1.f, ImColor(0,0,0,180), 0, 2.f);
                dl->AddCircle({ hs.x, hs.y }, radius,       boxCol,             0, 1.5f);
            }
        }

        
        if (s.espDist) {
            char buf[32];
            snprintf(buf, sizeof(buf), "%.0fm", e.dist);
            float fs = 11.f;
            ImVec2 tsz = ImGui::GetFont()->CalcTextSizeA(fs, FLT_MAX, 0.f, buf);
            float tx = feetS.x - tsz.x * 0.5f, ty = by + boxH + 3.f;
            dl->AddText(ImGui::GetFont(), fs, {tx+1,ty+1}, ImColor(0,0,0,200),      buf);
            dl->AddText(ImGui::GetFont(), fs, {tx,  ty  }, ImColor(220,220,220,255), buf);
        }

        
        if (s.espName && e.name[0] != '\0') {
            float fs = 11.f;
            ImVec2 tsz = ImGui::GetFont()->CalcTextSizeA(fs, FLT_MAX, 0.f, e.name);
            float tx = feetS.x - tsz.x * 0.5f, ty = by - tsz.y - 3.f;
            dl->AddText(ImGui::GetFont(), fs, {tx+1,ty+1}, ImColor(0,0,0,200), e.name);
            dl->AddText(ImGui::GetFont(), fs, {tx,  ty  }, boxCol,              e.name);
        }

        
        if (s.espSkel && e.hasBones) {
            for (int p = 0; p < SKEL_PAIR_COUNT; p++) {
                const Vector3& ba = e.bones[SKEL_PAIRS[p][0]];
                const Vector3& bb = e.bones[SKEL_PAIRS[p][1]];

                
                if (ba.x == 0.f && ba.y == 0.f && ba.z == 0.f) continue;
                if (bb.x == 0.f && bb.y == 0.f && bb.z == 0.f) continue;

                Vector3 sa = _WorldToScreen(ba, m, ScreenSize);
                Vector3 sb = _WorldToScreen(bb, m, ScreenSize);
                if (sa.z <= 0.f || sb.z <= 0.f) continue;

                
                float mx = ScreenSize.x * 0.1f;
                float my = ScreenSize.y * 0.1f;
                if (sa.x < -mx || sa.x > ScreenSize.x + mx) continue;
                if (sa.y < -my || sa.y > ScreenSize.y + my) continue;
                if (sb.x < -mx || sb.x > ScreenSize.x + mx) continue;
                if (sb.y < -my || sb.y > ScreenSize.y + my) continue;

                dl->AddLine({sa.x+1,sa.y+1},{sb.x+1,sb.y+1}, ImColor(0,0,0,160), 2.f);
                dl->AddLine({sa.x,  sa.y  },{sb.x,  sb.y  }, ImColor(217,252,255,220), 1.f);
            }
        }
    }
    g_debug.boxCount = boxCount;
}
