#include <unordered_map>
#include "../ThreeMaxLoader.h"
#include "Turtle.h"
#include "../profiling.h"
#include "../engine.h"
#include "../container.h"
#include "../sounds/sounds.h"

extern dWorldID world;
extern dSpaceID space;

extern std::unordered_map<std::string, GLuint> textures;
extern container<Vehicle*> entities;

Turtle::Turtle(int newfaction) : AdvancedWalrus(newfaction)
{

}


void Turtle::init()
{
    _model = (Model*)T3DSModel::loadModel(filereader("units/tank.3ds"),0,0,0,1,1,1,0);
    if (_model != NULL)
    {
        _topModel = (Model*)T3DSModel::loadModel(filereader("structures/turrettop.3ds"),0,0,0,0.1,0.1,0.1,0);
    }

    width=6.0f;
    height=3.0f;
    length=12.0f;

    viewport_height = 40.0;

    setForward(0,0,1);

    firingpos[1] = 2.3;
    firingpos[0]=firingpos[2]=0;

    aim = Vec3f(0,0,0);

    azimuth=0;
    elevation=0;
}

void Turtle::setNameByNumber(int number)
{
    setNumber(number);
    setName("Turtle", number);
}

void Turtle::drawModel(float yRot, float xRot, float x, float y, float z)
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
        CLog::Write(CLog::Debug,"Model is null.\n");
    }
}

void Turtle::embody(dWorldID world, dSpaceID space)
{
    me = dBodyCreate(world);
    embody(me);
    geom = dCreateBox( space, width, height, length);
    dGeomSetBody(geom, me);
}

void Turtle::embody(dBodyID myBodySelf)
{
    dMass m;

    float myMass = 60.0f;

    dBodySetPosition(myBodySelf, pos[0], pos[1], pos[2]);
    dMassSetBox(&m,1,width, height, length);
    dMassAdjust(&m, myMass*1.0f);
    dBodySetMass(myBodySelf,&m);

    me = myBodySelf;
}

dSpaceID Turtle::embody_in_space(dWorldID world, dSpaceID space)
{
    body_space = dSimpleSpaceCreate (space);
    dSpaceSetCleanup (body_space,1);

    embody(world, body_space);

    return body_space;
}

void Turtle::addWheels(Wheel *left, Wheel *right, Wheel *backleft, Wheel *backright)
{
    Turtle::left = left;
    Turtle::right = right;

    Turtle::backleft = backleft;
    Turtle::backright = backright;
}

void Turtle::getWheels(Wheel *&left, Wheel *&right, Wheel *&backleft, Wheel *&backright)
{
    left = Turtle::left;
    right = Turtle::right;
    backleft = Turtle::backleft;
    backright = Turtle::backright;
}


void Turtle::doControl(Controller controller)
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
        controller.registers.thrust = 0.0;
        AdvancedWalrus::doControl(controller);
    }
}

void Turtle::doDynamics()
{
    doDynamics(getBodyID());
}


int Turtle::getSubType()
{
    return VehicleSubTypes::TURTLE;
}

EntityTypeId Turtle::getTypeId()
{
    return EntityTypeId::TTurtle;
}

void Turtle::doDynamics(dBodyID body)
{
    if (status == SailingStatus::ROLLING)
    {
        doAmphibious(me);
        wrapDynamics(me);
    }
    else
    {
        // @NOTE: Turtles cannot move on water.
        setThrottle(0);
        left->setAzimuth(0);
        right->setAzimuth(0);
        left->setThrottle(0.0);
        right->setThrottle(0.0);
        backleft->setThrottle(0.0);
        backright->setThrottle(0.0);
        AdvancedWalrus::doDynamics(body);
    }

}

