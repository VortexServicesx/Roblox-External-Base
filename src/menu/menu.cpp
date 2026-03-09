#define IMGUI_DEFINE_MATH_OPERATORS
#include "menu.h"
#include <dwmapi.h>
#include <d3d11.h>
#include <chrono>
#include <thread>
#include <timeapi.h>
#include "../../deps/imgui/imgui.h"
#include "../../deps/imgui/imgui_impl_win32.h"
#include "../../deps/imgui/imgui_impl_dx11.h"
#include "../utils/globals/globals.h"
#include "../utils/keybinds/keybinds.h"
#include "../hacks/visuals/visuals.h"
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "winmm.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

struct render_data {
    HWND wnd = nullptr;
    ID3D11Device* dev = nullptr;
    ID3D11DeviceContext* ctx = nullptr;
    IDXGISwapChain* swap = nullptr;
    ID3D11RenderTargetView* rtv = nullptr;
};

static render_data g_render;
static bool g_open = false;
static bool g_last_state = false;

LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wp, lp)) return true;
    
    switch (msg) {
    case WM_SIZE:
        if (g_render.dev && g_render.swap && wp != SIZE_MINIMIZED) {
            if (g_render.rtv) g_render.rtv->Release();
            g_render.swap->ResizeBuffers(0, LOWORD(lp), HIWORD(lp), DXGI_FORMAT_UNKNOWN, 0);
            ID3D11Texture2D* buf = nullptr;
            g_render.swap->GetBuffer(0, IID_PPV_ARGS(&buf));
            if (buf) {
                g_render.dev->CreateRenderTargetView(buf, nullptr, &g_render.rtv);
                buf->Release();
            }
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wp & 0xfff0) == SC_KEYMENU) return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

bool init_wnd() {
    WNDCLASSEXA wc{};
    wc.cbSize = sizeof(wc);
    wc.style = CS_CLASSDC;
    wc.lpfnWndProc = wnd_proc;
    wc.hInstance = GetModuleHandleA(nullptr);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = "wnd";
    
    if (!RegisterClassExA(&wc)) return false;
    
    g_render.wnd = CreateWindowExA(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
        wc.lpszClassName, "main", WS_POPUP,
        0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
        nullptr, nullptr, wc.hInstance, nullptr
    );
    
    if (!g_render.wnd) return false;
    
    SetLayeredWindowAttributes(g_render.wnd, RGB(0, 0, 0), 255, LWA_ALPHA);
    MARGINS m = { -1 };
    DwmExtendFrameIntoClientArea(g_render.wnd, &m);
    ShowWindow(g_render.wnd, SW_SHOW);
    UpdateWindow(g_render.wnd);
    
    return true;
}

bool init_d3d() {
    DEVMODEA dm{};
    dm.dmSize = sizeof(dm);
    EnumDisplaySettingsA(nullptr, ENUM_CURRENT_SETTINGS, &dm);
    int refresh_rate = dm.dmDisplayFrequency;
    
    if (refresh_rate < 100) refresh_rate = 0;
    
    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = refresh_rate;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = g_render.wnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    
    D3D_FEATURE_LEVEL lvl[] = { D3D_FEATURE_LEVEL_11_0 };
    D3D_FEATURE_LEVEL out_lvl;
    
    if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        lvl, 1, D3D11_SDK_VERSION, &sd, &g_render.swap, &g_render.dev, &out_lvl, &g_render.ctx))) {
        return false;
    }
    
    ID3D11Texture2D* buf = nullptr;
    g_render.swap->GetBuffer(0, IID_PPV_ARGS(&buf));
    if (!buf) return false;
    
    g_render.dev->CreateRenderTargetView(buf, nullptr, &g_render.rtv);
    buf->Release();
    
    return true;
}

bool init_imgui() {
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    ImGui::StyleColorsDark();
    return ImGui_ImplWin32_Init(g_render.wnd) && ImGui_ImplDX11_Init(g_render.dev, g_render.ctx);
}

