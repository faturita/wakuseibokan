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
#include <GLUT/glut.h>


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
typedef struct {
	char name[20];
    
	int vertices_qty;
    int polygons_qty;

    vertex_type vertex[MAX_VERTICES]; 
    polygon_type polygon[MAX_POLYGONS];

} obj_type, *obj_type_ptr;


 int get_file_size(std::string filename) ;

 void draw3DSModel(obj_type object, float x, float y, float z, float scale);

 char Load3DS (obj_type_ptr p_object, char *p_filename);

 int draw3DSModel(char *p_filename,float x, float y, float z, float scale,GLuint _textureMetal);

#endif // !defined(AFX_THREEMAXLOADER_H__A3159AC5_B308_40E0_814C_5B537800FD30__INCLUDED_)
