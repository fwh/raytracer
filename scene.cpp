//
//  Framework for a raytracer
//  File: scene.cpp
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

#include "scene.h"
//#include "sphere.h"
#include "material.h"
#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include "camera.h"
#include  <cstdlib>

Hit min_hit(std::numeric_limits<double>::infinity(),Vector());

Object* Scene::determineObject(const Ray &ray)
{
    min_hit = Hit(std::numeric_limits<double>::infinity(),Vector());
    Object *obj = NULL;
    for (unsigned int i = 0; i < objects.size(); ++i) {
        Hit hit(objects[i]->intersect(ray));
        //Allow some error in the hit because of rounding
        if (hit.t - min_hit.t < 0.0001 && hit.t > 1.0f) {
            min_hit = hit;
            obj = objects[i];
        }
    }
    return obj;
}

Color Scene::renderPhong(const Ray &ray, const int reflectionCount)
{
    // Find hit object and distance
    Object* obj = determineObject(ray);
    // No hit? Return background color.
    if (!obj) return Color(0.0, 0.0, 0.0);

    if(obj == NULL)
        return Color(0.0,0.0,0.0);
    Material *material = obj->material;            //the hit objects material
    
    Point hit = ray.at(min_hit.t);      //the hit point
    Vector N = min_hit.N;                          //the normal at hit point
    Vector V = -ray.D;                             //the view vector
    Color c;
    Vector specular(0.0,0.0,0.0);
    
    //Texturemap if we have a texture
    if(material->texture != NULL)
        return obj->TextureMap(hit);
    
    //Diffuse, ambient, specular, shadows and reflection    
    for(unsigned int i = 0; i < lights.size(); ++i)
    {
      Vector L = (lights[i]->position - hit).normalized(); 
      Color ambient = material->ka*lights[i]->color;
      Ray r(hit, L);
      Object* possibleObject = determineObject(r);
      
      //Make sure we actually want shadows, and we hit something that is not ourselves on the way to the light
      if(m_RenderShadows && possibleObject != NULL && possibleObject != obj)
        c += ambient;
      else
      {
          Color diffuse = (lights[i]->color * (max(0.0, N.dot(L))) * material->kd);
          Vector R = 2.0*(N.dot(L))*N - L;
          specular += material->ks * lights[i]->color * pow(max(V.dot(R),0.0), material->n); 
          c += ambient + diffuse;
          
      }
    }
    
    Color returnCol = c * material->color;
    //Determine wheter we should reflec and take its specular color
    if(reflectionCount < m_MaxRecursionDepth)
    {
      Vector reflectDir = (ray.D - 2.0f*(ray.D.dot(N))*N);
      Ray reflect(hit, reflectDir);
      Object* newObj = determineObject(reflect);
      if(newObj)
        returnCol += renderPhong(reflect, reflectionCount+1) * material->ks;
    }
    return returnCol + specular;
}

Color Scene::renderZBuffer(const Ray &ray)
{    
  // Find hit object and distance
  Object* obj = determineObject(ray);
  // No hit? Return background color.
  if (!obj) return Color(0.0, 0.0, 0.0);

  double scaled = 1.0 - (min_hit.t - minZ) / (maxZ - minZ);
  return Color(scaled,scaled,scaled);
}

Color Scene::renderNormal(const Ray &ray)
{ 
  // Find hit object and distance
  Object* obj = determineObject(ray);
  // No hit? Return background color.
  if (!obj) return Color(0.0, 0.0, 0.0);

  return (min_hit.N  + 1.0) / 2.0;
}

