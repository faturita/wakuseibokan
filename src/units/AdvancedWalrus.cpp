#include <unordered_map>
#include "AdvancedWalrus.h"

#include "../profiling.h"

#include "../actions/ArtilleryAmmo.h"

#include "../engine.h"
extern dWorldID world;
extern dSpaceID space;
#include "../container.h"
#include "../sounds/sounds.h"

extern container<Vehicle*> entities;

extern std::unordered_map<std::string, GLuint> textures;

AdvancedWalrus::AdvancedWalrus(int newfaction) : Walrus(newfaction)
{

}

void AdvancedWalrus::init()
{
    // Keep in mind that the 3DSModel should be centered.
    _model = (Model*)T3DSModel::loadModel("units/walrus.3ds",0,0,0,1,1,1,0);
    if (_model != NULL)
    {
        _topModel = (Model*)T3DSModel::loadModel("structures/turrettop.3ds",0,0,0,0.1,0.1,0.1,0);
    }

    setForward(0,0,1);

    width=5.0f;
    height=4.0f;
    length=10.0f;

    firingpos[1] = 2.3;
    firingpos[0]=firingpos[2]=0;

    aim = Vec3f(0,0,0);

    azimuth=0;
    elevation=0;


}

void AdvancedWalrus::doMaterial()
{
    GLfloat specref[] = { 1.0f, 1.0f, 1.0f, 1.0f};

    glEnable(GL_COLOR_MATERIAL);

    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    glMaterialfv(GL_FRONT, GL_SPECULAR, specref);
    glMateriali(GL_FRONT, GL_SHININESS,128);
}

void AdvancedWalrus::drawModel(float yRot, float xRot, float x, float y, float z)
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

        glRotatef(-180.0f, 1.0f, 0.0f, 0.0f);

        doMaterial();
        //drawRectangularBox(width, height, length);

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



void AdvancedWalrus::embody(dWorldID world, dSpaceID space)
{
    me = dBodyCreate(world);
    embody(me);
    //geom = dCreateSphere( space, 2.64f);
    //geom = dCreateBox( space, 2.64f, 2.64f, 2.64f);
    geom = dCreateBox( space, width, height, length);
    //geom = dCreateBox (space,10.0f,2.0f,30.0f);
    dGeomSetBody(geom, me);
}

void AdvancedWalrus::embody(dBodyID myBodySelf)
{
    dMass m;

    float myMass = 20.0f;

    dBodySetPosition(myBodySelf, pos[0], pos[1], pos[2]);
    dMassSetBox(&m,1,width,height,length);
    //dMassSetSphere(&m,1,radius);
    dMassAdjust(&m, myMass*1.0f);
    dBodySetMass(myBodySelf,&m);

    me = myBodySelf;

}

void AdvancedWalrus::doDynamics()
{
    doDynamics(getBodyID());
}

void AdvancedWalrus::doAmphibious(dBodyID body)
{
    if (getTtl()<=0 && getStatus() == SailingStatus::OFFSHORING)
        setStatus(SailingStatus::SAILING);
    else if (getTtl()<=0 && getStatus() == SailingStatus::INSHORING)
        setStatus(SailingStatus::ROLLING);

    // This algorithm is generating too many seg faults with ODE library.
    //Vec3f dump;
    //if (vec3fV.magnitude() != 0 && vec3fF.magnitude() != 0)
    //{
    //	dump = - ((vec3fV.cross(vec3fF).magnitude())/(vec3fV.magnitude()*vec3fF.magnitude())*10.0f) * vec3fV - 0.001 * vec3fV;
    //}

    //if (!isnan(dump[0]) && !isnan(dump[1]) && !isnan(dump[2]))
    //{
        //dBodyAddForce(body, dump[0], dump[1], dump[2]);
    //}


    //dBodyAddForce(body[i],damping[0]*-dumpMedia[i][0],damping[1]*-dumpMedia[i][0],damping[2]*-dumpMedia[i][0]);

    //dReal *angulardumping = (dReal *)dBodyGetAngularVel(body);

    //dBodyAddTorque(body,angulardumping[0]*-0.1,angulardumping[1]*-0.1,angulardumping[2]*-0.1 );


    //if ((speed)>1.0 && speed < 1.3)
        //enginestart();

    dVector3 result;
    dBodyVectorFromWorld(body, 0,1,0,result);

    Vec3f upInBody = Vec3f(result[0],result[1],result[2]);
    Vec3f Up = Vec3f(0.0f,1.0f,0.0f);

    upInBody = upInBody.normalize();

    //CLog::Write(CLog::Debug,"Angle between vectors %10.5f\n", _acos(upInBody.dot(Up))*180.0/PI);

    float attitude = _acos(upInBody.dot(Up))*180.0/PI;

    //dout << "Attitude:" << attitude << std::endl;

    if (attitude>80 || attitude<-80)
    {
        // Walrus has tumbled.
        damage(1);
    }
}

