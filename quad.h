#ifndef QUAD_Q1
#define QUAD_Q1

#include "object.h"
#include "matrix.h"

class Quad : public Object
{
public:
    Quad(Point p1, Point p2, Point p3, Point p4) : p1(p1), p2(p2), p3(p3), p4(p4) {}

    virtual Hit intersect(const Ray &ray);
    virtual Triple TextureMap(Point &p);

    const Point p1, p2, p3, p4;
};

#endif /* end of include guard: SPHERE_H_115209AE */
