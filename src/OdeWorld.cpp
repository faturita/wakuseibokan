//
//  OdeWorld.cpp
//  wakuseibokan
//
//  Created by Rodrigo Ramele on 1/10/17.
//

#include "OdeWorld.h"

#include <GL/glut.h>

#include <ode/ode.h>

#include <vector>

#include "profiling.h"


#import <OpenAL/al.h>
#import <OpenAL/alc.h>


void doTransform (float pos[3], const float R[12])
{
    GLfloat matrix[16];
    matrix[0]=R[0];
    matrix[1]=R[4];
    matrix[2]=R[8];
    matrix[3]=0;
    matrix[4]=R[1];
    matrix[5]=R[5];
    matrix[6]=R[9];
    matrix[7]=0;
    matrix[8]=R[2];
    matrix[9]=R[6];
    matrix[10]=R[10];
    matrix[11]=0;
    matrix[12]=pos[0];
    matrix[13]=pos[1];
    matrix[14]=pos[2];
    matrix[15]=1;
    //glPushMatrix();
    glMultMatrixf (matrix);
}

struct MyObject
{
    dBodyID Body;
    dGeomID Geom[1];
};

dWorldID World;
dSpaceID Space;

dGeomID Ground;

dJointGroupID contactgroup;

#define DENSITY 1
#define MAX_CONTACTS 10


MyObject Object;

void InitWorfldModelling()
{
    dReal k;
    dMass m;
    
    /* create world */
    dInitODE();
    World = dWorldCreate();
    Space = dHashSpaceCreate (0);
    
    //dWorldSetAutoDisableFlag(World, 1);
    
    // The parameter needs to be zero.
    contactgroup = dJointGroupCreate (0);
    dWorldSetGravity (World,0,-9.81f,0);
    dWorldSetCFM (World,1e-5);
    
    // Set Damping
    dWorldSetLinearDamping(World, 0.01);  // 0.00001
    dWorldSetAngularDamping(World, 0.005);     // 0.005
    dWorldSetMaxAngularSpeed(World, 200);
    
    // Set and get the depth of the surface layer around all geometry objects. Contacts are allowed to sink into the surface layer up to the given depth before coming to rest. The default value is zero. Increasing this to some small value (e.g. 0.001) can help prevent jittering problems due to contacts being repeatedly made and broken.
    dWorldSetContactSurfaceLayer (World,0.001);
    
    Ground = dCreatePlane (Space,0,1,0,0);
    
    
    // create lift platform
    //platform = dCreateBox(space, 100, 10, 100);
    //dGeomSetPosition(platform, 0.0, 5, -300);
    
    
    //drawBoxIsland(300,5,300,1000,10);
    
    //dHeightfieldDataID heightid = dGeomHeightfieldDataCreate();
    
    // Create an finite heightfield.
    //dGeomHeightfieldDataBuildCallback( heightid, NULL, heightfield_callback,
    //                                  REAL( 1000.0 ), REAL (1000.0), HFIELD_WSTEP, HFIELD_DSTEP,
    //                                  REAL( 1.0 ), REAL( 0.0 ), REAL( 0.0 ), 0 );
    
    //dGeomHeightfieldDataSetBounds( heightid, REAL( -4.0 ), REAL( +6.0 ) );
    
    //gheight = dCreateHeightfield( space, heightid, 1 );
    
    //dGeomSetPosition( gheight, 300, 5, 300 );
    
    // Add this to destroy
    // dGeomHeightfieldDataDestroy( heightid );
    

    
    //initWorldPopulation();

}


