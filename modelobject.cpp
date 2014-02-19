#include "triangle.h"
#include "material.h"
#include "modelobject.h"
#include "sphere.h"
#include <iostream>
#include <cassert>
#include <math.h>
Color ModelObject::TextureMap(Point &p)
{
    return p;
}

Vector convertVector(GLfloat const *v)
{
    return Vector(v[0], v[1], v[2]);
}

Hit ModelObject::intersect(const Ray &ray)
{
    double min = std::numeric_limits<double>::infinity();
    Hit minHit = Hit::NO_HIT();
    Sphere boundingSphere(Point(0.0,0.0,0.0), scale, 0, Vector());
    
    //Create a bounding sphere around it to reduce the number of calculations
    double boundingSphereDist = boundingSphere.intersect(ray).t;
    if(boundingSphereDist > 0.0)
    {
      for(unsigned int i = 0; i < model->numtriangles; ++i)
      {
          GLMtriangle t = model->triangles[i];
          Vector p1 = position + convertVector(&model->vertices[3*t.vindices[0]]) * scale;
          Vector p2 = position + convertVector(&model->vertices[3*t.vindices[1]]) * scale;
          Vector p3 = position + convertVector(&model->vertices[3*t.vindices[2]]) * scale;
          Triangle triangle(p1, p2, p3);
          Hit inter = triangle.intersect(ray);
          if(inter.t < min && inter.t > 0.0)
          {
              min = inter.t;
              minHit = inter;
          }
      }
      return minHit;
    }
    return Hit::NO_HIT();
}