void Turtle::doControlAttack()
{
    Controller c;

    c.registers = registers;

    Vec3f Po = getPos();

    Vec3f Pf = destination;

    Vec3f T = Pf - Po;

    if (T.magnitude()>1000)
    {
        float distance = T.magnitude();

        Vec3f F = getForward();

        F = F.normalize();
        T = T.normalize();


        // Potential fields from the islands (to slow down Walrus)

        c.registers.thrust = 400.0f;

        if (distance<10000.0f)
        {
            c.registers.thrust = 200.0f;
        }

        if (distance<2000.0f)
        {
            c.registers.thrust = 100.0f;
        }

        BoxIsland *b = findNearestIsland(Po);
        float closest = (b->getPos() - Po).magnitude();
        if (closest > 1800 && closest < 1900)
        {
            //c.registers.thrust = 35.0f;                 // This is the thrust at which the walrus enter the island (more or less).

        }


        float e = _acos(  T.dot(F) );

        float signn = T.cross(F) [1];


        CLog::Write(CLog::Debug,"T: %10.3f %10.3f %10.3f %10.3f\n", closest, distance, e, signn);

        if (abs(e)>=0.5f)
        {
            c.registers.roll = 30.0 * (signn>0?+1:-1) ;
        } else
        if (abs(e)>=0.4f)
        {
            c.registers.roll = 20.0 * (signn>0?+1:-1) ;
        } else
        if (abs(e)>=0.2f)
            c.registers.roll = 10.0 * (signn>0?+1:-1) ;
        else {
            c.registers.roll = 0.0f;
        }


    } else
    if (T.magnitude()<1000)
    {
        Vec3f target = destination - getPos();
        Vec3f aim = toBody(me,target);

        dout << "Target:" << aim <<  ":Loc: " << getPos() << " Target: " << destination<< std::endl;


        float azimuth=getAzimuth(aim);
        float declination=getDeclination(aim);

        struct controlregister c;
        c = getControlRegisters();
        c.precesion = azimuth;
        c.pitch = declination;
        setControlRegisters(c);

        Vehicle *action = fire(0,world,space);

        if (action != NULL)
        {
            entities.push_at_the_back(action, action->getGeom());
            //gunshot();
            setTtl(100);
        }


    }

    BoxIsland *is = findNearestIsland(Po);

    Vec3f fw = getForward().normalize();

    Vec3f l = Po + 30*fw;

    float x,y;
    x = l[0];
    y = l[2];

    if (is->getHeight(x,y) < 5)
        // Go back and forth towards the target but moving back from the seashore.
        c.registers.thrust = -10;

    doControl(c);

}


void Turtle::doControlDestination()
{
    Controller c;

    c.registers = registers;


    Vec3f Po = getPos();

    Po[1] = 0.0f;

    Vec3f Pf = destination;

    Vec3f T = Pf - Po;

    float roundederror;

    roundederror = 5;


    if (dst_status != DestinationStatus::REACHED && T.magnitude()>roundederror)
    {
        float distance = T.magnitude();

        Vec3f F = getForward();

        F = F.normalize();
        T = T.normalize();


        // Potential fields from the islands (to slow down Walrus)

        c.registers.thrust = 600.0f;

        if (distance<600.0f)
        {
            c.registers.thrust = 20.0f;
        }


        float e = _acos(  T.dot(F) );

        float signn = T.cross(F) [1];


        c.registers.roll = abs(e) * (signn>0?+1:-1)  * 40;


    } else {
        if (dst_status != DestinationStatus::REACHED)
        {
            dst_status = DestinationStatus::REACHED;
            autostatus = AutoStatus::FREE;
            c.registers.thrust = 0.0f;
            setThrottle(0.0);
            c.registers.roll = 0.0f;
        }
    }

    BoxIsland *is = findNearestIsland(Po);

    Vec3f fw = getForward().normalize();

    Vec3f l = Po + 30*fw;

    float x,y;
    x = l[0];
    y = l[2];

    if (is->getHeight(x,y) < 5)
        // Go back and forth towards the target but moving back from the seashore.
        c.registers.thrust = -10;

    doControl(c);
}
