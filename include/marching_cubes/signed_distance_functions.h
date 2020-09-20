#ifndef SIGNED_DISTANCE_FUNCTIONS_H 
#define SIGNED_DISTANCE_FUNCTIONS_H 

#include "glm/glm.hpp"

/*
 * A common interface for signed distance functions to make them
 * interchangeable.
 */
class SignedDistanceFunction
{
    public:
        virtual float distance(glm::vec3 p) = 0;
};

/*
 * A signed distance function for a sphere.
 */
class SphereSDF : public SignedDistanceFunction
{
    public:
        SphereSDF(glm::vec3 center, float radius);
        float distance(glm::vec3 p);

    private:
        glm::vec3 c;
        float r;
};

/*
 * A signed distance function for a box.
 */
class BoxSDF : public SignedDistanceFunction
{
    public:
        BoxSDF(glm::vec3 bounds);
        float distance(glm::vec3 p);

    private:
        glm::vec3 b;
};

/*
 * A signed distance function for a torus.
 */
class TorusSDF : public SignedDistanceFunction
{
    public:
        TorusSDF(glm::vec2 radii);
        float distance(glm::vec3 p);

    private:
        glm::vec2 r;
};

#endif