Color Scene::trace(const Ray &ray, const int reflectionCount)
{

    switch(renderMode)
    {
      case PHONG:
        return renderPhong(ray, reflectionCount);

      case ZBUFFER:
        return renderZBuffer(ray);

      case NORMAL:
        return renderNormal(ray);
/*
      case GOOCH:
        return renderGooch();*/

      default:
        return Color(0.0,0.0,0.0);;
    }
    
    /*else if(renderMode.compare("gooch") == 0)
    {
        Material *material = obj->material;            //the hit objects material
        Point hit = ray.at(min_hit.t);                 //the hit point
        Vector N = min_hit.N;                          //the normal at hit point
        Vector V = -ray.D;                             //the view vector
        Color c;
        Color specular;
        
        for(unsigned int i = 0; i < lights.size(); ++i)  
        {
          Vector L = (lights[i]->position - hit).normalized(); 
          
          float NL = N.dot(L);
          
          //Create black outline. 0.23f was arbitrarily chosen
          if(N.dot(V) < 0.5f)
            return Color(0.0,0.0,0.0);
            
          Vector R = 2.0 * NL * N - L;
          Triple kd = lights[i]->color * obj->material->color * obj->material->kd;
          Triple kCool = Color(0.0,0.0,b) + alpha*kd;
          Triple kWarm = Color(y, y, 0.0) + beta*kd;
          Color gooch = kCool * (1.0 - NL)/2.0 + kWarm * (1.0 + NL)/2.0;
          specular += material->ks * lights[i]->color * pow(max(V.dot(R),0.0), material->n); 
          c += gooch;
        }
        return c + specular;
    }*/
}



void Scene::render(Image &img)
{
    int w = img.width();
    int h = img.height();

    determineMinMaxZ();

    cout << objects.size() << endl;

    int x,y;
    //#pragma omp parallel for private(y) shared(x)
    for (y = 0; y < h; y++) 
    {    
        for (x = 0; x < w; x++) 
        {
            Color col;
            for(int xSample = 1; xSample <= m_SamplingFactor; ++xSample)
            {
                for(int ySample = 1; ySample <= m_SamplingFactor; ++ySample)
                {
                    float dx = xSample / (m_SamplingFactor + 1.0);
                    float dy = ySample / (m_SamplingFactor + 1.0);
                    Point pixel(camera.m_Center + (x + dx - 0.5 * camera.m_Width) * camera.right()  + (y + dy - 0.5 * camera.m_Heigth) * -camera.m_Up);
                    Ray ray(camera.m_Eye, (pixel - camera.m_Eye).normalized());
                    col += trace(ray, 0);
                }
            }
            col /= (m_SamplingFactor * m_SamplingFactor);
            col.clamp();
            img(x,y) = col;
        }
    }
}

void Scene::addObject(Object *o)
{
    objects.push_back(o);
}

void Scene::addLight(Light *l)
{
    lights.push_back(l);
}

void Scene::setRenderMode(std::string s)
{
    if(s == "zbuffer")
      renderMode = ZBUFFER;
    else if(s == "normal")
      renderMode = NORMAL;
    else if(s == "gooch")
      renderMode = GOOCH;
    else
      renderMode = PHONG;
}

void Scene::setShadows(bool renderShadows)
{
    m_RenderShadows = renderShadows;
}

void Scene::setMaxRecurseDepth(int q)
{
    m_MaxRecursionDepth = q;
}

void Scene::setSamplingFactor(int s)
{
    m_SamplingFactor = s;
}

unsigned int Scene::getRenderMode()
{
    return renderMode;
}
/*
void Scene::setB(float B)
{
  b = B;
}
void Scene::setY(float Y)
{
  y = Y;
}
void Scene::setAlpha(float Alpha)
{
  alpha = Alpha;
}
void Scene::setBeta(float Beta)
{
  beta = Beta;
}
*/

//Calculate vectors which are needed for arbitrary viewing
void Scene::setCamera(const Camera& c)
{
  camera = c;
}

//Called before actual tracing so we can dynamically determine how deep the scene is for best results
void Scene::determineMinMaxZ()
{
    if(renderMode == RenderMode(ZBUFFER))
    {
        double minHit = std::numeric_limits<double>::infinity();
        double maxHit = -std::numeric_limits<double>::infinity();

        for (int y = 0; y < camera.m_Heigth; y++) 
        {
          for (int x = 0; x < camera.m_Width; x++) 
          {
              Point pixel(camera.m_Center + (x - 0.5 * camera.m_Width) * camera.right()  + (y - 0.5 * camera.m_Heigth) * -camera.m_Up);
              Ray ray(camera.m_Eye, (pixel - camera.m_Eye).normalized());
              for (unsigned int i = 0; i < objects.size(); ++i) 
              {
                Hit hit(objects[i]->intersect(ray));
                if (hit.t < minHit) 
                    minHit = hit.t;
                else if(hit.t > maxHit)
                    maxHit = hit.t;
              }
          }
       }
       minZ = minHit;
       maxZ = maxHit;
    }
}
