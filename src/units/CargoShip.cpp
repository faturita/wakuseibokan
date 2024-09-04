/*
 * CargoShip.cpp
 *
 *  Created on: Sep 2, 2024
 *      Author: faturita
 */

#include "CargoShip.h"
#include "../ThreeMaxLoader.h"
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
extern std::vector<BoxIsland*> islands;

CargoShip::CargoShip(int newfaction)
{
    setFaction(newfaction);
}

void CargoShip::init()
{
    //Load the model
    _model = (Model*)T3DSModel::loadModel(filereader("units/cargoship.3ds"),0.0f,-48.0f,0.0f,0.1,textures["boat"]);

    setForward(0,0,1);

    setName("CargoShip");

    height=75.0f;
    width=67.0f;
    length=290.0f;

    Vehicle::signal = CommLink::LONG_RANGE;

}

int CargoShip::getType()
{
    return WALRUS;
}

int CargoShip::getSubType()
{
    return CARGOSHIP;
}

EntityTypeId CargoShip::getTypeId()
{
    return EntityTypeId::TCargoShip;
}

void CargoShip::doMaterial()
{
    GLfloat specref[] = { 1.0f, 1.0f, 1.0f, 1.0f};

    glEnable(GL_COLOR_MATERIAL);

    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    glMaterialfv(GL_FRONT, GL_SPECULAR, specref);
    glMateriali(GL_FRONT, GL_SHININESS,128);
}


void CargoShip::drawModel(float yRot, float xRot, float x, float y, float z)
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
        //glRotatef(yRot, 0.0f, 1.0f, 0.0f);

        //glRotatef(xRot, 1.0f, 0.0f, 0.0f);

        doMaterial();


        _model->draw();
        //drawRectangularBox(width, height, length);

        glPopMatrix();
    }
    else
    {
    	printf ("model is null\n");
    }
}

void CargoShip::doControl()
{
    switch (autostatus) {
        case AutoStatus::ATTACK:        doControlAttack();      break;
        case AutoStatus::DESTINATION:   doControlDestination(); break;
        case AutoStatus::DOCKING:       doControlDocking(); break;
        default: break;
    }

}

void CargoShip::doControlAttack()
{
    Controller c;

    c.registers = registers;

    Vec3f Po = getPos();

    Po[1] = 0.0f;

    Vec3f Pf = destination;

    Vec3f T = Pf - Po;

    if (T.magnitude()>300)
    {
        float distance = T.magnitude();

        Vec3f F = getForward();

        F = F.normalize();
        T = T.normalize();


        // Potential fields from the islands (to slow down Walrus)

        c.registers.thrust = 400.0f;

        if (distance<3000.0f)
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
            c.registers.thrust = 15.0f;
        }


        float e = _acos(  T.dot(F) );

        float signn = T.cross(F) [1];


        //CLog::Write(CLog::Debug,"T: %10.3f %10.3f %10.3f %10.3f\n", closest, distance, e, signn);

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
    if (T.magnitude()<300)
    {
        float distance = T.magnitude();

        Vec3f F = getForward();

        F = F.normalize();
        T = T.normalize();

        float e = _acos(  T.dot(F) );

        float signn = T.cross(F) [1];


        //CLog::Write(CLog::Debug,"T: %10.3f %10.3f %10.3f %10.3f\n", closest, distance, e, signn);

        if (abs(e)>=0.2f)
            c.registers.roll = 400.0 * (signn>0?+1:-1) ;
        else {
            c.registers.roll = 0.0f;
        }

        c.registers.thrust = 2.0;

        Vehicle *action = fire(0,world,space);

        if (action != NULL)
        {
            entities.push_at_the_back(action, action->getGeom());
            gunshot(getPos());
            setTtl(5);
        }


    }

    doControl(c);

}

