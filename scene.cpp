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
#include "sphere.h"
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

Color Scene::renderPhong(Object* obj, const Ray &ray, const int reflectionCount)
{
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
      if(makeShadows && possibleObject != NULL && possibleObject != obj)
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
    if(reflectionCount < maxRecurseDepth)
    {
      Vector reflectDir = (ray.D - 2.0f*(ray.D.dot(N))*N);
      Ray reflect(hit, reflectDir);
      Object* newObj = determineObject(reflect);
      returnCol += renderPhong(newObj, reflect, reflectionCount+1) * material->ks;
    }
    return returnCol + specular;
}

Color Scene::trace(const Ray &ray, const int reflectionCount)
{
    // Find hit object and distance
    Object* obj = determineObject(ray);
    // No hit? Return background color.
    if (!obj) return Color(0.0, 0.0, 0.0);
    
    //If we should render phong, call that function with the hit object, ray and the reflectionCount(defaults to 0 if we don't want to reflect)
    else if(renderMode.compare("phong") == 0)
    {  
        return renderPhong(obj, ray, reflectionCount);
    }
    
    //Do zbuffer, minZ and maxZ should be initialized by determineMinMaxZ
    else if(renderMode.compare("zbuffer") == 0)
    {
        double scaled = 1.0 - ((min_hit.t) - minZ) / (maxZ - minZ);
        return Color(scaled,scaled,scaled);
    }
    
    else if(renderMode.compare("gooch") == 0)
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
    }
    //Scale the normal vector of the hit between 0.0 and 1.0, That should give its color
    else if(renderMode.compare("normal") == 0)
    {
        return (min_hit.N  + 1.0) / 2.0;
    }
    //If we have an unspecified render mode we return the background color
    else return Color(0.0,0.0,0.0);
}

void Scene::render(Image &img)
{
    int w = img.width();
    int h = img.height();
     
    cout << objects.size() << endl;
    //Default values are arbitrarily chosen
    //setCamera(Triple(0.0,1.0,0.0), Triple(200, 200, 0));
    //Determine minimal and maximal value before we actually trace
    Camera* camera = new Camera(eye);
    if(renderMode.compare("zbuffer") == 0)    
        determineMinMaxZ(w, h);
        
  
    int x,y;
    //#pragma omp parallel for private(y,x)
    for (y = 0; y < h; y++) 
    {    
        for (x = 0; x < w; x++) 
        {
            Color col;
            for(int xSample = 1; xSample <= samplingFactor; ++xSample)
            {
                for(int ySample = 1; ySample <=samplingFactor; ++ySample)
                {
                    float dx = xSample / (samplingFactor + 1.0);
                    float dy = ySample / (samplingFactor + 1.0);
                    Point pixel(center + (x + dx - 0.5 * w) * camera->getHVec() + (y + dy - 0.5 * h) * camera->getVVec());
                    Ray ray(eye, (pixel - eye).normalized());
                    //#pragma omp critical
                    col += trace(ray, 0);
                }
            }
            col /= (samplingFactor * samplingFactor);
            col.clamp();
            img(x,y) = col;
        }
    }
}

Color Scene::determineColor(int x, int y, int w, int h)
{
    Color col(0.0,0.0,0.0);
    for(int xSample = 1; xSample <= samplingFactor; ++xSample)
    {
        for(int ySample = 1; ySample <=samplingFactor; ++ySample)
        {
            float dx, dy;
            dx = xSample / (samplingFactor + 1.0);
            dy = ySample / (samplingFactor + 1.0);
            Point pixel(center + (x + dx - 0.5 * w) * H + (y + dy - 0.5 * h) * V);
            Ray ray(eye, (pixel - eye).normalized());
            col += trace(ray, 0);
        }
    }
    col /= (samplingFactor * samplingFactor);
    col.clamp();
    return col;
}

void Scene::addObject(Object *o)
{
    objects.push_back(o);
}

void Scene::addLight(Light *l)
{
    lights.push_back(l);
}

void Scene::setEye(Triple e)
{
    eye = e;
}

void Scene::setCenter(Triple e)
{
    hasCamera = true;
    center = e;
}

void Scene::setUp(Triple e)
{
    up = e;
}

void Scene::setRenderMode(std::string s)
{
    renderMode = s;
}

void Scene::setShadows(bool b)
{
    makeShadows = b;
}

void Scene::setMaxRecurseDepth(int q)
{
    maxRecurseDepth = q;
}

void Scene::setSamplingFactor(int s)
{
    samplingFactor = s;
}

std::string Scene::getRenderMode()
{
    return renderMode;
}

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
/*
//Calculate vectors which are needed for arbitrary viewing
void Scene::setCamera(Vector const defaultUp, Vector const defaultCenter)
{
    if(!hasCamera)
    {
        up = defaultUp;
        center = defaultCenter;
    }
    Vector cameraDirection = (center-eye).normalized();
    Vector hVec = cameraDirection.cross(up).normalized();
    Vector vVec = cameraDirection.cross(hVec).normalized();
    H = hVec * up.length();
    V = vVec * up.length();
}
*/
//Called before actual tracing so we can dynamically determine how deep the scene is for best results
void Scene::determineMinMaxZ(int w, int h)
{
    if(renderMode.compare("zbuffer") == 0)
    {
        double minHit = std::numeric_limits<double>::infinity();
        double maxHit = -std::numeric_limits<double>::infinity();
        for (int y = 0; y < h; y++) 
        {
          for (int x = 0; x < w; x++) 
          {
              Point pixel = (center + (x - 0.5 * w) * H + (y - 0.5 * h) * V);
              Ray ray(eye, (pixel-eye).normalized());
              for (unsigned int i = 0; i < objects.size(); ++i) 
              {
                  Hit hit(objects[i]->intersect(ray));
                  if (hit.t<minHit) 
                      minHit = hit.t;
                  if(hit.t > maxHit)
                      maxHit = hit.t;
              }
          }
       }
       minZ = minHit;
       maxZ = maxHit;
    }
}
