#include "openglutils.h"
#include "hud.h"

void drawOverlyMark(int uc, int lc, int w, int h)
{
    glLineWidth(2.5);

    glBegin(GL_LINES);
    glVertex3f(uc-50,    lc-50+h, 0);
    glVertex3f(uc-50,    lc-50  , 0);
    glEnd();

    glBegin(GL_LINES);
    glVertex3f(uc-50   , lc-50, 0);
    glVertex3f(uc-50+w , lc-50, 0);
    glEnd();

    glBegin(GL_LINES);
    glVertex3f(uc-50, lc+50-h, 0);
    glVertex3f(uc-50, lc+50  , 0);
    glEnd();

    glBegin(GL_LINES);
    glVertex3f(uc-50  , lc+50 ,0);
    glVertex3f(uc-50+w, lc+50 ,0);
    glEnd();

    glBegin(GL_LINES);
    glVertex3f(uc+50-w, lc-50, 0);
    glVertex3f(uc+50  , lc-50, 0);
    glEnd();

    glBegin(GL_LINES);
    glVertex3f(uc+50, lc-50 ,0);
    glVertex3f(uc+50, lc-50+h, 0);
    glEnd();

    glBegin(GL_LINES);
    glVertex3f(uc+50, lc+50-h, 0);
    glVertex3f(uc+50,lc+50,0);
    glEnd();

    glBegin(GL_LINES);
    glVertex3f(uc+50, lc+50, 0);
    glVertex3f(uc+50-w, lc+50, 0);
    glEnd();
}

void drawCross(int uc, int cc)
{
    glBegin(GL_LINES);
    glVertex3f(uc-10, cc + 0, 0.0);
    glVertex3f(uc-2, cc + 0, 0);
    glEnd();

    glBegin(GL_LINES);
    glVertex3f(uc+0+2, cc + 0, 0.0);
    glVertex3f(uc+0+10, cc + 0, 0);
    glEnd();

    glBegin(GL_LINES);
    glVertex3f(uc+0, cc + 0-10, 0.0);
    glVertex3f(uc+0, cc + 0-2, 0);
    glEnd();

    glBegin(GL_LINES);
    glVertex3f(uc+0, cc + 0+10, 0.0);
    glVertex3f(uc+0, cc + 0+2, 0);
    glEnd();
}
