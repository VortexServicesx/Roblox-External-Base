#pragma once
#include <vector>
#include <memory>
#include <mutex>
#include "../../sdk/sdk.h"

struct part_data {
    inst instance;
    vec3 pos;
    vec3 size;
    mat3 rot;
};

struct ent {
    uintptr_t addr = 0;
    bool local = false;
    bool r15 = false;
    float hp = 100.f;
    float max_hp = 100.f;
    float dist;
    std::string name;
    std::string display;
    inst team;
    inst player;
    inst character;
    std::vector<inst> parts;
    std::unordered_map<std::string, part_data> r15_parts;
    std::unordered_map<std::string, part_data> r6_parts;
};

inline std::vector<ent> g_ents;
inline std::mutex g_ents_mutex;
inline inst g_dm;
inline inst g_ws;
inline inst g_vis;
inline std::vector<inst> g_players;
inline uintptr_t g_game_id = 0;
inline int g_player_count = 0;

namespace cfg {
    inline bool esp = true;
    inline bool box = true;
    inline bool names = true;
    inline bool health = true;
    inline bool rig_type = true;
    inline int name_type = 0;
    inline float box_color[3] = { 1.f, 1.f, 1.f };
    inline float name_color[3] = { 1.f, 1.f, 1.f };
    inline float health_color[3] = { 0.f, 1.f, 0.f };
    inline float rig_color[3] = { 1.f, 1.f, 1.f };
    inline int health_position = 0;
}
