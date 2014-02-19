#ifndef MO_Q1
#define MO_Q1

#include "object.h"
#include "matrix.h"
#include "sphere.h"
#include "glm.h"

class ModelObject : public Object
{
public:
    ModelObject(GLMmodel* model, Point position, float scale) : model(model), position(position), scale(scale) {}

    virtual Hit intersect(const Ray &ray);
    virtual Triple TextureMap(Point &p);

    const GLMmodel* model;
    const Point position;
    const float scale;
};

#endif /* end of include guard: SPHERE_H_115209AE */