void draw_menu() {
    static float fps = 0.f;
    static auto last = std::chrono::high_resolution_clock::now();
    static int frames = 0;
    frames++;
    auto now = std::chrono::high_resolution_clock::now();
    float delta = std::chrono::duration<float>(now - last).count();
    if (delta >= 1.f) {
        fps = frames / delta;
        frames = 0;
        last = now;
    }
    
    ImGui::Begin("menu");
    ImGui::Text("fps: %.1f", fps);
    ImGui::Text("game: %llu | players: %d", g_game_id, g_player_count);
    ImGui::Separator();
    
    if (ImGui::BeginTabBar("tabs")) {
        if (ImGui::BeginTabItem("esp")) {
            ImGui::Checkbox("Enabled", &cfg::esp);
            
            ImGui::Checkbox("Draw box", &cfg::box);
            ImGui::SameLine();
            ImGui::ColorEdit3("##boxcolor", cfg::box_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
            
            ImGui::Checkbox("Draw names", &cfg::names);
            ImGui::SameLine();
            ImGui::ColorEdit3("##namecolor", cfg::name_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(100);
            const char* items[] = { "Display name", "Username" };
            ImGui::Combo("##nametype", &cfg::name_type, items, 2);
            
            ImGui::Checkbox("Draw health", &cfg::health);
            ImGui::SameLine();
            ImGui::ColorEdit3("##healthcolor", cfg::health_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
            
            ImGui::Checkbox("Rig type", &cfg::rig_type);
            ImGui::SameLine();
            ImGui::ColorEdit3("##rigcolor", cfg::rig_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
            
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

void render_loop() {
    MSG msg{};
    
    timeBeginPeriod(1);
    
    DEVMODEA dm{};
    dm.dmSize = sizeof(dm);
    EnumDisplaySettingsA(nullptr, ENUM_CURRENT_SETTINGS, &dm);
    float target_fps = (float)dm.dmDisplayFrequency;
    
    if (target_fps < 100.f) target_fps = 180.f;
    
    const double frame_time_ms = 1000.0 / target_fps;
    auto last_frame = std::chrono::high_resolution_clock::now();
    
    static HWND cached_target = nullptr;
    static auto last_window_check = std::chrono::high_resolution_clock::now();
    static RECT last_rc = {0, 0, 0, 0};
    
    while (msg.message != WM_QUIT) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
        if (msg.message == WM_QUIT) break;
        
        if (key_pressed(VK_F1)) g_open = !g_open;
        
        auto now_check = std::chrono::high_resolution_clock::now();
        if (!cached_target || std::chrono::duration_cast<std::chrono::milliseconds>(now_check - last_window_check).count() > 500) {
            cached_target = FindWindowA(nullptr, "Roblox");
            last_window_check = now_check;
        }
        
        if (!cached_target || IsIconic(cached_target)) {
            MoveWindow(g_render.wnd, 0, 0, 0, 0, true);
            Sleep(16);
            continue;
        } else {
            RECT rc;
            if (GetClientRect(cached_target, &rc)) {
                if (rc.left != last_rc.left || rc.top != last_rc.top || 
                    rc.right != last_rc.right || rc.bottom != last_rc.bottom) {
                    POINT pt = { rc.left, rc.top };
                    ClientToScreen(cached_target, &pt);
                    MoveWindow(g_render.wnd, pt.x, pt.y, rc.right - rc.left, rc.bottom - rc.top, true);
                    last_rc = rc;
                }
            }
        }
        
        if (g_last_state != g_open) {
            if (g_open) {
                SetWindowLong(g_render.wnd, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW);
            } else {
                SetWindowLong(g_render.wnd, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW);
            }
            g_last_state = g_open;
        }
        
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        
        draw_esp();
        if (g_open) draw_menu();
        
        ImGui::Render();
        const float clr[4] = { 0, 0, 0, 0 };
        g_render.ctx->OMSetRenderTargets(1, &g_render.rtv, nullptr);
        g_render.ctx->ClearRenderTargetView(g_render.rtv, clr);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        g_render.swap->Present(0, 0);
        
        auto now = std::chrono::high_resolution_clock::now();
        double elapsed_ms = std::chrono::duration<double, std::milli>(now - last_frame).count();
        
        if (elapsed_ms < frame_time_ms) {
            double sleep_ms = frame_time_ms - elapsed_ms;
            if (sleep_ms > 1.0) {
                Sleep((DWORD)(sleep_ms - 1.0));
            }
            while (std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - last_frame).count() < frame_time_ms) {
            }
        }
        
        last_frame = std::chrono::high_resolution_clock::now();
    }
    
    timeEndPeriod(1);
}

bool init_render() {
    return init_wnd() && init_d3d() && init_imgui();
}
