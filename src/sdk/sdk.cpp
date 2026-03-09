#include "sdk.h"

std::string inst::name() const {
    if (!addr) return "";
    uintptr_t ptr = g_mem->read<uintptr_t>(addr + offsets::Instance::Name);
    if (ptr) return g_mem->read_str(ptr);
    return "";
}

inst inst::model() const {
    return inst(g_mem->read<uintptr_t>(addr + offsets::Player::ModelInstance));
}

std::string inst::classname() const {
    uintptr_t desc = g_mem->read<uintptr_t>(addr + offsets::Instance::ClassDescriptor);
    uintptr_t name_ptr = g_mem->read<uintptr_t>(desc + offsets::Instance::ClassName);
    return g_mem->read_str(name_ptr);
}

uintptr_t inst::team() {
    return g_mem->read<uintptr_t>(addr + offsets::Player::Team);
}

std::vector<inst> inst::children() {
    std::vector<inst> result;
    if (!addr) return result;
    
    uintptr_t start = g_mem->read<uintptr_t>(addr + offsets::Instance::ChildrenStart);
    if (!start) return result;
    
    uintptr_t end = g_mem->read<uintptr_t>(start + offsets::Instance::ChildrenEnd);
    if (!end) return result;
    
    for (uintptr_t i = g_mem->read<uintptr_t>(start); i != end; i += 0x10) {
        uintptr_t child = g_mem->read<uintptr_t>(i);
        if (child) result.push_back(inst(child));
    }
    return result;
}

uintptr_t inst::find_child(const std::string& n) {
    if (!addr) return 0;
    for (const auto& c : children()) {
        if (c.name() == n) return c.addr;
    }
    return 0;
}

uintptr_t inst::find_class(const std::string& cn) {
    for (auto& c : children()) {
        if (c.classname() == cn) return c.addr;
    }
    return 0;
}

uintptr_t inst::parent() {
    return g_mem->read<uintptr_t>(addr + offsets::Instance::Parent);
}

uint8_t inst::rig() {
    return g_mem->read<uint8_t>(addr + offsets::Humanoid::RigType);
}

vec2 inst::dims() {
    return g_mem->read<vec2>(addr + offsets::VisualEngine::Dimensions);
}

mat4 inst::view() {
    return g_mem->read<mat4>(addr + offsets::VisualEngine::ViewMatrix);
}

vec2 inst::w2s(vec3 world, vec2 dims, mat4 view) {
    vec4 clip = {
        world.x * view.data[0] + world.y * view.data[1] + world.z * view.data[2] + view.data[3],
        world.x * view.data[4] + world.y * view.data[5] + world.z * view.data[6] + view.data[7],
        world.x * view.data[8] + world.y * view.data[9] + world.z * view.data[10] + view.data[11],
        world.x * view.data[12] + world.y * view.data[13] + world.z * view.data[14] + view.data[15]
    };
    
    if (clip.w <= 1e-6f) return { -1.f, -1.f };
    
    float iw = 1.f / clip.w;
    vec3 ndc = { clip.x * iw, clip.y * iw, clip.z * iw };
    
    return {
        (dims.x / 2.f) * (ndc.x + 1.f),
        (dims.y / 2.f) * (1.f - ndc.y)
    };
}

vec3 inst::pos() const {
    uintptr_t prim = g_mem->read<uintptr_t>(addr + offsets::BasePart::Primitive);
    return g_mem->read<vec3>(prim + offsets::Primitive::Position);
}

mat3 inst::rot() const {
    uintptr_t prim = g_mem->read<uintptr_t>(addr + offsets::BasePart::Primitive);
    return g_mem->read<mat3>(prim + offsets::Primitive::Rotation);
}

vec3 inst::size() const {
    uintptr_t prim = g_mem->read<uintptr_t>(addr + offsets::BasePart::Primitive);
    return g_mem->read<vec3>(prim + offsets::Primitive::Size);
}

inst inst::prim() const {
    return inst(g_mem->read<uintptr_t>(addr + offsets::BasePart::Primitive));
}