void CargoShip::doControlDocking()
{
    Controller c;

    c.registers = registers;


    Vec3f Po = getPos();

    Po[1] = 0.0f;

    Vec3f Pf = destination;

    Vec3f T = Pf - Po;

    float roundederror = 100;


    if (dst_status != DestinationStatus::REACHED && getStatus() != SailingStatus::DOCKED || T.magnitude()<roundederror)
    {
        float distance = T.magnitude();

        Vec3f F = getForward();

        F = F.normalize();
        T = T.normalize();


        // Potential fields from the islands (to avoid them)
        int nearesti = 0;
        float closest = 0;
        for(size_t i=0;i<islands.size();i++)
        {
            BoxIsland *b = islands[i];
            Vec3f l(b->getX(),0.0f,b->getZ());

            if ((l-Po).magnitude()<closest || closest ==0) {
                closest = (l-Po).magnitude();
                nearesti = i;
            }
        }

        c.registers.thrust = 400.0f;



        // Potential fields to avoid islands (works great).
        if (closest > 1800 && closest < 4000)
        {
            BoxIsland *b = islands[nearesti];
            Vec3f l = b->getPos();
            Vec3f d = Po-l;
            d = d.normalize();

            if (distance>2000.0)
            {
                T = T+d;
                T = T.normalize();
            }

            c.registers.thrust = 40.0f;
            
        }

        if (distance<800.0f)
        {
            c.registers.thrust = 10.0f;
        }

        // Potential fields from Carrier
        Vehicle *cd = findCarrier(getFaction());
        if (cd)
        {
            float obstacle = (cd->getPos() - Po).magnitude();
            if (obstacle < 650) // check the size
            {
                Vec3f l = cd->getPos();
                Vec3f d = Po-l;

                d = d.normalize();

                T = T+d;
                T = T.normalize();

                c.registers.thrust = 100.0f;
            }
        }


        float e = _acos(  T.dot(F) );

        float signn = T.cross(F) [1];


        CLog::Write(CLog::Debug,"T: %10.3f %10.3f %10.3f %10.3f\n", closest, distance, e, signn);


        /**
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
        }**/


        c.registers.roll = abs(e) * (signn>0?+1:-1)  * 40;


    } else {
        if (dst_status != DestinationStatus::REACHED)
        {
            char str[256];
            Message mg;
            mg.faction = getFaction();
            sprintf(str, "%s has arrived to destination.", getName().c_str());
            mg.msg = std::string(str);mg.timer = 0;
            messages.insert(messages.begin(), mg);
            CLog::Write(CLog::Debug,"Walrus has reached its destination.\n");
            dst_status = DestinationStatus::REACHED;
            autostatus = AutoStatus::FREE;
            c.registers.thrust = 0.0f;
            setThrottle(0.0);
            c.registers.roll = 0.0f;
            disableAuto();
        }
    }

    doControl(c);    
}

void CargoShip::doControlDestination()
{
    Controller c;

    c.registers = registers;


    Vec3f Po = getPos();

    Po[1] = 0.0f;

    Vec3f Pf = destination;

    Vec3f T = Pf - Po;

    float roundederror = 400;

    if (getStatus() == SailingStatus::ROLLING)
        roundederror = 5;


    if (dst_status != DestinationStatus::REACHED && T.magnitude()>roundederror)
    {
        float distance = T.magnitude();

        Vec3f F = getForward();

        F = F.normalize();
        T = T.normalize();


        // Potential fields from the islands (to slow down Walrus)

        c.registers.thrust = 400.0f;

        if (distance<800.0f)
        {
            c.registers.thrust = 20.0f;
        }

        float closest=0;
        BoxIsland *b = findNearestIsland(Po);
        if (b)
        {
            closest = (b->getPos() - Po).magnitude();
            if (closest > 1600 && closest < 2200)
            {
                c.registers.thrust = 15.0f;

            }
        }

        if (island != NULL)
        {
            c.registers.thrust = 400.0;
        }

        // Potential fields from Carrier
        Vehicle *cd = findCarrier(getFaction());
        if (cd)
        {
            float obstacle = (cd->getPos() - Po).magnitude();
            if (obstacle < 650) // check the size
            {
                Vec3f l = cd->getPos();
                Vec3f d = Po-l;

                d = d.normalize();

                T = T+d;
                T = T.normalize();

                c.registers.thrust = 100.0f;
            }
        }


        float e = _acos(  T.dot(F) );

        float signn = T.cross(F) [1];


        CLog::Write(CLog::Debug,"T: %10.3f %10.3f %10.3f %10.3f\n", closest, distance, e, signn);


        /**
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
        }**/


        c.registers.roll = abs(e) * (signn>0?+1:-1)  * 40;


    } else {
        if (dst_status != DestinationStatus::REACHED)
        {
            char str[256];
            Message mg;
            mg.faction = getFaction();
            sprintf(str, "%s has arrived to destination.", getName().c_str());
            mg.msg = std::string(str);mg.timer = 0;
            messages.insert(messages.begin(), mg);
            CLog::Write(CLog::Debug,"Walrus has reached its destination.\n");
            dst_status = DestinationStatus::REACHED;
            autostatus = AutoStatus::FREE;
            c.registers.thrust = 0.0f;
            setThrottle(0.0);
            c.registers.roll = 0.0f;
            disableAuto();
        }
    }

    doControl(c);
}

