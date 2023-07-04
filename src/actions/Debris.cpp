#include <unordered_map>
#include "Debris.h"

Debris::Debris()
{
    Vehicle::setTtl(150);
}

void Debris::init(Vec3f dimensions)
{
    Debris::height=dimensions[0];
    Debris::length=dimensions[1];
    Debris::width=dimensions[2];

    Debris::mass = 0.01f;

    setDamage(5);
    setName("Debris");

    setForward(0,0,1);
}

void Debris::init()
{
    init(Vec3f(0.5f,0.5f,0.5f));
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

        drawTexturedBox(texture,Gunshot::width, Gunshot::height, Gunshot::length);

        glPopMatrix();
    }
    //else
    //{
    //    printf ("model is null\n");
    //}
}

void Debris::setTexture(const GLuint &value)
{
    texture = value;
}


EntityTypeId Debris::getTypeId()
{
    return EntityTypeId::TDebris;
}