void InitWorldModelling()
{
    dInitODE();
    World = dWorldCreate();
    
    Space = dSimpleSpaceCreate(0);
    
    contactgroup = dJointGroupCreate(0);
    
    dCreatePlane(Space, 0,1,0,0);
    
    dWorldSetGravity(World, 0,-1.0,0);
    
    dWorldSetERP(World, 0.2);
    dWorldSetCFM(World, 1e-5);
    
    dWorldSetContactMaxCorrectingVel(World, 0.9);
    
    dWorldSetContactSurfaceLayer(World, 0.001);
    
    dWorldSetAutoDisableFlag(World, 1);
    
    Object.Body = dBodyCreate(World);
    
    dBodySetPosition(Object.Body, 0, 3, 5);
    
    dBodySetLinearVel(Object.Body, 0.0, 0.0, 0.0);
    
    dMatrix3 R;
    
    dRFromAxisAndAngle(R,   dRandReal() * 2.0 - 1.0,
                       dRandReal() * 2.0 - 1.0,
                       dRandReal() * 2.0 - 1.0,
                       dRandReal() * 10.0 - 5.0);
    
    dBodySetRotation(Object.Body, R);
    
    size_t i=0;
    dBodySetData(Object.Body, (void*)i);
    
    dMass m;
    dReal sides[3];
    sides[0]=sides[1]=sides[2]=2.0;
    
    dMassSetBox(&m, DENSITY, sides[0], sides[1], sides[2]);
    
    dBodySetMass(Object.Body, &m);
    
    Object.Geom[0] = dCreateBox(Space, sides[0], sides[1], sides[2]);
    
    dGeomSetBody(Object.Geom[0], Object.Body);
    
}

void CloseODE()
{
    dJointGroupDestroy(contactgroup);
    
    dSpaceDestroy(Space);
    
    dWorldDestroy(World);
}


static void nearCallback(void *data, dGeomID o1, dGeomID o2)
{
    int i;
    
    dBodyID b1 = dGeomGetBody(o1);
    dBodyID b2 = dGeomGetBody(o2);
    
    dContact contact[MAX_CONTACTS];
    
    for (i=0;i<MAX_CONTACTS;i++)
    {
        contact[i].surface.mode = dContactBounce | dContactSoftCFM;
        contact[i].surface.mu = dInfinity;
        contact[i].surface.mu2 = 0;
        contact[i].surface.bounce = 0.01;
        contact[i].surface.bounce_vel = 0.1;
        contact[i].surface.soft_cfm = 0.01;
        
    }
    
    int numc=0;
    if (numc = dCollide(o1, o2, MAX_CONTACTS, &contact[0].geom, sizeof(dContact)))
    {
        for (i=0; i< numc; i++)
        {
            dJointID c = dJointCreateContact(World, contactgroup, contact+i);
            dJointAttach(c,b1,b2);
        }
    }
    
    
}

