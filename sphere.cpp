//
//  Framework for a raytracer
//  File: sphere.cpp
//
//  Created for the Computer Science course "Introduction Computer Graphics"
//  taught at the University of Groningen by Tobias Isenberg.
//
//  Authors:
//    Maarten Everts
//    Jasper van de Gronde
//
//  This framework is inspired by and uses code of the raytracer framework of 
//  Bert Freudenberg that can be found at
//  http://isgwww.cs.uni-magdeburg.de/graphik/lehre/cg2/projekt/rtprojekt.html 
//
#include "matrix.h"
#include "sphere.h"
#include "material.h"
#include <iostream>
#include <cassert>
#include <math.h>

/************************** Sphere **********************************/
    /****************************************************
    * RT1.1: INTERSECTION CALCULATION
    *
    * Given: ray, position, r
    * Sought: intersects? if true: *t
    * 
    * Insert calculation of ray/sphere intersection here. 
    *
    * You have the sphere's center (position) and radius (r) as well as
    * the ray's origin (ray.O) and direction (ray.D).
    *
    * If the ray does not intersect the sphere, return false.
    * Otherwise, return true and place the distance of the
    * intersection point from the ray origin in *t (see example).
    ****************************************************/
//Convert sphere coordinates to map coordinates[0,1]
Color Sphere::TextureMap(Point &p)
{
    double angleRadians = angle / 180.0f * M_PI;
    Matrix rotationMatrix(axis.normalized(), angleRadians);
    Vector d = rotationMatrix * (p - position);
    
    float theta = acos(d.z / r);
    float phi = atan2(d.y, d.x);

    double u = phi / (2 * M_PI);
    double v = theta / M_PI;
    return material->texture->colorAt(u, v);
}

Hit Sphere::intersect(const Ray &ray)
{
    double A = ray.D.dot(ray.D);
    double B = 2.0*(ray.O - position).dot(ray.D);
    double C = (ray.O - position).dot(ray.O - position) - r*r;
    
    double D = B*B - 4.0*A*C;
    if(D < 0.0)
      return Hit::NO_HIT();
      
    //Speed optimalisation, calculate one square root for both intersects
    double q;
    if(B < 0.0)
      q = (-B + sqrt(D))/2.0;
    else
      q = (-B - sqrt(D))/2.0;
    
    double t0 = q/A;
    double t1 = C/q;
    
    if(t0 < 0.0 && t1 < 0.0)
      return Hit::NO_HIT();
      
    if(t0 < 0.0 || t1 < 0.0)
    {
      double maxT = max(t0,t1);
      Vector N = (position - ray.at(maxT)).normalized();
      return Hit(maxT,N);
    }
    else
    {
      double minT = min(t0, t1);


      /****************************************************
      * RT1.2: NORMAL CALCULATION
      *
      * Given: t, position, r
      * Sought: N
      * 
      * Insert calculation of the sphere's normal at the intersection point.
      ****************************************************/

      Vector N = (ray.at(minT) - position).normalized();

      return Hit(minT,N);
    }
}