void AdvancedWalrus::doDynamics(dBodyID body)
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

    doAmphibious(body);

    dVector3 result;
    dBodyVectorFromWorld(body, 0,1,0,result);

    Vec3f upInBody = Vec3f(result[0],result[1],result[2]);
    Vec3f Up = Vec3f(0.0f,1.0f,0.0f);

    upInBody = upInBody.normalize();

    //CLog::Write(CLog::Debug,"Angle between vectors %10.5f\n", _acos(upInBody.dot(Up))*180.0/PI);

    float attitude = _acos(upInBody.dot(Up))*180.0/PI;


    if (VERIFY(pos,me) && !Vehicle::inert)
    {
        if (attitude < 45)
        {
            //dBodyAddRelForce (body,0, 0,getThrottle());

            //dBodyAddRelTorque(body, 0, -xRotAngle*0.1, 0);
            Vec3f p(0.0, 0.0, getThrottle());

            p = toVectorInFixedSystem(p[0],p[1],p[2],-xRotAngle*0.1, 0.0);

            dBodyAddRelForceAtRelPos(body,p[0], p[1], p[2], 0.0, -0.8, -4.9);
        }
    }

    wrapDynamics(body);
}



// Executed by the AI
void AdvancedWalrus::doControl()
{
    switch (autostatus) {
        case AutoStatus::ATTACK:        doControlAttack();break;
        case AutoStatus::DESTINATION:   doControlDestination();break;
        default: break;
    }
}

void AdvancedWalrus::doControl(Controller controller)
{
    doControl(controller.registers);

}

void AdvancedWalrus::doControl(struct controlregister conts)
{
    Walrus::doControl(conts);

    azimuth = conts.precesion;
    elevation = conts.pitch;

    setAim(toVectorInFixedSystem(0,0,1,azimuth, -elevation));

    registers.thrust = getThrottle();
}


void AdvancedWalrus::doControlAttack()
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

    doControl(c);

}

Vec3f AdvancedWalrus::getAim()
{
    return aim;
}

void AdvancedWalrus::setAim(Vec3f aim)
{
    AdvancedWalrus::aim = aim;
}

Vec3f AdvancedWalrus::getFiringPort()
{
    return Vec3f(getPos()[0],getPos()[1]+firingpos[1],getPos()[2]);
}

void  AdvancedWalrus::getViewPort(Vec3f &Up, Vec3f &position, Vec3f &viewforward)
{
    position = getPos();
    viewforward = toVectorInFixedSystem(0,0,1,azimuth, -0);                         // I dont care the declination for the viewport

    // ViewForward is in body coordinates, I need to convert it to global coordinates.
    viewforward = toWorld(me, viewforward);

    Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    Vec3f orig;


    viewforward = viewforward.normalize();
    orig = position;
    Up[0]=Up[2]=0;Up[1]=10;
    position = position - viewport_height*viewforward + Up;
    viewforward = orig-position;
}

Vehicle* AdvancedWalrus::fire(int weapon, dWorldID world, dSpaceID space)
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
    float alpha = _acos( forward.dot(f1)/(f1.magnitude()*forward.magnitude()));

    dMatrix3 Re;
    dRSetIdentity(Re);
    dRFromAxisAndAngle(Re,f2[0],f2[1],f2[2],-alpha);

    // Shift origin up towards where the turret is located.
    //position = orig;
    position[1] += firingpos[1];
    action->embody(world,space);
    action->setPos(position[0],position[1],position[2]);

    Vec3f d = action->getPos() - getPos();

    //dout << d << std::endl;

    dout << "Elevation:" << elevation << " Azimuth:" << azimuth << std::endl;

    dBodySetLinearVel(action->getBodyID(),Ft[0],Ft[1],Ft[2]);
    dBodySetRotation(action->getBodyID(),Re);
    dBodyAddRelTorque(action->getBodyID(),5.0, 4.0, 2.0);

    // Recoil (excellent for the simulation, cumbersome for playing...)
    Ft = Ft.normalize();  Ft=Ft * 0.2;

    //dBodyAddRelForceAtPos(me,-Ft[0],-Ft[1],-Ft[2], 0.0, firingpos[1], 0.0);
    artilleryshot();
    //setTtl(200);

    // I can set power or something here.
    return (Vehicle*)action;
}

int AdvancedWalrus::getSubType()
{
    return VehicleSubTypes::ADVANCEDWALRUS;
}

EntityTypeId AdvancedWalrus::getTypeId()
{
    return EntityTypeId::TAdvancedWalrus;
}