void drawBox(float xx, float yy, float zz)
{
    int x=0, y=0, z=0;
    
    static float boxangle = 0;
    
    const float BOX_SIZE = 7.0f; //The length of each side of the cube
    
    //glLoadIdentity();
    glPushMatrix();
    glTranslatef(xx,yy,zz);
    glRotatef(boxangle, 0.0f, 0.0f, 1.0f);
    glBegin(GL_QUADS);
    
    //Top face
    glColor3f(1.0f, 1.0f, 0.0f);
    glNormal3f(0.0, 1.0f, 0.0f);
    glVertex3f(-BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, -BOX_SIZE / 2 + z);
    glVertex3f(-BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
    glVertex3f(BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
    glVertex3f(BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, -BOX_SIZE / 2 + z);
    
    //Bottom face
    glColor3f(1.0f, 0.0f, 1.0f);
    glNormal3f(0.0, -1.0f, 0.0f);
    glVertex3f(-BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, -BOX_SIZE / 2 + z);
    glVertex3f(BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, -BOX_SIZE / 2 + z);
    glVertex3f(BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, BOX_SIZE /2 + z);
    glVertex3f(-BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
    
    //Left face
    glNormal3f(-1.0, 0.0f, 0.0f);
    glColor3f(0.0f, 1.0f, 1.0f);
    glVertex3f(-BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, -BOX_SIZE / 2 + z);
    glVertex3f(-BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
    glVertex3f(-BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, -BOX_SIZE / 2 + z);
    
    //Right face
    glNormal3f(1.0, 0.0f, 0.0f);
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, -BOX_SIZE / 2 + z);
    glVertex3f(BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, -BOX_SIZE / 2 + z);
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
    glVertex3f(BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
    
    glEnd();
    
    glEnable(GL_TEXTURE_2D);
    //glBindTexture(GL_TEXTURE_2D, _textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glColor3f(1.0f, 1.0f, 1.0f);
    
    glBegin(GL_QUADS);
    
    //Front face
    glNormal3f(0.0, 0.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
    
    //Back face
    glNormal3f(0.0, 0.0f, -1.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-BOX_SIZE / 2+ x, -BOX_SIZE / 2+ y, -BOX_SIZE / 2+ z);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(-BOX_SIZE / 2+ x, BOX_SIZE / 2+ y, -BOX_SIZE / 2+ z);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(BOX_SIZE / 2+ x, BOX_SIZE / 2+ y, -BOX_SIZE / 2+ z);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(BOX_SIZE / 2+ x, -BOX_SIZE / 2+ y, -BOX_SIZE / 2+ z);
    
    glEnd();
    glPopMatrix();
    
    boxangle += 0.5f;
    
    if (boxangle >= 360.f)
        boxangle = 0.0f;
}


void drawArrow(float x, float y, float z,float red, float green, float blue)
{
    glPushMatrix();
    glLineWidth(3.0f);
    
    // RED
    glTranslatef(0.0f,0.0f,0.0f);
    glColor3f(red,green,blue);
    glBegin(GL_LINES);
    glVertex3f(0.0f,0.0f,0.0f);
    glVertex3f(x,y,z);
    glEnd();
    
    glTranslatef(x,y,z);
    glRotatef(90.0f,0.0f,1.0f,0.0f);
    glutSolidCone(0.100,0.5f,10,10);
    
    glPopMatrix();
}


void drawArrow(float scale)
{
    glPushMatrix();
    
    // RED
    glTranslatef(0.0f,0.0f,0.0f);
    glColor3f(1.0f,0.0f,0.0f);
    glBegin(GL_LINES);
    glVertex3f(0.0f,0.0f,0.0f);
    glVertex3f(scale,0.0f,0.0f);
    glEnd();
    
    glTranslatef(scale,0.0f,0.0f);
    glRotatef(90.0f,0.0f,1.0f,0.0f);
    glutSolidCone(0.100,0.5f,10,10);
    
    glPopMatrix();
    
    // GREEN
    glPushMatrix();
    glTranslatef(0.0f,0.0f,0.0f);
    glColor3f(0.0f,1.0f,0.0f);
    glBegin(GL_LINES);
    glVertex3f(0.0f,0.0f,0.0f);
    glVertex3f(0.0f,scale,0.0f);
    glEnd();
    
    glTranslatef(0.0f,scale,0.0f);
    glRotatef(-90.0f,1.0f,0.0f,0.0f);
    glutSolidCone(0.100,0.5f,10,10);
    
    glPopMatrix();
    
    // BLUE
    glPushMatrix();
    glTranslatef(0.0f,0.0f,0.0f);
    glColor3f(0.0f,0.0f,1.0f);
    glBegin(GL_LINES);
    glVertex3f(0.0f,0.0f,0.0f);
    glVertex3f(0.0f,0.0f,scale);
    glEnd();
    
    glTranslatef(0.0f,0.0f,scale);
    glRotatef(-90.0f,0.0f,0.0f,1.0f);
    glutSolidCone(0.100,0.5f,10,10);
    
    glPopMatrix();
    
}


void DrawGeom (dGeomID g, const dReal *pos, const dReal *R, int show_aabb)

{
    
    // If the geom ID is missing then return immediately.
    
    if (!g)
        
        return;
    
    
    
    // If there was no position vector supplied then get the existing position.
    
    if (!pos)
        
        pos = dGeomGetPosition (g);
    
    
    
    // If there was no rotation matrix given then get the existing rotation.
    
    if (!R)
        
        R = dGeomGetRotation (g);
    
    
    
    // Get the geom's class type.
    
    int type = dGeomGetClass (g);
    
    
    
    
    
    if (type == dBoxClass)
        
    {
        
        // Create a temporary array of floats to hold the box dimensions.
        
        dReal sides[3];
        
        dGeomBoxGetLengths(g, sides);
        
        
        
        // Now to actually render the box we make a call to DrawBox, passing the geoms dimensions, position vector and
        
        // rotation matrix. And this function is the subject of our next discussion.
        
        //DrawBox(sides, pos, R);
        
    }
    
}




void drawScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    float xxx=-30, yyy=0.0, zzz=0;
    float forward[3];
    
    forward[0] = 1.0;
    forward[1] = 0.0;
    forward[2] = 0.0;
    

    
    float up[3];
    up[0]=0.0;
    up[1]=1.0;
    up[2]=0.0;
    
    gluLookAt(
              //Position
              xxx,
              yyy,
              zzz,
              
              //View 'direction'
              xxx+forward[0],
              yyy+forward[1],
              zzz+forward[2],
              
              //Upward vector
              up[0], up[1], up[2]);
    
    // Draw CENTER OF coordinates RGB=(x,y,b)
    glPushAttrib(GL_CURRENT_BIT);
    drawArrow(3);
    glPopAttrib();
    
    // Floor is changing color.
    glPushAttrib(GL_CURRENT_BIT);
    
    
    const dReal *pos = dGeomGetPosition (Object.Geom[0]);
    
    
    CLog::Write(CLog::Debug,"%10.8f, %10.8f, %10.8f\n",pos[0],pos[1],pos[2]);
    
    drawBox(pos[0],pos[1],pos[2]);
    //drawBox(0.0, 3.0, 5.0);
    
    glPopAttrib();
    
    
    glDisable(GL_TEXTURE_2D);
    
    glutSwapBuffers();
}


void initRendering() {
    // Lightning
    
    glEnable(GL_LIGHTING);
    
    // Lighting not working.
    glEnable(GL_LIGHT0);
    
    // Normalize the normals (this is very expensive).
    glEnable(GL_NORMALIZE);
    
    
    // Do not show hidden faces.
    glEnable(GL_DEPTH_TEST);
    
    
    // Enable wireframes
    //glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
    
    
    glShadeModel(GL_SMOOTH); // Type of shading for the polygons
    
    glEnable(GL_COLOR_MATERIAL);
    
    // Do not show the interior faces....
    //glEnable(GL_CULL_FACE);
    
    // Blue sky !!!
    glClearColor(0.7f, 0.9f, 1.0f, 1.0f);
    
    // Initialize scene textures.
    //initTextures();
    
}

void handleResize(int w, int h) {
    CLog::Write(CLog::Debug,"Handling Resize: %d, %d \n", w, h);
    glViewport(0, 0, w, h);
    
    // ADDED
    glOrtho( 0, w, 0, h, -1, 1);
    
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float)w / (float)h, 1.0, 10450.0f  /**+ yyy**/);
}


void update(int value)
{
    dSpaceCollide(Space, 0, &nearCallback);
    dWorldQuickStep(World, 0.05);
    dJointGroupEmpty(contactgroup);
    
    
    glutPostRedisplay();
    glutTimerFunc(25, update, 0);
}


void handleKeypress(unsigned char key, int x, int y) {
    switch (key) {
        case 27: //Escape key
            exit(1);
            break;
        default:break;

    }
}

void handleSpecKeypress(int key, int x, int y)
{
    //specialKey = glutGetModifiers();
    
    switch (key) {
        case GLUT_KEY_LEFT :
            ;break;
        case GLUT_KEY_RIGHT :
            ;break;
        case GLUT_KEY_UP :
            ;break;
        case GLUT_KEY_DOWN :
            ;break;
    }
    
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    
    glutCreateWindow("Wakuseibokan");
    
    if (argc>1 && strcmp(argv[1],"-d")==0)
        glutInitWindowSize(1200, 800);
    else
        glutFullScreen();
    
    
    //dAllocateODEDataForThread(dAllocateMaskAll);
    InitWorldModelling();
    
    CLog::Write(CLog::Debug,"World Modelling completed...\n");
    
    //Initialize all the models and structures.
    initRendering();
    
    
    // OpenGL callback functions.
    glutDisplayFunc(drawScene);
    glutKeyboardFunc(handleKeypress);
    glutSpecialFunc(handleSpecKeypress);
    
    // Resize callback function.
    glutReshapeFunc(handleResize);
    
    
    // this is the first time to call to update.
    glutTimerFunc(25, update, 0);
    
    CLog::Write(CLog::Debug,"Ready for main loop\n");
    
    // main loop, hang here.
    glutMainLoop();
    return 0;
}

