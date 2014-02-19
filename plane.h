#ifndef PLANE_H_54123
#define PLANE_H_54123

#include "object.h"

class Plane : public Object
{
public:
    Plane(Point a, Vector N) : a(a), N(N.normalized()) { }

    virtual Hit intersect(const Ray &ray);
    virtual Triple TextureMap(Point &p);
    const Point a;
    const Vector N;
};
#endif
