#pragma once
#include <string>
#include <vector>
#include "../memory/mem.h"
#include "../utils/offsets/offsets.h"
#include "../utils/math/math.h"

class inst {
public:
    uintptr_t addr;
    inst() : addr(0) {}
    inst(uintptr_t a) : addr(a) {}
    
    std::string name() const;
    inst model() const;
    std::string classname() const;
    uintptr_t team();
    std::vector<inst> children();
    uintptr_t find_child(const std::string& n);
    uintptr_t find_class(const std::string& cn);
    uintptr_t parent();
    uint8_t rig();
    vec3 size() const;
    vec3 pos() const;
    mat3 rot() const;
    inst prim() const;
    vec2 dims();
    mat4 view();
    vec2 w2s(vec3 world, vec2 dims, mat4 view);
};
