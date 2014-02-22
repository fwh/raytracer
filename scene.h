//
//  Framework for a raytracer
//  File: scene.h
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

#ifndef SCENE_H_KNBLQLP6
#define SCENE_H_KNBLQLP6

#include <vector>
#include "triple.h"
#include "light.h"
#include "object.h"
#include "image.h"
#include "camera.h"

class Scene
{
private:
    enum RenderMode {
        PHONG,
        ZBUFFER,
        NORMAL,
        GOOCH
    };

    struct GoochParameters {
        double b;
        double y;
        double alpha;
        double beta;
    };
    double minZ, maxZ;
    //int maxRecurseDepth;
    //int samplingFactor;
    //int* viewSize;
    /*float b, y, alpha, beta;*/
    std::vector<Object*> objects;
    std::vector<Light*> lights;
    Camera camera;
    RenderMode renderMode;
    GoochParameters goochParameters;
    bool m_RenderShadows;
    unsigned int m_MaxRecursionDepth;
    unsigned int m_SamplingFactor;

public:
    Scene() : renderMode(PHONG), m_RenderShadows(false), m_MaxRecursionDepth(1), m_SamplingFactor(1){};
    //void setB(float B);
    //void setY(float Y);
    //void setAlpha(float Alpha);
    //void setBeta(float Beta);
    Color trace(const Ray &ray, const int recurseCount);
    void render(Image &img);
    void addObject(Object *o);
    void addLight(Light *l);
    unsigned int getNumObjects() { return objects.size(); }
    unsigned int getNumLights() { return lights.size(); }
    void setRenderMode(std::string s);
    void setShadows(bool s);
    Object* determineObject(const Ray &ray);
    void setMaxRecurseDepth(int recursionDepth);
    void setSamplingFactor(int samplingFactor);
    unsigned int getRenderMode();
    Color renderZBuffer(const Ray &ray);
    void determineMinMaxZ();
    //Color determineColor(int x, int y, int w, int h);
    void setCamera(const Camera& camera);
    const Camera& getCamera(){return camera;}
    Color renderPhong(const Ray &ray, const int recurseCount);
    Color renderNormal(const Ray &ray);
    void processPixel(Image &img, int x, int y);
};

#endif /* end of include guard: SCENE_H_KNBLQLP6 */
