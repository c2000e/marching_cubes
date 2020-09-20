#include "marching_cubes/signed_distance_functions.h"

SphereSDF::SphereSDF(glm::vec3 center, float radius) : c(center), r(radius) {}

// Equation from iqulezlez.org/www/articles/distfunctions/distfunctions.htm
float SphereSDF::distance(glm::vec3 p)
{
    return glm::length(p - c) - r;
} 

BoxSDF::BoxSDF(glm::vec3 bounds) : b(bounds) {}

// Equation from iqulezlez.org/www/articles/distfunctions/distfunctions.htm
float BoxSDF::distance(glm::vec3 p)
{
    glm::vec3 q = glm::abs(p) - b;
    return glm::length(glm::max(q, 0.0f)) +
        glm::min(glm::max(q.x, glm::max(q.y, q.z)), 0.0f);
}

TorusSDF::TorusSDF(glm::vec2 radii) : r(radii) {}

// Equation from iqulezlez.org/www/articles/distfunctions/distfunctions.htm
float TorusSDF::distance(glm::vec3 p)
{
    glm::vec2 q(glm::length(glm::vec2(p.x, p.z)) - r.x, p.y);
    return glm::length(q) - r.y;
}
