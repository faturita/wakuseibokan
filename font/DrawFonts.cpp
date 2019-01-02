#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include <math.h>


#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif



typedef enum {
   MODE_BITMAP,
   MODE_STROKE
} mode_type;

static mode_type mode;
static int font_index;

void
print_bitmap_string(void* font, char* s)
{
   if (s && strlen(s)) {
      while (*s) {
         glutBitmapCharacter(font, *s);
         s++;
      }
   }
}

void
print_stroke_string(void* font, char* s)
{
   if (s && strlen(s)) {
      while (*s) {
         glutStrokeCharacter(font, *s);
         s++;
      }
   }
}
void
my_init()
{
   mode = MODE_STROKE;
   font_index = 0;
}

/**
void
draw_stuff()
{
   char string[8][256];
   unsigned int i, j;
   unsigned int count;
   void* bitmap_fonts[7] = {
      GLUT_BITMAP_9_BY_15,
      GLUT_BITMAP_8_BY_13,
      GLUT_BITMAP_TIMES_ROMAN_10,
      GLUT_BITMAP_TIMES_ROMAN_24,
      GLUT_BITMAP_HELVETICA_10,
      GLUT_BITMAP_HELVETICA_12,
      GLUT_BITMAP_HELVETICA_18
   };

   char* bitmap_font_names[7] = {
      "GLUT_BITMAP_9_BY_15",
      "GLUT_BITMAP_8_BY_13",
      "GLUT_BITMAP_TIMES_ROMAN_10",
      "GLUT_BITMAP_TIMES_ROMAN_24",
      "GLUT_BITMAP_HELVETICA_10",
      "GLUT_BITMAP_HELVETICA_12",
      "GLUT_BITMAP_HELVETICA_18"
   };

   void* stroke_fonts[2] = {
      GLUT_STROKE_ROMAN,
      GLUT_STROKE_MONO_ROMAN
   };

   char* stroke_font_names[2] = {
      "GLUT_STROKE_ROMAN",
      "GLUT_STROKE_MONO_ROMAN"
   };

   GLfloat x, y, ystep, yild, stroke_scale;

   // Set up some strings with the characters to draw.
   count = 0;
   for (i=1; i < 32; i++) { // Skip zero - it's the null terminator!
      string[0][count] = i;
      count++;
   }
   string[0][count] = '\0';

   count = 0;
   for (i=32; i < 64; i++) {
      string[1][count] = i;
      count++;
   }
   string[1][count] = '\0';

   count = 0;
   for (i=64; i < 96; i++) {
      string[2][count] = i;
      count++;
   }
   string[2][count] = '\0';

   count = 0;
   for (i=96; i < 128; i++) {
      string[3][count] = i;
      count++;
   }
   string[3][count] = '\0';

   count = 0;
   for (i=128; i < 160; i++) {
      string[4][count] = i;
      count++;
   }
   string[4][count] = '\0';

   count = 0;
   for (i=160; i < 192; i++) {
      string[5][count] = i;
      count++;
   }
   string[5][count] = '\0';

   count = 0;
   for (i=192; i < 224; i++) {
      string[6][count] = i;
      count++;
   }
   string[6][count] = '\0';

   count = 0;
   for (i=224; i < 256; i++) {
      string[7][count] = i;
      count++;
   }
   string[7][count] = '\0';


   // Draw the strings, according to the current mode and font.
   glColor4f(0.0, 1.0, 0.0, 0.0);
   x = -225.0;
   y = 70.0;
   ystep  = 100.0;
   yild   = 20.0;
   if (mode == MODE_BITMAP) {
      glRasterPos2f(-150, y+1.25*yild);
      print_bitmap_string(
         bitmap_fonts[font_index], bitmap_font_names[font_index]);
      for (j=0; j<7; j++) {
         glRasterPos2f(x, y);
         print_bitmap_string(bitmap_fonts[font_index], string[j]);
         y -= yild;
      }
   }
   else {
      stroke_scale = 0.1f;
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix(); {
         glTranslatef(x, y+1.25*yild, 0.0);
         glScalef(stroke_scale, stroke_scale, stroke_scale);
         print_stroke_string(
            stroke_fonts[font_index], stroke_font_names[font_index]);
      } glPopMatrix();
      glPushMatrix(); {
         glTranslatef(x, y, 0.0);
         for (j=0; j<4; j++) {
            glPushMatrix(); {
               glScalef(stroke_scale, stroke_scale, stroke_scale);
               print_stroke_string(stroke_fonts[font_index], string[j]);
            } glPopMatrix();
            glTranslatef(0.0, -yild, 0.0);
         }
         glTranslatef(0.0, -ystep, 0.0);
      } glPopMatrix();
   }
}**/


void drawString(GLfloat x, GLfloat y, GLfloat z, char *string, GLfloat stroke_scale, GLfloat r, GLfloat g, GLfloat b)
{
	/**
	void* bitmap_fonts[7] = {
	  GLUT_BITMAP_9_BY_15,
	  GLUT_BITMAP_8_BY_13,
	  GLUT_BITMAP_TIMES_ROMAN_10,
	  GLUT_BITMAP_TIMES_ROMAN_24,
	  GLUT_BITMAP_HELVETICA_10,
	  GLUT_BITMAP_HELVETICA_12,
	  GLUT_BITMAP_HELVETICA_18
	};

	char* bitmap_font_names[7] = {
	  "GLUT_BITMAP_9_BY_15",
	  "GLUT_BITMAP_8_BY_13",
	  "GLUT_BITMAP_TIMES_ROMAN_10",
	  "GLUT_BITMAP_TIMES_ROMAN_24",
	  "GLUT_BITMAP_HELVETICA_10",
	  "GLUT_BITMAP_HELVETICA_12",
	  "GLUT_BITMAP_HELVETICA_18"
	};

	char* stroke_font_names[2] = {
	  "GLUT_STROKE_ROMAN",
	  "GLUT_STROKE_MONO_ROMAN"
	};
	**/
	void* stroke_fonts[2] = {
	  GLUT_STROKE_ROMAN,
	  GLUT_STROKE_MONO_ROMAN
	};



	my_init();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix(); {
		glTranslatef(x, y, z);

        glColor3f(r,g,b);
		glScalef(stroke_scale, stroke_scale, stroke_scale);
		print_stroke_string(
		  stroke_fonts[0], string);
        glColor3f(1.0f,1.0f,1.0f);
	} glPopMatrix();

}


void drawString(GLfloat x, GLfloat y, GLfloat z, char *string, GLfloat stroke_scale)
{
    drawString(x,y,z,string,stroke_scale,1.0f,1.0f,1.0f);
}


