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

    double minZ, maxZ;
    std::vector<Object*> m_Objects;
    std::vector<Light*> m_Lights;
    Camera m_Camera;
    bool m_RenderShadows;
    unsigned int m_MaxRecursionDepth;
    unsigned int m_SamplingFactor;

public:
    Scene() : m_RenderShadows(false), m_MaxRecursionDepth(1), m_SamplingFactor(1), m_RenderMode(PHONG){};    

    struct GoochParameters {
        double b;
        double y;
        double alpha;
        double beta;
    };

    enum RenderMode {
        PHONG,
        ZBUFFER,
        NORMAL,
        GOOCH
    };

    Color trace(const Ray &ray, const int recurseCount);
    Object* determineObject(const Ray &ray);
    void render(Image &img);

    void addObject(Object *o) { m_Objects.push_back(o); }
    void addLight(Light *l)   { m_Lights.push_back(l); }

    unsigned int const getNumObjects() { return m_Objects.size(); }
    unsigned int const getNumLights()  { return m_Lights.size(); }
    unsigned int const getRenderMode() { return m_RenderMode; }
    const Camera&      getCamera()     { return m_Camera; }

    void setRenderMode(unsigned int s)                  { m_RenderMode = (RenderMode)s; }
    void setShadows(bool s)                             { m_RenderShadows = s; }
    void setMaxRecurseDepth(int recursionDepth)         { m_MaxRecursionDepth = recursionDepth; }
    void setSamplingFactor(int samplingFactor)          { m_SamplingFactor = samplingFactor; }
    void setGoochParameters(GoochParameters parameters) { m_GoochParameters = parameters; }
    void setCamera(const Camera& camera)                { m_Camera = camera; }

    Color renderPhong(const Ray &ray, const int recurseCount);
    Color renderNormal(const Ray &ray);
    Color renderGooch(const Ray &ray);
    Color renderZBuffer(const Ray &ray);

    void determineMinMaxZ();
private:
    GoochParameters m_GoochParameters;
    RenderMode m_RenderMode;
};

#endif /* end of include guard: SCENE_H_KNBLQLP6 */
