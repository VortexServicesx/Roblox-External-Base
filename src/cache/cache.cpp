#include "cache.h"
#include "../utils/globals/globals.h"
#include "../utils/offsets/offsets.h"
#include <thread>
#include <chrono>


// comment out debugging if u dont need it, will lower cpu a bit
void sync_ents(std::vector<ent>& old_list, std::vector<ent>& new_list) {
    if (new_list.empty()) {
        old_list.clear();
        return;
    }
    
    std::unordered_map<uintptr_t, ent> map;
    map.reserve(new_list.size());
    
    for (auto& e : new_list) {
        if (e.player.addr) map[e.player.addr] = e;
    }
    
    old_list.erase(std::remove_if(old_list.begin(), old_list.end(), [&](auto& e) {
        return map.find(e.player.addr) == map.end();
    }), old_list.end());
    
    for (auto& [addr, new_e] : map) {
        auto it = std::find_if(old_list.begin(), old_list.end(), [&](auto& e) {
            return e.player.addr == addr;
        });
        if (it != old_list.end()) {
            *it = new_e;
        } else {
            old_list.push_back(new_e);
        }
    }
}

void tp_check() {
    auto last_print = std::chrono::high_resolution_clock::now();
    
    while (true) {
        uintptr_t base = g_mem->get_base();
        if (!base) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            continue;
        }
        
        uintptr_t fake = g_mem->read<uintptr_t>(base + offsets::FakeDataModel::Pointer);
        if (!fake) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            continue;
        }
        
        uintptr_t curr = g_mem->read<uintptr_t>(fake + offsets::FakeDataModel::RealDataModel);
        if (!curr) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            continue;
        }
        
        if (g_dm.addr != curr) {
            printf("[tp] dm changed\n");
            g_dm = inst(curr);
            g_vis = inst(g_mem->read<uintptr_t>(base + offsets::VisualEngine::Pointer));
            
            uintptr_t ws_addr = g_dm.find_child("Workspace");
            if (ws_addr) g_ws = inst(ws_addr);
            
            g_game_id = g_mem->read<uintptr_t>(g_dm.addr + offsets::DataModel::GameId);
            printf("[tp] new dm: 0x%llx\n", curr);
            printf("[tp] game: %llu | players: %d\n", g_game_id, g_player_count);
            last_print = std::chrono::high_resolution_clock::now();
        }
        
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_print).count();
        if (elapsed >= 3) {
            printf("game: %llu | players: %d\n", g_game_id, g_player_count);
            last_print = now;
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void cache_loop() {
    while (true) {
        if (!g_dm.addr) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            continue;
        }
        
        std::vector<ent> temp;
        temp.reserve(32);
        
        uintptr_t players_addr = g_dm.find_child("Players");
        if (!players_addr) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            continue;
        }
        
        std::string local_username;
        uintptr_t local_player_addr = g_mem->read<uintptr_t>(players_addr + offsets::Player::LocalPlayer);
        if (local_player_addr) {
            inst local_player(local_player_addr);
            if (local_player.addr) {
                local_username = local_player.name();
            }
        }
        
        g_players = inst(players_addr).children();
        g_player_count = (int)g_players.size();
        
        for (inst& p : g_players) {
            if (!p.addr) continue;
            
            std::string player_name = p.name();
            if (player_name.empty()) continue;
            if (!local_username.empty() && player_name == local_username) {
                inst local_char = p.model();
                if (local_char.addr) {
                    for (inst& part : local_char.children()) {
                        if (!part.addr) continue;
                        if (part.name() == "HumanoidRootPart") {
                            uintptr_t prim = g_mem->read<uintptr_t>(part.addr + offsets::BasePart::Primitive);
                            if (prim) g_local_pos = g_mem->read<vec3>(prim + offsets::Primitive::Position);
                            break;
                        }
                    }
                }
                continue;
            }
            
            ent e;
            e.player = p;
            e.character = p.model();
            if (!e.character.addr) continue;
            
            e.parts = e.character.children();
            e.team = inst(p.team());
            e.name = player_name;
            
            uintptr_t hum_addr = e.character.find_child("Humanoid");
            if (hum_addr) {
                e.display = g_mem->read_str(hum_addr + offsets::Humanoid::DisplayName);
                if (e.display.empty()) e.display = e.name;
                
                e.hp = g_mem->read<float>(hum_addr + offsets::Humanoid::Health);
                e.max_hp = g_mem->read<float>(hum_addr + offsets::Humanoid::MaxHealth);
                if (e.max_hp <= 0.f) e.max_hp = 100.f;
            } else {
                e.display = e.name;
            }
            
            if (!hum_addr) continue;
            
            uint8_t rig = g_mem->read<uint8_t>(hum_addr + offsets::Humanoid::RigType);
            e.r15 = (rig == 1);
            
            for (inst& part : e.parts) {
                if (!part.addr) continue;
                std::string pname = part.name();
                if (pname.empty()) continue;
                
                part_data pd;
                pd.instance = part;
                pd.pos = part.pos();
                pd.size = part.size();
                pd.rot = part.rot();
                
                if (rig == 0) {
                    e.r6_parts[pname] = pd;
                } else if (rig == 1) {
                    e.r15_parts[pname] = pd;
                }
            }
            
            temp.push_back(e);
        }
        
        {
            std::lock_guard<std::mutex> lock(g_ents_mutex);
            sync_ents(g_ents, temp);
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(550));
    }
}
