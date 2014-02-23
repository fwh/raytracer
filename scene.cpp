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
#include "material.h"
#include <omp.h>
#include "camera.h"
#include <algorithm>
#include  <cstdlib>

Hit min_hit(std::numeric_limits<double>::infinity(),Vector());

Object* Scene::determineObject(const Ray &ray)
{
    min_hit = Hit(std::numeric_limits<double>::infinity(),Vector());
    Object *obj = NULL;
    for (unsigned int i = 0; i < m_Objects.size(); ++i) {
        Hit hit(m_Objects[i]->intersect(ray));
        //Allow some error in the hit because of rounding
        if (hit.t - min_hit.t < 0.0001 && hit.t > 1.0f) {
            min_hit = hit;
            obj = m_Objects[i];
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
    for(unsigned int i = 0; i < m_Lights.size(); ++i)
    {
      Vector L = (m_Lights[i]->position - hit).normalized(); 
      Color ambient = material->ka*m_Lights[i]->color;
      Ray r(hit, L);
      Object* possibleObject = determineObject(r);
      
      //Make sure we actually want shadows, and we hit something that is not ourselves on the way to the light
      if(m_RenderShadows && possibleObject != NULL && possibleObject != obj)
        c += ambient;
      else
      {
          Color diffuse = (m_Lights[i]->color * (max(0.0, N.dot(L))) * material->kd);
          Vector R = 2.0*(N.dot(L))*N - L;
          specular += material->ks * m_Lights[i]->color * pow(max(V.dot(R),0.0), material->n); 
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

Color Scene::renderGooch(const Ray& ray)
{ 
  // Find hit object and distance
  Object* obj = determineObject(ray);
  // No hit? Return background color.
  if (!obj) return Color(0.0, 0.0, 0.0);
 
  Material *material = obj->material;            //the hit objects material
  Point hit = ray.at(min_hit.t);                 //the hit point
  Vector N = min_hit.N;                          //the normal at hit point
  Vector V = -ray.D;                             //the view vector
  Color c;
  Color specular;

  for(unsigned int i = 0; i < m_Lights.size(); ++i)  
  {
    Vector L = (m_Lights[i]->position - hit).normalized(); 
    
    float NL = N.dot(L);
    
    //Create black outline. 0.23f was arbitrarily chosen
    if(N.dot(V) < 0.23f)
      return Color(0.0,0.0,0.0);
      
    Vector R = 2.0 * NL * N - L;
    Triple kd = m_Lights[i]->color * obj->material->color * obj->material->kd;
    Triple kCool = Color(0.0,0.0, m_GoochParameters.b) + m_GoochParameters.alpha*kd;
    Triple kWarm = Color(m_GoochParameters.y, m_GoochParameters.y, 0.0) + m_GoochParameters.beta*kd;
    Color gooch = kCool * (1.0 - NL)/2.0 + kWarm * (1.0 + NL)/2.0;
    specular += material->ks * m_Lights[i]->color * pow(max(V.dot(R),0.0), material->n); 
    c += gooch;
  }
  return c + specular;
}


Color Scene::trace(const Ray &ray, const int reflectionCount)
{

    switch(m_RenderMode)
    {
      case PHONG:
        return renderPhong(ray, reflectionCount);

      case ZBUFFER:
        return renderZBuffer(ray);

      case NORMAL:
        return renderNormal(ray);

      case GOOCH:
        return renderGooch(ray);

      default:
        return Color(0.0,0.0,0.0);;
    }
}

void Scene::render(Image &img)
{
    int w = img.width();
    int h = img.height();

    determineMinMaxZ();

    cout << m_Objects.size() << endl;

   // #pragma omp parallel for private(y, xSample, ySample, dx, dy, img) shared(x)
    for (int y = 0; y < h; y++) 
    {    
        for (int x = 0; x < w; x++) 
        {
            Color col;
            for(int xSample = 1; xSample <= m_SamplingFactor; ++xSample)
            {
                for(int ySample = 1; ySample <= m_SamplingFactor; ++ySample)
                {
                    float dx = xSample / (m_SamplingFactor + 1.0);
                    float dy = ySample / (m_SamplingFactor + 1.0);
                    Point pixel(m_Camera.m_Center + (x + dx - 0.5 * m_Camera.m_Width) * m_Camera.right()  + (y + dy - 0.5 * m_Camera.m_Heigth) * -m_Camera.m_Up);
                    Ray ray(m_Camera.m_Eye, (pixel - m_Camera.m_Eye).normalized());
                    col += trace(ray, 0);
                }
            }
            col /= (m_SamplingFactor * m_SamplingFactor);
            col.clamp();
            img(x,y) = col;
        }
    }
}

//Called before actual tracing so we can dynamically determine how deep the scene is for best results
void Scene::determineMinMaxZ()
{
    if(m_RenderMode == RenderMode(ZBUFFER))
    {
        double minHit = std::numeric_limits<double>::infinity();
        double maxHit = -std::numeric_limits<double>::infinity();

        for (int y = 0; y < m_Camera.m_Heigth; y++) 
        {
          for (int x = 0; x < m_Camera.m_Width; x++) 
          {
              Point pixel(m_Camera.m_Center + (x - 0.5 * m_Camera.m_Width) * m_Camera.right()  + (y - 0.5 * m_Camera.m_Heigth) * - m_Camera.m_Up);
              Ray ray(m_Camera.m_Eye, (pixel - m_Camera.m_Eye).normalized());
              for (unsigned int i = 0; i < m_Objects.size(); ++i) 
              {
                Hit hit(m_Objects[i]->intersect(ray));
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
