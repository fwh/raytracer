#ifndef TRIANGLE_H_54123
#define TRIANGLE_H_54123

#include "object.h"

class Triangle : public Object
{
public:
    Triangle(Point a, Point b, Point c) : a(a), b(b), c(c) { }

    virtual Hit intersect(const Ray &ray);
    virtual Triple TextureMap(Point &p);
    const Point a, b, c;
};
#endif
