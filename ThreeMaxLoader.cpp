// ThreeMaxLoader.cpp: implementation of the CThreeMaxLoader class.
//
//////////////////////////////////////////////////////////////////////

#include "ThreeMaxLoader.h"

int get_file_size(std::string filename) // path to file
{
    FILE *p_file = NULL;
    p_file = fopen(filename.c_str(),"rb");
    fseek(p_file,0,SEEK_END);
    int size = ftell(p_file);
    fclose(p_file);
    return size;
};

void draw3DSModel(obj_type object, float x, float y, float z, float scale)
{
    int l_index;

    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(scale,scale,scale);
    glRotatef(90,1.0,0.0,0.0);
    glRotatef(180,1.0,0.0,0.0);


    glEnable(GL_TEXTURE_2D);
    //glBindTexture(GL_TEXTURE_2D, _textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_TRIANGLES); // glBegin and glEnd delimit the vertices that define a primitive (in our case triangles)

    printf("%d\n",object.polygons_qty);

    for (l_index=0;l_index<object.polygons_qty;l_index++)
    {
        //----------------- FIRST VERTEX -----------------
        // Coordinates of the first vertex
        glTexCoord2f(0.0f,0.0f);
        glVertex3f( object.vertex[ object.polygon[l_index].a ].x,
                   object.vertex[ object.polygon[l_index].a ].y,
                   object.vertex[ object.polygon[l_index].a ].z); //Vertex definition

        //----------------- SECOND VERTEX -----------------
        // Coordinates of the second vertex
        //float x= object.vertex[ object.polygon[l_index].b ].x;
        glTexCoord2f(1.0f,0.0f);
        glVertex3f( object.vertex[ object.polygon[l_index].b ].x,
                   object.vertex[ object.polygon[l_index].b ].y,
                   object.vertex[ object.polygon[l_index].b ].z);

        //----------------- THIRD VERTEX -----------------
        // Coordinates of the Third vertex
        glTexCoord2f(0.0f,1.0f);
        glVertex3f( object.vertex[ object.polygon[l_index].c ].x,
                   object.vertex[ object.polygon[l_index].c ].y,
                   object.vertex[ object.polygon[l_index].c ].z);
    }
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

char Load3DS (obj_type_ptr p_object, char *p_filename)
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


obj_type load3DSModel(const char *p_filename)
{
    obj_type object;
    obj_type *p_object;

    p_object = &object;

    int i; //Index variable

    FILE *l_file; //File pointer

    unsigned short l_chunk_id; //Chunk identifier
    unsigned int l_chunk_lenght; //Chunk lenght

    unsigned char l_char; //Char variable
    unsigned short l_qty; //Number of elements in each chunk

    unsigned short l_face_flags; //Flag that stores some face information

    if ((l_file=fopen (p_filename, "rb"))== NULL)
    {
        printf("Model not found: %s\n",p_filename);
        return object;
    } //Open the file

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

    return object;
}

int draw3DSModel(obj_type object,float x, float y, float z, float scalex, float scaley, float scalez, GLuint _texture)
{

    int l_index;

    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(scalex,scaley,scalez);
    glRotatef(90,1.0,0.0,0.0);
    glRotatef(180,1.0,0.0,0.0);


    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, _texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glColor3f(1.0f, 1.0f, 1.0f);

    Vec3f centerOfMass;

    glBegin(GL_TRIANGLES); // glBegin and glEnd delimit the vertices that define a primitive (in our case triangles)

    for (l_index=0;l_index<object.polygons_qty;l_index++)
    {

        //glColor3f((float)(l_index/2)/object.polygons_qty,(float)(l_index/2)/object.polygons_qty,(float)(l_index/2)/object.polygons_qty);

        if (l_index % 3 == 0)
            glColor3f(0.3f,0.3f,0.3f);
        else if (l_index % 3 == 1)
            glColor3f(0.5f,0.5f,0.5f);
        else
            glColor3f(0.2f,0.2f,0.2f);

        //----------------- FIRST VERTEX -----------------
        // Coordinates of the first vertex
        glTexCoord2f(0.0f,0.0f);
        glVertex3f( object.vertex[ object.polygon[l_index].a ].x,
                   object.vertex[ object.polygon[l_index].a ].y,
                   object.vertex[ object.polygon[l_index].a ].z); //Vertex definition

        //----------------- SECOND VERTEX -----------------
        // Coordinates of the second vertex
        //float x= object.vertex[ object.polygon[l_index].b ].x;
        glTexCoord2f(1.0f,0.0f);

        glVertex3f( object.vertex[ object.polygon[l_index].b ].x,
                   object.vertex[ object.polygon[l_index].b ].y,
                   object.vertex[ object.polygon[l_index].b ].z);

        //----------------- THIRD VERTEX -----------------
        // Coordinates of the Third vertex
        glTexCoord2f(0.0f,1.0f);
        glVertex3f( object.vertex[ object.polygon[l_index].c ].x,
                   object.vertex[ object.polygon[l_index].c ].y,
                   object.vertex[ object.polygon[l_index].c ].z);
    }
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

int draw3DSModel(obj_type object,float x, float y, float z, float scale, GLuint _texture)
{
    return draw3DSModel(object,x,y,z,scale,scale,scale,_texture);
}
void calculateCenterOfMass(obj_type &object)
{
    int l_index;
    Vec3f centerOfMass(0.0f,0.0f,0.0f);

    Vec3f min(object.vertex[ object.polygon[0].a ].x,object.vertex[ object.polygon[0].a ].y,object.vertex[ object.polygon[0].a ].z);
    Vec3f max(object.vertex[ object.polygon[0].a ].x,object.vertex[ object.polygon[0].a ].y,object.vertex[ object.polygon[0].a ].z);

    for (l_index=0;l_index<object.polygons_qty;l_index++)
    {
        centerOfMass[0] += object.vertex[ object.polygon[l_index].a ].x;
        centerOfMass[0] += object.vertex[ object.polygon[l_index].b ].x;
        centerOfMass[0] += object.vertex[ object.polygon[l_index].c ].x;

        centerOfMass[1] += object.vertex[ object.polygon[l_index].a ].y;
        centerOfMass[1] += object.vertex[ object.polygon[l_index].b ].y;
        centerOfMass[1] += object.vertex[ object.polygon[l_index].c ].y;

        centerOfMass[2] += object.vertex[ object.polygon[l_index].a ].z;
        centerOfMass[2] += object.vertex[ object.polygon[l_index].b ].z;
        centerOfMass[2] += object.vertex[ object.polygon[l_index].c ].z;
    }

    centerOfMass[0] = centerOfMass[0]/(3*((float)object.polygons_qty));
    centerOfMass[1] = centerOfMass[1]/(3*((float)object.polygons_qty));
    centerOfMass[2] = centerOfMass[2]/(3*((float)object.polygons_qty));

    printf("Offset: %10.5f\t%10.5f\t%10.5f\n",(centerOfMass[0]),(centerOfMass[1]),(centerOfMass[2]));

    for (l_index=0;l_index<object.polygons_qty;l_index++)
    {

        if (object.vertex[ object.polygon[l_index].a ].x<min[0])
            min[0]=object.vertex[ object.polygon[l_index].a ].x;
        if (object.vertex[ object.polygon[l_index].a ].y<min[1])
            min[1]=object.vertex[ object.polygon[l_index].a ].y;
        if (object.vertex[ object.polygon[l_index].a ].z<min[2])
            min[2]=object.vertex[ object.polygon[l_index].a ].z;

        if (object.vertex[ object.polygon[l_index].a ].x>max[0])
            max[0]=object.vertex[ object.polygon[l_index].a ].x;
        if (object.vertex[ object.polygon[l_index].a ].y>max[1])
            max[1]=object.vertex[ object.polygon[l_index].a ].y;
        if (object.vertex[ object.polygon[l_index].a ].z>max[2])
            max[2]=object.vertex[ object.polygon[l_index].a ].z;


        if (object.vertex[ object.polygon[l_index].b ].x<min[0])
            min[0]=object.vertex[ object.polygon[l_index].b ].x;
        if (object.vertex[ object.polygon[l_index].b ].y<min[1])
            min[1]=object.vertex[ object.polygon[l_index].b ].y;
        if (object.vertex[ object.polygon[l_index].b ].z<min[2])
            min[2]=object.vertex[ object.polygon[l_index].b ].z;

        if (object.vertex[ object.polygon[l_index].b ].x>max[0])
            max[0]=object.vertex[ object.polygon[l_index].b ].x;
        if (object.vertex[ object.polygon[l_index].b ].y>max[1])
            max[1]=object.vertex[ object.polygon[l_index].b ].y;
        if (object.vertex[ object.polygon[l_index].b ].z>max[2])
            max[2]=object.vertex[ object.polygon[l_index].b ].z;


        if (object.vertex[ object.polygon[l_index].c ].x<min[0])
            min[0]=object.vertex[ object.polygon[l_index].c ].x;
        if (object.vertex[ object.polygon[l_index].c ].y<min[1])
            min[1]=object.vertex[ object.polygon[l_index].c ].y;
        if (object.vertex[ object.polygon[l_index].c ].z<min[2])
            min[2]=object.vertex[ object.polygon[l_index].c ].z;

        if (object.vertex[ object.polygon[l_index].c ].x>max[0])
            max[0]=object.vertex[ object.polygon[l_index].c ].x;
        if (object.vertex[ object.polygon[l_index].c ].y>max[1])
            max[1]=object.vertex[ object.polygon[l_index].c ].y;
        if (object.vertex[ object.polygon[l_index].c ].z>max[2])
            max[2]=object.vertex[ object.polygon[l_index].c ].z;
    }

    printf("Dimensions: %10.5f\t%10.5f\t%10.5f\n",(max[0]-min[0]),(max[1]-min[1]),(max[2]-min[2]));

    printf("Geometrical offset: %10.5f\t%10.5f\t%10.5f\n",(max[0]+min[0])/2,(max[1]+min[1])/2,(max[2]+min[2])/2);
}

int draw3DSModel(char *p_filename,float x, float y, float z, float scale, GLuint _texture)
{
    obj_type object;
    obj_type *p_object;

    p_object = &object;

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

    int l_index;

    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(scale,scale,scale);
    glRotatef(90,1.0,0.0,0.0);
    glRotatef(180,1.0,0.0,0.0);


    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, _texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_TRIANGLES); // glBegin and glEnd delimit the vertices that define a primitive (in our case triangles)

    for (l_index=0;l_index<object.polygons_qty;l_index++)
    {

        //glColor3f((float)(l_index/2)/object.polygons_qty,(float)(l_index/2)/object.polygons_qty,(float)(l_index/2)/object.polygons_qty);

        if (l_index % 3 == 0)
            glColor3f(0.3f,0.3f,0.3f);
        else if (l_index % 3 == 1)
            glColor3f(0.5f,0.5f,0.5f);
        else
            glColor3f(0.2f,0.2f,0.2f);

        //----------------- FIRST VERTEX -----------------
        // Coordinates of the first vertex
        glTexCoord2f(0.0f,0.0f);
        glVertex3f( object.vertex[ object.polygon[l_index].a ].x,
                   object.vertex[ object.polygon[l_index].a ].y,
                   object.vertex[ object.polygon[l_index].a ].z); //Vertex definition

        //----------------- SECOND VERTEX -----------------
        // Coordinates of the second vertex
        //float x= object.vertex[ object.polygon[l_index].b ].x;
        glTexCoord2f(1.0f,0.0f);

        glVertex3f( object.vertex[ object.polygon[l_index].b ].x,
                   object.vertex[ object.polygon[l_index].b ].y,
                   object.vertex[ object.polygon[l_index].b ].z);

        //----------------- THIRD VERTEX -----------------
        // Coordinates of the Third vertex
        glTexCoord2f(0.0f,1.0f);
        glVertex3f( object.vertex[ object.polygon[l_index].c ].x,
                   object.vertex[ object.polygon[l_index].c ].y,
                   object.vertex[ object.polygon[l_index].c ].z);
    }
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}


T3DSModel::T3DSModel()
{

}
T3DSModel::~T3DSModel()
{


}

//Switches to the given animation
void T3DSModel::setAnimation(const char* name)
{
    assert(!"This method is not implemented for this model type.");
}
//Draws the current state of the animated model.
void T3DSModel::draw()
{
    //draw3DSModel(filename,x,y,z,scale,_texture);
    draw(T3DSModel::texture);
}
void T3DSModel::draw(GLuint _texture)
{
    draw3DSModel(T3DSModel::object,x,y,z,scalex,scaley,scalez,_texture);
}
void T3DSModel::setFilename(const char* p_filename)
{
    strcpy(filename,p_filename);
}
void T3DSModel::setLocation(float x,float y,float z)
{
    T3DSModel::x=x;
    T3DSModel::y=y;
    T3DSModel::z=z;
}
void T3DSModel::setScale(float scalex,float scaley,float scalez)
{
    T3DSModel::scalex=scalex;
    T3DSModel::scaley=scaley;
    T3DSModel::scalez=scalez;
}
void T3DSModel::setTexture(GLuint texture)
{
    T3DSModel::texture=texture;
}

void T3DSModel::setObject(obj_type object)
{
    T3DSModel::object = object;
    calculateCenterOfMass(T3DSModel::object);
}

//Loads an MD2Model from the specified file.
T3DSModel* T3DSModel::loadModel(const char *p_filename,float x, float y, float z, float scale,GLuint texture)
{
    return loadModel(p_filename,x,y,z,scale,scale,scale,texture);
}

/**
 * @brief T3DSModel::loadModel
 * @param p_filename
 * @param x
 * @param y
 * @param z
 * @param scalex
 * @param scaley
 * @param scalez
 * @param texture       Zero if you dont want any texture at all.
 * @return
 */
T3DSModel* T3DSModel::loadModel(const char *p_filename,float x, float y, float z, float scalex, float scaley, float scalez,GLuint texture)
{
    T3DSModel* td = new T3DSModel();
    printf("Model:%s\n",p_filename);
    td->setFilename(p_filename);
    td->setLocation(x,y,z);
    td->setScale(scalex,scaley,scalez);
    td->setTexture(texture);
    td->setObject(load3DSModel(p_filename));
    return td;
}

