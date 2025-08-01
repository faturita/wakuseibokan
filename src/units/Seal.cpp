#include "Seal.h"
#include "../md2model.h"
#include "../actions/Gunshot.h"
#include "../engine.h"
#include "../container.h"
#include "../sounds/sounds.h"
#include "../profiling.h"

extern dWorldID world;
extern dSpaceID space;
extern container<Vehicle*> entities;
extern std::unordered_map<std::string, GLuint> textures;
extern std::vector<Message> messages;


Seal::Seal(int newfaction) : Walrus(newfaction)
{

}

void Seal::init()
{

    _model = (Model*)T3DSModel::loadModel(filereader("units/seal.3ds"),0.5,0.5,0.5,3,3,3,0);

    if (_model != NULL)
    {
        //_model->setAnimation("run");
        _topModel = (Model*)T3DSModel::loadModel(filereader("structures/turrettop.3ds"),0,0,0,0.3,0.3,0.3,0);
    }
    else
    	printf ("Model has been initialized");

    setForward(0,0,1);

    height=4.0f;
    width=5.0f;
    length=10.0f;

}

int Seal::getSubType()
{
    return SIMPLEWALRUS;
}

EntityTypeId Seal::getTypeId()
{
    return EntityTypeId::TWalrus;
}


void Seal::drawModel(float yRot, float xRot, float x, float y, float z)
{
    float f[3];
    f[0] = 0; f[1] = 0; f[2] = 0;

    //Draw the saved model
    if (_model != NULL)
    {
        glPushMatrix();
        glTranslatef(x, y, z);

        doTransform(f, R);
        
        //drawArrow();

       	//glRotatef(-180.0f, 1.0f, 0.0f, 0.0f);
        glRotatef(180.0f,0.0f,1.0f,0.0f);
        //glRotatef(yRot, 0.0f, 1.0f, 0.0f);

        //glRotatef(xRot, 1.0f, 0.0f, 0.0f);

        //Vehicle::doMaterial();
        //drawRectangularBox(width, height, length);

        _model->setTexture(textures["sky"]);
        //glTranslatef(0.0f,0.0f,-2.0f);
        _model->draw();

        glRotatef(90.0f,0.0f,1.0f,0.0f);
        _topModel->setTexture(textures["sky"]);
        _topModel->draw();

        glPopMatrix();
    }
    else
    {
    	printf ("model is null\n");
    }
}

void  Seal::getViewPort(Vec3f &Up, Vec3f &position, Vec3f &forward)
{
    position = getPos();
    forward = getForward();
    Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    Vec3f orig;


    forward = forward.normalize();
    orig = position;
    Up[0]=Up[2]=0;Up[1]=10;
    position = position - 50*forward + Up;
    forward = orig-position;
}