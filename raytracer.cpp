//
//  Framework for a raytracer
//  File: raytracer.cpp
//
//  Created for the Computer Science course "Introduction Computer Graphics"
//  taught at the University of Groningen by Tobias Isenberg.
//
//  Author: Maarten Everts
//
//  This framework is inspired by and uses code of the raytracer framework of 
//  Bert Freudenberg that can be found at
//  http://isgwww.cs.uni-magdeburg.de/graphik/lehre/cg2/projekt/rtprojekt.html 
//

#include "raytracer.h"
#include "object.h"
#include "sphere.h"
#include "triangle.h"
#include "quad.h"
#include "plane.h"
#include "material.h"
#include "modelobject.h"
#include "light.h"
#include "image.h"
#include "yaml/yaml.h"
#include "glm.h"
#include <ctype.h>
#include <fstream>
#include <assert.h>

// Functions to ease reading from YAML input
void operator >> (const YAML::Node& node, Triple& t);
Triple parseTriple(const YAML::Node& node);

void operator >> (const YAML::Node& node, Triple& t)
{
    assert(node.size()==3);
    node[0] >> t.x;
    node[1] >> t.y;
    node[2] >> t.z;
}

Triple parseTriple(const YAML::Node& node)
{
    Triple t;
    node[0] >> t.x;
    node[1] >> t.y;
    node[2] >> t.z;	
    return t;
}

Material* Raytracer::parseMaterial(const YAML::Node& node)
{
    Material *m = new Material();
	if(node.FindValue("texture") != NULL){
	    std::string path;
	    node["texture"] >> path;
	    
		Image *texture = new Image(path.c_str());
		m->texture = texture;
	}
	else{
	    node["color"] >> m->color;	
	    m->texture = NULL;
	}
    node["ka"] >> m->ka;
    node["kd"] >> m->kd;
    node["ks"] >> m->ks;
    node["n"] >> m->n;
    return m;
}

GLMmodel* model;
Object* Raytracer::parseObject(const YAML::Node& node)
{
    Object *returnObject = NULL;
    std::string objectType;
    node["type"] >> objectType;
    if (objectType == "mesh")
    {
        char objectFile[200];
        std::string path;
        float scale;
        node["path"] >> path;
        node["scale"] >> scale;
        snprintf(objectFile, sizeof(char)*sizeof(objectFile), "%s.obj", path.c_str());
        model = glmReadOBJ(objectFile);
        glmUnitize(model);
        cout << "The model has " << model->numtriangles << " triangles" << endl;
        Point position(model->position[0], model->position[1], model->position[2]);
        ModelObject* mo = new ModelObject(model, position, scale);
        returnObject = mo;
    }
    if (objectType == "sphere") {
        Point pos;
        node["position"] >> pos;
        double r;
		double angle;
		Triple axis;
		if(node.FindValue("angle") != NULL)
		{
            node["radius"][0] >> r;
		    node["angle"] >> angle;
		    axis = parseTriple(node["radius"][1]);
		}
		else
		{
		    node["radius"] >> r;
		    angle = 0;
	    }
        Sphere *sphere = new Sphere(pos,r,angle,axis);		
        returnObject = sphere;
    }
    if(objectType == "triangle")
    {
        Point a,b,c;
        node["a"] >> a;
        node["b"] >> b;
        node["c"] >> c;
        Triangle *triangle = new Triangle(a, b, c);
        returnObject = triangle;
    }
    if(objectType == "plane") {
        Point a;
        Vector N;
        node["a"] >> a;
        node["N"] >> N;
        Plane *plane = new Plane(a, N);
        returnObject = plane;
    }
	if(objectType == "quad"){
		Point p1,p2,p3,p4;
		//p1 = parseTriple(node["p1"]);
		node["p1"] >> p1;
		node["p2"] >> p2;
		node["p3"] >> p3;
		node["p4"] >> p4;
		Quad *quad = new Quad(p1,p2,p3,p4);
		returnObject = quad;
	}
    if (returnObject) {
        // read the material and attach to object
        returnObject->material = parseMaterial(node["material"]);
    }
    return returnObject;
}

Light* Raytracer::parseLight(const YAML::Node& node)
{
    Point position;
    node["position"] >> position;		
    Color color;
    node["color"] >> color;
    return new Light(position,color);
}

/*
* Read a scene from file
*/

