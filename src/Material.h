#ifndef MATERIAL_H
#define MATERIAL_H

#include <glm/glm.hpp>

class Material {
public:
    glm::vec3 ka, kd, ks;
    float s;

    Material(glm::vec3 a, glm::vec3 d, glm::vec3 s_in, float shininess)
        : ka(a), kd(d), ks(s_in), s(shininess) {
    }
};

#endif#pragma once
