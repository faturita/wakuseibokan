#include "../profiling.h"
#include "../ThreeMaxLoader.h"
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

    float myMass = 10.0f;

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

void Cephalopod::getViewPort(Vec3f &Up, Vec3f &position, Vec3f &forward)
{
    position = getPos();
    forward = getForward();
    Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    Vec3f orig;


    forward = forward.normalize();
    orig = position;
    Up[0]=Up[2]=0;Up[1]=20;// poner en 4 si queres que este un toque arriba desde atras.
    position = position - 100*forward + Up;
    forward = orig-position;
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


    if (VERIFY(pos,me) && !Vehicle::inert)
    {
        if (attitude < 45)
        {
            //dBodyAddRelForce (body,0, 0,getThrottle());


            dBodyAddRelTorque(body, 0, -aileron*2, 0);
            Vec3f p1(-rudder*2, getThrottle(),-elevator*3);
            Vec3f p2(-rudder*2, getThrottle(),-elevator*3);

            //p = toVectorInFixedSystem(p[0],p[1],p[2],aileron*10,elevator*10);

            dBodyAddRelForceAtRelPos(body,p1[0], p1[1], p1[2], 0.0, +35, 0.0);
            dBodyAddRelForceAtRelPos(body,p2[0], p2[1], p2[2], 0.0, -35, 0.0);
        }
    }

    wrapDynamics(body);
}

