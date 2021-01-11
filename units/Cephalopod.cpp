#include "../profiling.h"
#include "../sounds/sounds.h"
#include "../ThreeMaxLoader.h"
#include "../actions/Gunshot.h"
#include "Cephalopod.h"

Cephalopod::Cephalopod(int newfaction) : SimplifiedDynamicManta(newfaction)
{

}

void Cephalopod::init()
{
    //draw3DSModel("units/drone.3ds",1200.0+100,15.0,700.0+300.0,1,_textureBox);


    //Load the model
    _model = (Model*)T3DSModel::loadModel("units/drone.3ds",0,-13,0,0.4,1,1,0);
    if (_model != NULL)
    {
        //_topModel = (Model*)T3DSModel::loadModel("structures/turrettop.3ds",0,0,0,0.1,0.1,0.1,0);
    }


    Cephalopod::height=15.0f;
    Cephalopod::width=40.0f;
    Cephalopod::length=28.0f;


    setForward(0,0,1);

    status = 0;
}

void Cephalopod::embody(dWorldID world, dSpaceID space)
{
    me = dBodyCreate(world);
    embody(me);
    //geom = dCreateSphere( space, 2.64f);
    geom = dCreateBox( space, width, height, length);
    dGeomSetBody(geom, me);
}

void Cephalopod::embody(dBodyID myBodySelf)
{
    dMass m;

    float myMass = 20.0f;

    dBodySetPosition(myBodySelf, pos[0], pos[1], pos[2]);
    dMassSetBox(&m,1,width,height,length);
    dMassAdjust(&m, myMass*1.0f);
    dBodySetMass(myBodySelf,&m);

    me = myBodySelf;
}


void Cephalopod::drawModel(float yRot, float xRot, float x, float y, float z)
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

        //drawRectangularBox(width, height, length);

        glRotatef(180.0f, 0.0f, 0.0f, 1.0f);
        glRotatef(-180.0f, 1.0f, 0.0f, 0.0f);
        glRotatef(-180.0f, 0.0f, 1.0f, 0.0f);

        glColor3f(1.0,1.0f,1.0f);
        _model->setTexture(texture);
        _model->draw();

        glPopMatrix();
    }
    else
    {
        printf ("model is null\n");
    }
}

void  Cephalopod::getViewPort(Vec3f &Up, Vec3f &position, Vec3f &viewforward)
{
    position = getPos();
    viewforward = toVectorInFixedSystem(0,0,1,pan, tilt);                         // I dont care the declination for the viewport

    // ViewForward is in body coordinates, I need to convert it to global coordinates.
    viewforward = toWorld(me, viewforward);

    Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    Vec3f orig;


    viewforward = viewforward.normalize();
    orig = position;
    Up[0]=Up[2]=0;Up[1]=30;
    position = position - 90*viewforward + Up;
    viewforward = orig-position;
}


void Cephalopod::doControl(struct controlregister regs)
{
    Manta::doControl(regs);

    Cephalopod::pan = regs.yaw;
    Cephalopod::tilt = regs.bank;
}

int Cephalopod::getSubType()
{
    return CEPHALOPOD;
}

void Cephalopod::doDynamics()
{
    doDynamics(getBodyID());
}

void Cephalopod::doDynamics(dBodyID body)
{
    dReal *v = (dReal *)dBodyGetLinearVel(body);

    dVector3 O;
    dBodyGetRelPointPos( body, 0,0,0, O);

    dVector3 F;
    dBodyGetRelPointPos( body, 0,0,1, F);

    F[0] = (F[0]-O[0]);
    F[1] = (F[1]-O[1]);
    F[2] = (F[2]-O[2]);

    Vec3f vec3fF;
    vec3fF[0] = F[0];vec3fF[1] = F[1]; vec3fF[2] = F[2];

    Vec3f vec3fV;
    vec3fV[0]= v[0];vec3fV[1] = v[1]; vec3fV[2] = v[2];

    speed = vec3fV.magnitude();


    dVector3 result;
    dBodyVectorFromWorld(body, 0,1,0,result);

    Vec3f upInBody = Vec3f(result[0],result[1],result[2]);
    Vec3f Up = Vec3f(0.0f,1.0f,0.0f);

    upInBody = upInBody.normalize();

    //CLog::Write(CLog::Debug,"Angle between vectors %10.5f\n", acos(upInBody.dot(Up))*180.0/PI);

    float attitude = acos(upInBody.dot(Up))*180.0/PI;

    dout << "Attitude:" << attitude << std::endl;

    if (attitude>80 || attitude<-80)
    {
        // Walrus has tumbled.
        //damage(1);
    }

    if (speed > 10 && speed <32 )
    {
        droneflying();

    }

    if (VERIFY(pos,me) && !Vehicle::inert)
    {
        if (attitude < 45)
        {
            //dBodyAddRelForce (body,0, 0,getThrottle());

            dBodyAddRelTorque(body, 0, -aileron*16, 0);
            //Vec3f p1(-rudder*4, getThrottle(),-elevator*10);
            //Vec3f p2(-rudder*4, getThrottle(),-elevator*10);

            //p = toVectorInFixedSystem(p[0],p[1],p[2],aileron*10,elevator*10);


            Vec3f p1(0, 1,0);
            Vec3f p2(0,1,0);

            p1 = toVectorInFixedSystem(p1[0],p1[1],p1[2],aileron*10,elevator*10);
            p2 = toVectorInFixedSystem(p2[0],p2[1],p2[2],aileron*10,elevator*10);

            p1 = p1.normalize();
            p2 = p2.normalize();

            p1 = p1*getThrottle();
            p2 = p2*getThrottle();

            dBodyAddRelForceAtRelPos(body,p1[0], p1[1], p1[2], 0.0, +35, 0.0);
            dBodyAddRelForceAtRelPos(body,p2[0], p2[1], p2[2], 0.0, -35, 0.0);
        }
    }

    wrapDynamics(body);
}



