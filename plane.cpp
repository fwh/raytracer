//
//  Framework for a raytracer
//  File: plane.cpp
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

#include "plane.h"
#include <iostream>
#include <math.h>
Triple Plane::TextureMap(Point &p)
{
  return p;
}
Hit Plane::intersect(const Ray &ray)
{
  //Solution to At^2 + Bt + C = D
  double nom = (a-ray.O).dot(N);//negD - ray.O.dot(N);
  double denom = ray.D.dot(N);//(ray.D.dot(N));
  // Ray is either a clear miss or parallel to the plane. Either way no hit
  if (nom == 0.0 || denom == 0.0)
    return Hit::NO_HIT();
  else
    return Hit(nom/denom, N);
  
}
