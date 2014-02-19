//
//  Framework for a raytracer
//  File: triangle.cpp
//
//  Created for the Computer Science course "Introduction Computer Graphics"
//  taught at the University of Groningen by Tobias Isenberg.
//
//  Authors:
//    Maarten Terpstra
//    Folkert de Vries
//
//  This framework is inspired by and uses code of the raytracer framework of 
//  Bert Freudenberg that can be found at
//  http://isgwww.cs.uni-magdeburg.de/graphik/lehre/cg2/projekt/rtprojekt.html 
//

#include "triangle.h"
#include <iostream>
#include <math.h>

Triple Triangle::TextureMap(Point &p)
{
  return p;
}

Hit Triangle::intersect(const Ray &ray)
{
    //calculate ray-plane intersection, retrieve point
    Vector e1 = (b-a);
    Vector e2 = (c-a);
    
    Vector h = ray.D.cross(e2);
    double x = e1.dot(h);
    if(x > -0.00001 && x < 0.00001)
      return Hit::NO_HIT();
    
    double f = 1.0/x;
    Vector s = ray.O - a;
    double u = f * s.dot(h);
    
    if(u < 0.0 || u > 1.0)
      return Hit::NO_HIT();
    
    Vector q = s.cross(e1);
    double v = f * ray.D.dot(q);
    
    if(v < 0.0 || u + v > 1.0)
      return Hit::NO_HIT();
      
    double t = f * e2.dot(q);
    if(t > 0.000001)
    {
      //Normal vector for the triangle
      Vector N = e1.cross(e2).normalized();
      return Hit(t, N);
    }
    else 
      return Hit::NO_HIT();
}
