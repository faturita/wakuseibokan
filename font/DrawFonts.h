/*
 * DrawFonts.h
 *
 *  Created on: Jan 10, 2011
 *      Author: faturita
 */

#ifndef DRAWFONTS_H_
#define DRAWFONTS_H_

#ifdef __linux
#include <GL/glut.h>
#elif __APPLE__
#include <GLUT/glut.h>
#endif

void drawString(float x, float y , float xz, char *string, GLfloat stroke_scale);

void drawString(GLfloat x, GLfloat y, GLfloat z, char *string, GLfloat stroke_scale, GLfloat r, GLfloat g, GLfloat b);

#endif /* DRAWFONTS_H_ */
