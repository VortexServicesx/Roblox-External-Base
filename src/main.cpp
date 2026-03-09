#include <iostream>
#include <thread>
#include "memory/mem.h"
#include "sdk/sdk.h"
#include "cache/cache.h"
#include "menu/menu.h"
#include "utils/globals/globals.h"
#include "utils/offsets/offsets.h"

int main() {
    g_mem = new mem();
    
    if (!g_mem->attach("RobloxPlayerBeta.exe")) {
        printf("attach failed\n");
        return 1;
    }
    
    uintptr_t base = g_mem->get_base();
    uintptr_t fake_dm = g_mem->read<uintptr_t>(base + offsets::FakeDataModel::Pointer);
    uintptr_t real_dm = g_mem->read<uintptr_t>(fake_dm + offsets::FakeDataModel::RealDataModel);
    
    g_dm = inst(real_dm);
    g_vis = inst(g_mem->read<uintptr_t>(base + offsets::VisualEngine::Pointer));
    g_ws = inst(g_dm.find_child("Workspace"));
    g_game_id = g_mem->read<uintptr_t>(g_dm.addr + offsets::DataModel::GameId);
    
    printf("base: 0x%llx\n", base);
    printf("datamodel: 0x%llx\n", g_dm.addr);
    printf("visualengine: 0x%llx\n", g_vis.addr);
    printf("workspace: 0x%llx\n", g_ws.addr);
    printf("pid: %d\n", GetProcessId(g_mem->handle));
    printf("game: %llu\n", g_game_id);
    
    std::thread(tp_check).detach();
    std::thread(cache_loop).detach();
    std::thread([] {
        if (init_render()) {
            render_loop();
        }
    }).detach();
    
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    delete g_mem;
    return 0;
}
