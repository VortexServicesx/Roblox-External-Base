#include "visuals.h"
#include "../../../deps/imgui/imgui.h"
#include "../../utils/globals/globals.h"
#include "../../../deps/imgui/imgui_internal.h"


// yes we know esp is a bit shitty, but this is a base not an actual external
void draw_esp() {
    if (!cfg::esp || !g_vis.addr) return;
    
    ImDrawList* draw = ImGui::GetForegroundDrawList();
    vec2 dims = g_vis.dims();
    mat4 view = g_vis.view();
    
    static ImU32 box_col, name_col, health_col, rig_col;
    static float last_colors[12] = {0};
    bool colors_changed = false;
    for (int i = 0; i < 3; i++) {
        if (cfg::box_color[i] != last_colors[i] || cfg::name_color[i] != last_colors[i+3] || 
            cfg::health_color[i] != last_colors[i+6] || cfg::rig_color[i] != last_colors[i+9]) {
            colors_changed = true;
            break;
        }
    }
    if (colors_changed) {
        box_col = IM_COL32((int)(cfg::box_color[0] * 255), (int)(cfg::box_color[1] * 255), (int)(cfg::box_color[2] * 255), 255);
        name_col = IM_COL32((int)(cfg::name_color[0] * 255), (int)(cfg::name_color[1] * 255), (int)(cfg::name_color[2] * 255), 255);
        health_col = IM_COL32((int)(cfg::health_color[0] * 255), (int)(cfg::health_color[1] * 255), (int)(cfg::health_color[2] * 255), 255);
        rig_col = IM_COL32((int)(cfg::rig_color[0] * 255), (int)(cfg::rig_color[1] * 255), (int)(cfg::rig_color[2] * 255), 255);
        for (int i = 0; i < 3; i++) {
            last_colors[i] = cfg::box_color[i];
            last_colors[i+3] = cfg::name_color[i];
            last_colors[i+6] = cfg::health_color[i];
            last_colors[i+9] = cfg::rig_color[i];
        }
    }
    
    std::vector<ent> ents_copy;
    {
        std::lock_guard<std::mutex> lock(g_ents_mutex);
        ents_copy = g_ents;
    }
    
    for (auto& e : ents_copy) {
        if (!e.character.addr) continue;
        
        std::vector<const part_data*> parts;
        
        if (!e.r15_parts.empty()) {
            const char* names[] = { "Head", "UpperTorso", "LowerTorso", "LeftUpperLeg", "LeftLowerLeg",
                "LeftFoot", "RightUpperLeg", "RightLowerLeg", "RightFoot", "LeftUpperArm",
                "LeftLowerArm", "LeftHand", "RightUpperArm", "RightLowerArm", "RightHand" };
            for (auto& n : names) {
                auto it = e.r15_parts.find(n);
                if (it != e.r15_parts.end() && it->second.instance.addr) parts.push_back(&it->second);
            }
        } else if (!e.r6_parts.empty()) {
            const char* names[] = { "Head", "Torso", "Left Arm", "Right Arm", "Left Leg", "Right Leg" };
            for (auto& n : names) {
                auto it = e.r6_parts.find(n);
                if (it != e.r6_parts.end() && it->second.instance.addr) parts.push_back(&it->second);
            }
        }
        
        if (parts.empty()) continue;
        
        float l = FLT_MAX, t = FLT_MAX, r = -FLT_MAX, b = -FLT_MAX;
        bool ok = false;
        
        for (auto* p : parts) {
            vec3 pos = p->pos;
            vec3 sz = p->size;
            mat3 rot = p->rot;
            
            vec3 half_sz = sz * 0.5f;
            
            vec3 corners[8] = {
                {pos.x - rot.data[0] * half_sz.x - rot.data[1] * half_sz.y - rot.data[2] * half_sz.z,
                 pos.y - rot.data[3] * half_sz.x - rot.data[4] * half_sz.y - rot.data[5] * half_sz.z,
                 pos.z - rot.data[6] * half_sz.x - rot.data[7] * half_sz.y - rot.data[8] * half_sz.z},
                {pos.x + rot.data[0] * half_sz.x - rot.data[1] * half_sz.y - rot.data[2] * half_sz.z,
                 pos.y + rot.data[3] * half_sz.x - rot.data[4] * half_sz.y - rot.data[5] * half_sz.z,
                 pos.z + rot.data[6] * half_sz.x - rot.data[7] * half_sz.y - rot.data[8] * half_sz.z},
                {pos.x - rot.data[0] * half_sz.x + rot.data[1] * half_sz.y - rot.data[2] * half_sz.z,
                 pos.y - rot.data[3] * half_sz.x + rot.data[4] * half_sz.y - rot.data[5] * half_sz.z,
                 pos.z - rot.data[6] * half_sz.x + rot.data[7] * half_sz.y - rot.data[8] * half_sz.z},
                {pos.x + rot.data[0] * half_sz.x + rot.data[1] * half_sz.y - rot.data[2] * half_sz.z,
                 pos.y + rot.data[3] * half_sz.x + rot.data[4] * half_sz.y - rot.data[5] * half_sz.z,
                 pos.z + rot.data[6] * half_sz.x + rot.data[7] * half_sz.y - rot.data[8] * half_sz.z},
                {pos.x - rot.data[0] * half_sz.x - rot.data[1] * half_sz.y + rot.data[2] * half_sz.z,
                 pos.y - rot.data[3] * half_sz.x - rot.data[4] * half_sz.y + rot.data[5] * half_sz.z,
                 pos.z - rot.data[6] * half_sz.x - rot.data[7] * half_sz.y + rot.data[8] * half_sz.z},
                {pos.x + rot.data[0] * half_sz.x - rot.data[1] * half_sz.y + rot.data[2] * half_sz.z,
                 pos.y + rot.data[3] * half_sz.x - rot.data[4] * half_sz.y + rot.data[5] * half_sz.z,
                 pos.z + rot.data[6] * half_sz.x - rot.data[7] * half_sz.y + rot.data[8] * half_sz.z},
                {pos.x - rot.data[0] * half_sz.x + rot.data[1] * half_sz.y + rot.data[2] * half_sz.z,
                 pos.y - rot.data[3] * half_sz.x + rot.data[4] * half_sz.y + rot.data[5] * half_sz.z,
                 pos.z - rot.data[6] * half_sz.x + rot.data[7] * half_sz.y + rot.data[8] * half_sz.z},
                {pos.x + rot.data[0] * half_sz.x + rot.data[1] * half_sz.y + rot.data[2] * half_sz.z,
                 pos.y + rot.data[3] * half_sz.x + rot.data[4] * half_sz.y + rot.data[5] * half_sz.z,
                 pos.z + rot.data[6] * half_sz.x + rot.data[7] * half_sz.y + rot.data[8] * half_sz.z}
            };
            
            for (auto& c : corners) {
                vec2 scr = g_vis.w2s(c, dims, view);
                if (scr.x < 0 || scr.y < 0) continue;
                
                ok = true;
                if (scr.x < l) l = scr.x;
                if (scr.y < t) t = scr.y;
                if (scr.x > r) r = scr.x;
                if (scr.y > b) b = scr.y;
            }
        }
        
        if (!ok || l >= r || t >= b) continue;
        
        float w = (r - l) + 2.f;
        float h = (b - t) + 2.f;
        
        if (w > dims.x * 0.4f || h > dims.y * 0.6f) continue;
        
        if (cfg::box) { // real
            draw->AddRect({l - 1, t - 1}, {l - 1 + w, t - 1 + h}, IM_COL32(0, 0, 0, 255), 0, 0, 3);
            draw->AddRect({l - 1, t - 1}, {l - 1 + w, t - 1 + h}, box_col, 0, 0, 1);
        }
        
        if (cfg::health && e.max_hp > 0.f) {
            float hp_pct = e.hp / e.max_hp;
            if (hp_pct > 1.f) hp_pct = 1.f;
            if (hp_pct < 0.f) hp_pct = 0.f;

            if (cfg::health_position == 0) {
                // Left vertical bar (original behavior)
                float bar_w = 4.f;
                float bar_h = h;
                float bar_x = l - 1 - bar_w - 2.f;
                float bar_y = t - 1;

                draw->AddRectFilled({ bar_x, bar_y }, { bar_x + bar_w, bar_y + bar_h }, IM_COL32(0, 0, 0, 200));
                draw->AddRectFilled({ bar_x, bar_y + bar_h * (1.f - hp_pct) }, { bar_x + bar_w, bar_y + bar_h }, health_col);
                draw->AddRect({ bar_x, bar_y }, { bar_x + bar_w, bar_y + bar_h }, IM_COL32(0, 0, 0, 255));
            }
            else {
                // Bottom horizontal bar
                float bar_h = 4.f;
                float bar_w = w;
                float bar_x = l - 1;
                float bar_y = t - 1 + h + 2.f;

                draw->AddRectFilled({ bar_x, bar_y }, { bar_x + bar_w, bar_y + bar_h }, IM_COL32(0, 0, 0, 200));
                draw->AddRectFilled({ bar_x, bar_y }, { bar_x + bar_w * hp_pct, bar_y + bar_h }, health_col);
                draw->AddRect({ bar_x, bar_y }, { bar_x + bar_w, bar_y + bar_h }, IM_COL32(0, 0, 0, 255));
            }
        }
        
        if (cfg::rig_type) {
            const char* rig_txt = e.r15 ? "R15" : "R6";
            float rig_x = l - 1 + w + 4.f;
            float rig_y = t - 1;
            draw->AddText({rig_x + 1, rig_y + 1}, IM_COL32(0, 0, 0, 255), rig_txt);
            draw->AddText({rig_x, rig_y}, rig_col, rig_txt);
        }
        
        if (cfg::names) {
            const char* txt = cfg::name_type == 0 ? e.display.c_str() : e.name.c_str();
            ImVec2 txt_sz = ImGui::CalcTextSize(txt);
            float txt_x = l - 1 + (w - txt_sz.x) * 0.5f;
            float txt_y = t - 1 - txt_sz.y - 2;
            draw->AddText({txt_x + 1, txt_y + 1}, IM_COL32(0, 0, 0, 255), txt);
            draw->AddText({txt_x, txt_y}, name_col, txt);
        }
    }
}
