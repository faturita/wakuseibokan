//
//  opengltemplate.cpp
//  wakuseibokan
//
//  Created by Rodrigo Ramele on 1/23/15.
//

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stdarg.h>
#include <math.h>

#include <GL/glut.h>

#include <vector>

#include "openglutils.h"

#include "profiling.h"


void processMouseEntry(int state) {

    
}


void processMouse(int button, int state, int x, int y) {
    
    

}



void processMouseActiveMotion(int x, int y) {

}




// Movement of the mouse alone.
void processMousePassiveMotion(int x, int y) {

}



void handleKeypress(unsigned char key, int x, int y) {

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



void drawScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    //drawLightning();
    
    //drawBox(0,0,-10);
    
    
    gluLookAt(
              //Position
              0,
              0,
              10,
              
              //View 'direction'
              0,
              0,
              -1,
              
              //Upward vector
              0, 1, 0);
    
    
    float x=1,y=0,z=0;
    
    drawArrow(3);
    
    //glLineWidth(2.5);
    //glColor3f(1.0, 0.0, 0.0);
    //glBegin(GL_LINES);
    //glVertex3f(0.0, 0.0, 0.0);
    //glVertex3f(15, 0, 0);
    //glEnd();
    
    //glPopMatrix();

    //glLineWidth(34.0f);
    
    // RED
//    glTranslatef(0.0f,0.0f,-5.0f);
//    glColor3f(1.0f,0.0f,0.0f);
//    glBegin(GL_LINE);
//     glVertex3f(0.0f,0.0f,0.0f);
//     glVertex3f(0.0,100.0,0.0);
//    glVertex3f(10.0,0.0,0.0);
//    glEnd();
//    
//    glFlush();
//    
//    glTranslatef(x,y,z);
//    glRotatef(90.0f,0.0f,1.0f,0.0f);
//    glutSolidCone(0.100,0.5f,10,10);
//    
//    glPopMatrix();
    
    
    
    
    glDisable(GL_TEXTURE_2D);
    
    glutSwapBuffers();
}


void initRendering() {
    // Lightning
    
    glEnable(GL_LIGHTING);
    
    glEnable(GL_LIGHT0);
    
    
    // Normalize the normals (this is very expensive).
    glEnable(GL_NORMALIZE);
    
    
    // Do not show hidden faces.
    glEnable(GL_DEPTH_TEST);
    
    glEnable(GL_COLOR_MATERIAL);
    
    // Do not show the interior faces....
    glEnable(GL_CULL_FACE);
    
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
    gluPerspective(45.0, (float)w / (float)h, 1.0, /*Camera.pos[2]+*/ 1000.0  /**+ yyy**/);
}


void update(int value)
{
    glutPostRedisplay();
    glutTimerFunc(25, update, 0);
}



int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    //glutInitWindowSize(1200, 800);
    //glutInitWindowPosition(100, 100);
    
    glutCreateWindow("OpenGLTester");
    //glutFullScreen();
    
    //Initialize all the models and structures.
    initRendering();
    
    
    // OpenGL callback functions.
    glutDisplayFunc(drawScene);
    glutKeyboardFunc(handleKeypress);
    glutSpecialFunc(handleSpecKeypress);
    
    // Resize callback function.
    glutReshapeFunc(handleResize);
    
    //adding here the mouse processing callbacks
    glutMouseFunc(processMouse);
    glutMotionFunc(processMouseActiveMotion);
    glutPassiveMotionFunc(processMousePassiveMotion);
    glutEntryFunc(processMouseEntry);
    
    // this is the first time to call to update.
    glutTimerFunc(25, update, 0);
    
    // main loop, hang here.
    glutMainLoop();
    return 0;
}

