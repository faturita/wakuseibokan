#include <unordered_map>
#include "../ThreeMaxLoader.h"
#include "Otter.h"
#include "../profiling.h"

extern std::unordered_map<std::string, GLuint> textures;
extern container<Vehicle*> entities;

Otter::Otter(int newfaction) : AdvancedWalrus(newfaction)
{

}


void Otter::init()
{
    AdvancedWalrus::init();
    width=6.0f;
    height=3.0f;
    length=12.0f;

    viewport_height = 40.0;

    setForward(0,0,1);
}

void Otter::drawModel(float yRot, float xRot, float x, float y, float z)
{
    float f[3];
    f[0] = 0; f[1] = 0; f[2] = 0;

    //Draw the saved model
    if (_model != NULL)
    {
        glPushMatrix();
        glTranslatef(x, y, z);

        glScalef(1.5f,1.5f,1.5f);

        doTransform(f, R);

        //drawArrow();

        glRotatef(-180.0f, 1.0f, 0.0f, 0.0f);

        doMaterial();
        //drawRectangularBox(width/1, height/1, length/1);

        glRotatef(90.0, 0.0f, 1.0, 0.0f);

        _model->setTexture(textures["sky"]);
        _model->draw();


        // Move to the roof of the Walrus
        glTranslatef(0.0f,-2.3f,0.0f);

        glRotatef(270.0f, 0.0f, 1.0f, 0.0f);

        // Adjust azimuth and elevation which are used to aim the turret.
        glRotatef(azimuth,0.0f,1.0f,0.0f);
        glRotatef(-elevation,1.0f,0.0f,0.0f);

        // Rotate the turret cannon so that it is aligned properly.
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

void Otter::embody(dWorldID world, dSpaceID space)
{
    me = dBodyCreate(world);
    embody(me);
    geom = dCreateBox( space, width, height, length);
    dGeomSetBody(geom, me);
}

void Otter::embody(dBodyID myBodySelf)
{
    dMass m;

    float myMass = 20.0f;

    dBodySetPosition(myBodySelf, pos[0], pos[1], pos[2]);
    dMassSetBox(&m,1,width, height, length);
    dMassAdjust(&m, myMass*1.0f);
    dBodySetMass(myBodySelf,&m);

    me = myBodySelf;
}

dSpaceID Otter::embody_in_space(dWorldID world, dSpaceID space)
{
    body_space = dSimpleSpaceCreate (space);
    dSpaceSetCleanup (body_space,1);

    embody(world, body_space);

    return body_space;
}

void Otter::addWheels(Wheel *left, Wheel *right, Wheel *backleft, Wheel *backright)
{
    Otter::left = left;
    Otter::right = right;

    Otter::backleft = backleft;
    Otter::backright = backright;
}


void Otter::doControl(Controller controller)
{
    if (status == SailingStatus::ROLLING)
    {
        azimuth = controller.registers.precesion;
        elevation = controller.registers.pitch;

        setAim(toVectorInFixedSystem(0,0,1,azimuth, -elevation));

        left->setThrottle(controller.registers.thrust);
        right->setThrottle(controller.registers.thrust);
        backright->setThrottle(controller.registers.thrust);
        backleft->setThrottle(controller.registers.thrust);

        //if (controller.registers.thrust>0)
        {

            left->setAzimuth(controller.registers.roll/10.0);
            right->setAzimuth(controller.registers.roll/10.0);
        }

        backleft->setAzimuth(0);
        backright->setAzimuth(0);
    } else {
        AdvancedWalrus::doControl(controller);
    }
}

void Otter::doDynamics()
{
    doDynamics(getBodyID());
}


int Otter::getSubType()
{
    return VehicleSubTypes::OTTER;
}

EntityTypeId Otter::getTypeId()
{
    return EntityTypeId::TOtter;
}

void Otter::doDynamics(dBodyID body)
{
    if (status == SailingStatus::ROLLING)
    {
        doAmphibious(me);
        wrapDynamics(me);
    }
    else
    {
        left->setAzimuth(0);
        right->setAzimuth(0);
        left->setThrottle(0.0);
        right->setThrottle(0.0);
        backleft->setThrottle(0.0);
        backright->setThrottle(0.0);
        AdvancedWalrus::doDynamics(body);
    }

}
