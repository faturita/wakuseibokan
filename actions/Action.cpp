#include "Action.h"

Action::Action()
{

}

void Action::init()
{
    Action::height=2;
    Action::length=20;
    Action::width=1000;

    setForward(0,0,1);
}



void Action::drawModel(float yRot, float xRot, float x, float y, float z)
{
    float f[3];
    f[0] = 0; f[1] = 0; f[2] = 0;

    //Draw the saved model
    if (_model != NULL)
    {
        glPushMatrix();
        glTranslatef(x, y, z);

        glScalef(1.0f,1.0f,1.0f);

        drawArrow(100.0f,0.0f,0.0f,0.0,1.0,0.0);
        //drawRectangularBox(Structure::width, Structure::height, Structure::length);

        glPopMatrix();
    }
    else
    {
        printf ("model is null\n");
    }
}