void CargoShip::doControl(Controller controller)
{
    doControl(controller.registers);

}

void CargoShip::doControl(struct controlregister conts)
{
    static bool didit = false;

    // Thrust is limited !  It cannot be unbounded !
    if (conts.thrust>200.0f)
        conts.thrust=200.0f;

    setThrottle(conts.thrust);

    if (getThrottle()>=10 && getThrottle()<=20 and !didit)
    {
        didit = true;
        smallenginestart(pos);
    } else if (getThrottle()==0)
        didit = false;
    
    xRotAngle = conts.roll;
    
}


void CargoShip::drawDirectModel()
{
    float modX=0.0f, modY=15.0f, modZ=0.0f;

    modX = pos[0];
    modY = pos[1];
    modZ = pos[2];

    Vec3f forward = toVectorInFixedSystem(0, 0, -0.1,-xRotAngle,yRotAngle);

    modX+=speed*forward[0]; modY+=speed*forward[1];modZ+=speed*forward[2];

    pos += speed * forward;

    drawModel(xRotAngle, yRotAngle, modX, modY, modZ);
}


void CargoShip::embody(dWorldID world, dSpaceID space)
{
    me = dBodyCreate(world);
    embody(me);
    //geom = dCreateSphere( space, 2.64f);
    //geom = dCreateBox( space, 2.64f, 2.64f, 2.64f);
    geom = dCreateBox( space, width, height, length);
    //geom = dCreateBox (space,10.0f,2.0f,30.0f);
    dGeomSetBody(geom, me);
}

void CargoShip::embody(dBodyID myBodySelf)
{
    dMass m;
    
    float myMass = 100.0f;
    
    dBodySetPosition(myBodySelf, pos[0], pos[1], pos[2]);
    dMassSetBox(&m,1,width,height,length);
    //dMassSetSphere(&m,1,radius);
    dMassAdjust(&m, myMass*1.0f);
    dBodySetMass(myBodySelf,&m);
    
    me = myBodySelf;
    
}

void CargoShip::doDynamics()
{
    doDynamics(getBodyID());
}

void CargoShip::doDynamics(dBodyID body)
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
        // Walrus has capsized.
        damage(1);
    }


    if (VERIFY(pos,me) && !Vehicle::inert)
    {
        if (attitude < 45)
        {
            //dBodyAddRelForce (body,0, 0,getThrottle());

            //dBodyAddRelTorque(body, 0, -xRotAngle*0.1, 0);
            Vec3f p(0.0, 0.0, getThrottle()*10);

            p = toVectorInFixedSystem(p[0],p[1],p[2],-xRotAngle*0.1, 0.0);

            dBodyAddRelForceAtRelPos(body,p[0], p[1], p[2], 0.0, -37, -144);
        }
    }

    wrapDynamics(body);
}


Vehicle* CargoShip::fire(int weapon, dWorldID world, dSpaceID space)
{
    if (getTtl()>0)
        return NULL;
    Gunshot *action = new Gunshot();
    // Need axis conversion.
    action->init();
    action->setOrigin(me);

    Vec3f position = getPos();
    position[1] += .5f; // Move upwards to the center of the real rotation.
    forward = getForward();
    Vec3f Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    Vec3f orig;

    forward = forward.normalize();
    orig = position;
    position = position + 60.0f*forward;
    forward = -orig+position;

    // Shoot faster to avoid hurting myself (moving myself indeed, hurting is disabled per collision handling).
    Vec3f Ft = forward*100.0f;

    Vec3f f1(0.0,0.0,1.0);
    Vec3f f2 = forward.cross(f1);
    f2 = f2.normalize();
    float alpha = _acos( forward.dot(f1)/(f1.magnitude()*forward.magnitude()));

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

void CargoShip::setStatus(int status)
{
    if (status == SailingStatus::OFFSHORING || status == SailingStatus::INSHORING)
        setTtl(500);

    Vehicle::status = status;
}


void CargoShip::attack(Vec3f target)
{
    autostatus = AutoStatus::ATTACK;
    destination = target;
}

void CargoShip::setNameByNumber(int number)
{
    setNumber(number);
    setName("CargoShip", number);
}

void  CargoShip::getViewPort(Vec3f &Up, Vec3f &position, Vec3f &forward)
{
    position = getPos();
    forward = getForward();
    Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    Vec3f orig;


    forward = forward.normalize();
    orig = position;
    Up[0]=Up[2]=0;Up[1]=70;
    position = position - 350*forward + Up;
    forward = orig-position;
}
