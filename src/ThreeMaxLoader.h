// ThreeMaxLoader.h: interface for the CThreeMaxLoader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_THREEMAXLOADER_H__A3159AC5_B308_40E0_814C_5B537800FD30__INCLUDED_)
#define AFX_THREEMAXLOADER_H__A3159AC5_B308_40E0_814C_5B537800FD30__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#elif __linux
#include <GL/glut.h>
#endif
#include "model.h"


#define MAX_VERTICES 80000 // Max number of vertices (for each object)
#define MAX_POLYGONS 80000 // Max number of polygons (for each object)

// Our vertex type
typedef struct{
    float x,y,z;
}vertex_type;

// The polygon (triangle), 3 numbers that aim 3 vertices
typedef struct{
    int a,b,c;
}polygon_type;

// The object type
struct obj_type {
	char name[20];
    
	int vertices_qty=0;
    int polygons_qty=0;

    vertex_type vertex[MAX_VERTICES]; 
    polygon_type polygon[MAX_POLYGONS];

};


 int get_file_size(std::string filename) ;

 void draw3DSModel(obj_type object, float x, float y, float z, float scale);

 char Load3DS (obj_type *p_object, char *p_filename);

 int draw3DSModel(char *p_filename,float x, float y, float z, float scale,GLuint _textureMetal);

 obj_type load3DSModel(char *p_filename);

 int draw3DSModel(obj_type object,float x, float y, float z, float scale, GLuint _textureMetal);


 class T3DSModel : Model {
     private:
         GLuint texture;
         float scalex,scaley,scalez;
         float x,y,z;
         char filename[256];
         obj_type object;
         T3DSModel();
     public:
         ~T3DSModel();

         void setFilename(const char* p_filename);
         void setLocation(float x,float y,float z);
         void setScale(float scalex,float scaley, float scalez);
         void virtual setTexture(GLuint texture);
         void load3DSModel(const char* p_filename);
         void calculateCenterOfMass();

         //Switches to the given animation
         void setAnimation(const char* name);
         //Draws the current state of the animated model.
         void virtual draw();
         void virtual draw(GLuint);

         //Loads an MD2Model from the specified file.  Returns NULL if there was
         //an error loading it.
         static T3DSModel* loadModel(const char *p_filename,float x, float y, float z, float scale,GLuint texture);
         static T3DSModel* loadModel(const char *p_filename,float x, float y, float z, float scalex, float scaley, float scalez, GLuint texture);
 };

#endif // !defined(AFX_THREEMAXLOADER_H__A3159AC5_B308_40E0_814C_5B537800FD30__INCLUDED_)
