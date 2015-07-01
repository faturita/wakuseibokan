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


class CThreeMaxLoader  
{
public:
	CThreeMaxLoader();
	virtual ~CThreeMaxLoader();
    static int get_file_size(std::string filename) // path to file
    {
        FILE *p_file = NULL;
        p_file = fopen(filename.c_str(),"rb");
        fseek(p_file,0,SEEK_END);
        int size = ftell(p_file);
        fclose(p_file);
        return size;
    };
    
    static void draw3DSModel(obj_type object)
    {
        int l_index;
        
        glBegin(GL_TRIANGLES); // glBegin and glEnd delimit the vertices that define a primitive (in our case triangles)
        //glScalef(1.0f,1.0f,1.0f);
        for (l_index=0;l_index<object.polygons_qty;l_index++)
        {
            //----------------- FIRST VERTEX -----------------
            // Coordinates of the first vertex
            glVertex3f( object.vertex[ object.polygon[l_index].a ].x,
                       object.vertex[ object.polygon[l_index].a ].y,
                       object.vertex[ object.polygon[l_index].a ].z); //Vertex definition
            
            //----------------- SECOND VERTEX -----------------
            // Coordinates of the second vertex
            //float x= object.vertex[ object.polygon[l_index].b ].x;
            
            glVertex3f( object.vertex[ object.polygon[l_index].b ].x,
                       object.vertex[ object.polygon[l_index].b ].y,
                       object.vertex[ object.polygon[l_index].b ].z);
            
            //----------------- THIRD VERTEX -----------------
            // Coordinates of the Third vertex
            glVertex3f( object.vertex[ object.polygon[l_index].c ].x,
                       object.vertex[ object.polygon[l_index].c ].y,
                       object.vertex[ object.polygon[l_index].c ].z);
        }
        glEnd();
    }
    
	static char Load3DS (obj_type_ptr p_object, char *p_filename)
	{
		int i; //Index variable
		
		FILE *l_file; //File pointer
		
		unsigned short l_chunk_id; //Chunk identifier
		unsigned int l_chunk_lenght; //Chunk lenght

		unsigned char l_char; //Char variable
		unsigned short l_qty; //Number of elements in each chunk

		unsigned short l_face_flags; //Flag that stores some face information

		if ((l_file=fopen (p_filename, "rb"))== NULL) return 0; //Open the file

		while (ftell (l_file) < get_file_size (p_filename)) //Loop to scan the whole file
		{
			//getche(); //Insert this command for debug (to wait for keypress for each chuck reading)

			fread (&l_chunk_id, 2, 1, l_file); //Read the chunk header
			//printf("ChunkID: %x\n",l_chunk_id); 
			fread (&l_chunk_lenght, 4, 1, l_file); //Read the lenght of the chunk
			//printf("ChunkLenght: %x\n",l_chunk_lenght);

			switch (l_chunk_id)
			{
				//----------------- MAIN3DS -----------------
				// Description: Main chunk, contains all the other chunks
				// Chunk ID: 4d4d 
				// Chunk Lenght: 0 + sub chunks
				//-------------------------------------------
				case 0x4d4d: 
				break;    

				//----------------- EDIT3DS -----------------
				// Description: 3D Editor chunk, objects layout info 
				// Chunk ID: 3d3d (hex)
				// Chunk Lenght: 0 + sub chunks
				//-------------------------------------------
				case 0x3d3d:
				break;
				
				//--------------- EDIT_OBJECT ---------------
				// Description: Object block, info for each object
				// Chunk ID: 4000 (hex)
				// Chunk Lenght: len(object name) + sub chunks
				//-------------------------------------------
				case 0x4000: 
					i=0;
					do
					{
						fread (&l_char, 1, 1, l_file);
						p_object->name[i]=l_char;
						i++;
					}while(l_char != '\0' && i<20);
				break;

				//--------------- OBJ_TRIMESH ---------------
				// Description: Triangular mesh, contains chunks for 3d mesh info
				// Chunk ID: 4100 (hex)
				// Chunk Lenght: 0 + sub chunks
				//-------------------------------------------
				case 0x4100:
				break;
				
				//--------------- TRI_VERTEXL ---------------
				// Description: Vertices list
				// Chunk ID: 4110 (hex)
				// Chunk Lenght: 1 x unsigned short (number of vertices) 
				//             + 3 x float (vertex coordinates) x (number of vertices)
				//             + sub chunks
				//-------------------------------------------
				case 0x4110: 
					fread (&l_qty, sizeof (unsigned short), 1, l_file);
					p_object->vertices_qty = l_qty;
					//printf("Number of vertices: %d\n",l_qty);
					for (i=0; i<l_qty; i++)
					{

						fread (&p_object->vertex[i].x, sizeof(float), 1, l_file);
 						//printf("Vertices list x: %f\n",p_object->vertex[i].x);
						
						fread (&p_object->vertex[i].y, sizeof(float), 1, l_file);
 						//printf("Vertices list y: %f\n",p_object->vertex[i].y);
						
						fread (&p_object->vertex[i].z, sizeof(float), 1, l_file);
 						//printf("Vertices list z: %f\n",p_object->vertex[i].z);
						 
						//Insert into the database

					}
					break;

				//--------------- TRI_FACEL1 ----------------
				// Description: Polygons (faces) list
				// Chunk ID: 4120 (hex)
				// Chunk Lenght: 1 x unsigned short (number of polygons) 
				//             + 3 x unsigned short (polygon points) x (number of polygons)
				//             + sub chunks
				//-------------------------------------------
				case 0x4120:
					fread (&l_qty, sizeof (unsigned short), 1, l_file);
					p_object->polygons_qty = l_qty;
					//printf("Number of polygons: %d\n",l_qty); 
					for (i=0; i<l_qty; i++)
					{
						fread (&p_object->polygon[i].a, sizeof (unsigned short), 1, l_file);
						//printf("Polygon point a: %d\n",p_object->polygon[i].a);
						fread (&p_object->polygon[i].b, sizeof (unsigned short), 1, l_file);
						//printf("Polygon point b: %d\n",p_object->polygon[i].b);
						fread (&p_object->polygon[i].c, sizeof (unsigned short), 1, l_file);
						//printf("Polygon point c: %d\n",p_object->polygon[i].c);
						fread (&l_face_flags, sizeof (unsigned short), 1, l_file);
						//printf("Face flags: %x\n",l_face_flags);
					}
					break;

				//------------- TRI_MAPPINGCOORS ------------
				// Description: Vertices list
				// Chunk ID: 4140 (hex)
				// Chunk Lenght: 1 x unsigned short (number of mapping points) 
				//             + 2 x float (mapping coordinates) x (number of mapping points)
				//             + sub chunks
				//-------------------------------------------
				//----------- Skip unknow chunks ------------
				//We need to skip all the chunks that currently we don't use
				//We use the chunk lenght information to set the file pointer
				//to the same level next chunk
				//-------------------------------------------
				default:
					 fseek(l_file, l_chunk_lenght-6, SEEK_CUR);
			} 
		}
		
		fclose (l_file); // Closes the file stream

		return (1); // Returns ok
	}

};


#endif // !defined(AFX_THREEMAXLOADER_H__A3159AC5_B308_40E0_814C_5B537800FD30__INCLUDED_)
