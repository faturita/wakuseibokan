#include <unordered_map>
#include "../ThreeMaxLoader.h"
#include "Medusa.h"

extern std::unordered_map<std::string, GLuint> textures;


Medusa::Medusa(int newfaction) : SimplifiedDynamicManta(newfaction)
{

}

void Medusa::init()
{
    //Load the model
    _model = (Model*)T3DSModel::loadModel("units/medusa.3ds",0,0,0,1,1,1,0);
    if (_model != NULL)
    {
        //_topModel = (Model*)T3DSModel::loadModel("structures/turrettop.3ds",0,0,0,0.1,0.1,0.1,0);
    }


    setForward(0,0,1);

    status = 0;
}


void Medusa::drawModel(float yRot, float xRot, float x, float y, float z)
{
    float f[3];
    f[0] = 0; f[1] = 0; f[2] = 0;

    //Draw the saved model
    if (_model != NULL)
    {
        glPushMatrix();
        glTranslatef(x, y, z);

        glScalef(2.0f,2.0f,2.0f);

        doTransform(f, R);

        //drawArrow();
        //drawArrow(S[0],S[1],S[2],1.0,0.0,0.0);

        // Draw linear velocity
        //drawArrow(V[0],V[1],V[2],0.0,1.0,0.0);

        //drawRectangularBox(16.0f/2.0f, 5.2f/2.0f, 8.0f/2.0f);

        glRotatef(180.0f, 0.0f, 0.0f, 1.0f);
        glRotatef(-180.0f, 1.0f, 0.0f, 0.0f);
        glRotatef(-180.0f, 0.0f, 1.0f, 0.0f);

        glColor3f(1.0,1.0f,1.0f);
        _model->setTexture(textures["metal"]);
        _model->draw();

        glPopMatrix();
    }
    else
    {
        printf ("model is null\n");
    }
}

int Medusa::getSubType()
{
    return MEDUSA;
}

EntityTypeId Medusa::getTypeId()
{
    return EntityTypeId::TMedusa;
}

void Medusa::setNameByNumber(int number)
{
    setNumber(number);
    setName("Medusa",number);
}