bool Raytracer::readScene(const std::string& inputFilename)
{
    // Initialize a new scene
    scene = new Scene();

    // Open file stream for reading and have the YAML module parse it
    std::ifstream fin(inputFilename.c_str());
    if (!fin) {
        cerr << "Error: unable to open " << inputFilename << " for reading." << endl;;
        return false;
    }
    try {
        YAML::Parser parser(fin);
        if (parser) {
            YAML::Node doc;
            parser.GetNextDocument(doc);

            // Read scene configuration options
            if(doc.FindValue("Camera") != NULL)
            {
              scene->setEye(parseTriple(doc["Camera"]["eye"]));
              scene->setCenter(parseTriple(doc["Camera"]["center"]));
              scene->setUp(parseTriple(doc["Camera"]["up"]));
              w = doc["Camera"]["viewSize"][0];
              h = doc["Camera"]["viewSize"][1];
            }
            else
            {
              Triple eye = parseTriple(doc["Eye"]);
              w = 400;
              h = 400;
              cout << "Using default eye: " << eye << " and image size of (" << w << ", " << h << ")" << endl;
              scene->setEye(eye);

            }
            if(doc.FindValue("RenderMode") != NULL)
              scene->setRenderMode(doc["RenderMode"]);
            else
            {
              cout << "Using default render mode: phong" << endl;
              scene->setRenderMode("phong");
            }

            if(doc.FindValue("GoochParameters") != NULL)
            {
              scene->setAlpha(doc["GoochParameters"]["alpha"]);
              scene->setBeta(doc["GoochParameters"]["beta"]); 
              scene->setY(doc["GoochParameters"]["y"]); 
              scene->setB(doc["GoochParameters"]["b"]);   
            }
            else
            {
              cout << "No Gooch parameters found. Using default values." << endl;
              scene->setAlpha(0.0f);
              scene->setBeta(0.0f);
              scene->setY(0.0f);
              scene->setB(0.0f);
            }
            if(doc.FindValue("Shadows") != NULL) 
              scene->setShadows(doc["Shadows"]);
            else
            {
              cout << "Using default shadow value: false" << endl;
              scene->setShadows(false);
            }
            if(doc.FindValue("MaxRecursionDepth") != NULL)
              scene->setMaxRecurseDepth(doc["MaxRecursionDepth"]);
            else
            {
              cout << "Using default recursion depth: 0" << endl;
              scene->setMaxRecurseDepth(0);
            }
            if(doc.FindValue("SuperSampling") != NULL)
              scene->setSamplingFactor(doc["SuperSampling"]["factor"]);
            else
            {
              cout << "Using default sampling factor: 1" << endl;
              scene->setSamplingFactor(1);
            }
            // Read and parse the scene objects
            const YAML::Node& sceneObjects = doc["Objects"];
            if (sceneObjects.GetType() != YAML::CT_SEQUENCE) {
                cerr << "Error: expected a sequence of objects." << endl;
                return false;
            }
            for(YAML::Iterator it=sceneObjects.begin();it!=sceneObjects.end();++it) {
                Object *obj = parseObject(*it);
                // Only add object if it is recognized
                if (obj) {
                    scene->addObject(obj);
                } else {
                    cerr << "Warning: found object of unknown type, ignored." << endl;
                }
            }
            
            // Read and parse light definitions
            const YAML::Node& sceneLights = doc["Lights"];
            if (sceneObjects.GetType() != YAML::CT_SEQUENCE) {
                cerr << "Error: expected a sequence of lights." << endl;
                return false;
            }
            for(YAML::Iterator it=sceneLights.begin();it!=sceneLights.end();++it) {
                scene->addLight(parseLight(*it));
            }
        }
        if (parser) {
            cerr << "Warning: unexpected YAML document, ignored." << endl;
        }
    } catch(YAML::ParserException& e) {
        std::cerr << "Error at line " << e.mark.line + 1 << ", col " << e.mark.column + 1 << ": " << e.msg << std::endl;
        return false;
    }

    cout << "YAML parsing results: " << scene->getNumObjects() << " objects read." << endl;
    return true;
}

void Raytracer::renderToFile(const std::string& outputFilename)
{
    Image img(w,h);
    cout << "Tracing image of (" << w << ", " << h << ")..." << endl;
    //In case of z buffer, determine minimum and maximum distance from eye first
    //if(scene->getRenderMode().compare("zbuffer") == 0)
    //    scene->determineMinMaxZ(img);
    scene->render(img);
    
    if(model != NULL) glmDelete(model);
    cout << "Writing image to " << outputFilename << "..." << endl;
    img.write_png(outputFilename.c_str());
    cout << "Done." << endl;
}
