#include <unordered_map>
#include "Debris.h"

extern std::unordered_map<std::string, GLuint> textures;

Debris::Debris()
{
    Vehicle::setTtl(150);
}

void Debris::init()
{
    Debris::height=0.5f;
    Debris::length=0.5f;
    Debris::width=0.5f;

    Debris::mass = 0.01f;

    setDamage(5);

    setForward(0,0,1);
}

void Debris::drawModel(float yRot, float xRot, float x, float y, float z)
{
    float f[3];
    f[0] = 0; f[1] = 0; f[2] = 0;

    //Draw the saved model
    if (visible)
    {
        glPushMatrix();
        glTranslatef(x, y, z);

        glScalef(1.0f,1.0f,1.0f);

        //doTransform(f, R);

        Vec3f v = dBodyGetLinearVelVec(me);

        v = v*100;

        //drawArrow(v[0],v[1],v[2],1.0,0.0,0.0);
        //drawRedBox(Gunshot::width, Gunshot::height, Gunshot::length);
        drawTexturedBox(textures["land"],Gunshot::width, Gunshot::height, Gunshot::length);

        glPopMatrix();
    }
    //else
    //{
    //    printf ("model is null\n");
    //}
}