Vehicle* Cephalopod::fire(dWorldID world, dSpaceID space)
{
    Gunshot *action = new Gunshot();
    // Need axis conversion.
    action->init();
    action->setOrigin(me);


    Vec3f position = getPos();
    position[1] += 0.0f; // Move upwards to the center of the real rotation.
    // Check where are we aiming in body coordinates.
    forward = toVectorInFixedSystem(0,0,1,pan, tilt);
    dVector3 result;
    dBodyVectorToWorld(me, forward[0],forward[1],forward[2],result);

    forward = Vec3f(result[0],result[1],result[2]);
    Vec3f Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    Vec3f orig;


    forward = forward.normalize();
    orig = position;
    position = position + 60.0f*forward;
    forward = -orig+position;

    Vec3f Ft = forward*100;

    Vec3f f1(0.0,0.0,1.0);
    Vec3f f2 = forward.cross(f1);
    f2 = f2.normalize();
    float alpha = acos( forward.dot(f1)/(f1.magnitude()*forward.magnitude()));

    dMatrix3 Re;
    dRSetIdentity(Re);
    dRFromAxisAndAngle(Re,f2[0],f2[1],f2[2],-alpha);


    action->embody(world,space);
    action->setPos(position[0],position[1],position[2]);
    dBodySetLinearVel(action->getBodyID(),Ft[0],Ft[1],Ft[2]);
    dBodySetRotation(action->getBodyID(),Re);

    // I can set power or something here.
    return (Vehicle*)action;
}


/**
Vehicle* AdvancedWalrus::fire(dWorldID world, dSpaceID space)
{
    if (getTtl()>0)
        return NULL;

    ArtilleryAmmo *action = new ArtilleryAmmo();
    // Need axis conversion.
    action->init();

    Vec3f position = getPos();

    // Check where are we aiming in body coordinates.
    forward = toVectorInFixedSystem(0,0,1,azimuth, elevation);
    dVector3 result;
    dBodyVectorToWorld(me, forward[0],forward[1],forward[2],result);

    forward = Vec3f(result[0],result[1],result[2]);


    Vec3f Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    Vec3f orig;

    // Calculates bullet initial position (trying to avoid hitting myself).
    forward = forward.normalize();
    orig = position;
    position = position + 40*forward;
    forward = -orig+position;

    // Bullet energy
    Vec3f Ft = forward*15;

    // Bullet rotation (alignment with forward direction)
    Vec3f f1(0.0,0.0,1.0);
    Vec3f f2 = forward.cross(f1);
    f2 = f2.normalize();
    float alpha = acos( forward.dot(f1)/(f1.magnitude()*forward.magnitude()));

    dMatrix3 Re;
    dRSetIdentity(Re);
    dRFromAxisAndAngle(Re,f2[0],f2[1],f2[2],-alpha);

    // Shift origin up towards where the turret is located.
    position = orig;
    position[1] += firingpos[1];
    action->embody(world,space);
    action->setPos(position[0],position[1],position[2]);

    Vec3f d = action->getPos() - getPos();

    //dout << d << std::endl;

    dout << "Elevation:" << elevation << " Azimuth:" << azimuth << std::endl;

    dBodySetLinearVel(action->getBodyID(),Ft[0],Ft[1],Ft[2]);
    dBodySetRotation(action->getBodyID(),Re);

    // Recoil (excellent for the simulation, cumbersome for playing...)
    Ft = Ft.normalize();  Ft=Ft * 0.2;

    //dBodyAddRelForceAtPos(me,-Ft[0],-Ft[1],-Ft[2], 0.0, firingpos[1], 0.0);
    artilleryshot();
    //setTtl(200);

    // I can set power or something here.
    return (Vehicle*)action;
}**/
