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
#include "quad.h"
#include "material.h"
#include "triangle.h"
#include <iostream>
#include <cassert>
#include <math.h>
Color Quad::TextureMap(Point &p)
{
    return p;
}

//Create two triangles and see if either one of them intersects the ray
//Set the first point to lower left and continue clockwise
Hit Quad::intersect(const Ray &ray)
{
    Triangle t1(p1, p2, p3);
    Triangle t2(p1, p3, p4);
    Hit h1 = t1.intersect(ray);
    if(h1.t > 0.0)
        return h1;
        
    return t2.intersect(ray);
}
