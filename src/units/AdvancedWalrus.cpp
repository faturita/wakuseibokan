#include <unordered_map>
#include "AdvancedWalrus.h"

#include "../profiling.h"

#include "../actions/ArtilleryAmmo.h"
#include "../math/geometry.h"
#include "../engine.h"
extern dWorldID world;
extern dSpaceID space;
#include "../container.h"
#include "../sounds/sounds.h"

extern container<Vehicle*> entities;
extern std::vector<BoxIsland*> islands;
extern std::unordered_map<std::string, GLuint> textures;
extern std::vector<Message> messages;

AdvancedWalrus::AdvancedWalrus(int newfaction) : Walrus(newfaction)
{

}

void AdvancedWalrus::init()
{
    // Keep in mind that the 3DSModel should be centered.
    _model = (Model*)T3DSModel::loadModel(filereader("units/walrus.3ds"),0,0,0,1,1,1,0);
    if (_model != NULL)
    {
        _topModel = (Model*)T3DSModel::loadModel(filereader("structures/turrettop.3ds"),0,0,0,0.1,0.1,0.1,0);
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

        Vehicle::doMaterial();
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
        CLog::Write(CLog::Debug,"Model is null.\n");
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
            Vec3f p(0.0, 0.0, getThrottle()*2);

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
        case AutoStatus::DOCKING:       doControlDocking(); break;
        case AutoStatus::WAYPOINT:      doControlWaypoint(); break;
        default: break;
    }
}

void AdvancedWalrus::doControlDocking()
{
    Controller c;

    c.registers = registers;

    Vec3f Po = getPos();

    Po[1] = 0.0f;

    Vec3f Pf = destination;

    Vec3f T = Pf - Po;

    float roundederror = 100;

    //dout << "Running docking control." << std::endl;

    if (getStatus() != SailingStatus::DOCKED || T.magnitude()<roundederror)
    {
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

        if (T.magnitude()>2000)
        {
            BoxIsland *b = islands[nearesti];
            Vec3f l = b->getPos();
            T = T-l;
            Vec3f newdestination = destination + T.normalize()*1000.0;
            T = newdestination - Po;
        }
        float distance = T.magnitude();

        Vec3f F = getForward();

        F = F.normalize();
        T = T.normalize();




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


        float e = _acos(  T.dot(F) );

        float signn = T.cross(F) [1];


        //CLog::Write(CLog::Debug,"T: %10.3f %10.3f %10.3f %10.3f\n", closest, distance, e, signn);


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
            dst_status = DestinationStatus::REACHED;
            c.registers.thrust = 0.0f;
            setThrottle(0.0);
            c.registers.roll = 0.0f;
            setAutoStatus(AutoStatus::IDLE);
        }
    }

    doControl(c);    
}

void AdvancedWalrus::doControlWaypoint()
{
    if ( (dst_status == DestinationStatus::READY || dst_status == DestinationStatus::REACHED) && !waypoints.empty())
    {
        destination = waypoints.front();
        waypoints.pop();
        dst_status = DestinationStatus::TRAVELLING;
    }
    if (dst_status == DestinationStatus::REACHED && waypoints.empty())
    {
        dst_status = DestinationStatus::REACHED;
        setAutoStatus(AutoStatus::IDLE);
    }

    doControlDestination();
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

void AdvancedWalrus::doControlDestination()
{
    Controller c;

    c.registers = registers;

    Vec3f Po = getPos();
    Po[1] = 0.0f; 

    Vec3f Pf = destination;

    Vec3f T = Pf - Po;

    float roundederror = 100;

    if (getStatus() == SailingStatus::ROLLING)
        roundederror = 5;

    if (T.magnitude()>100)
    {
        float distance = T.magnitude();

        Vec3f F = getForward();

        F = F.normalize();
        T = T.normalize();


        // Potential fields from the islands (to slow down Walrus)

        c.registers.thrust = 1000.0f;

        if (distance<10000.0f)
        {
            c.registers.thrust = 200.0f;
        }

        if (distance<2000.0f)
        {
            c.registers.thrust = 100.0f;
        }

        BoxIsland *b = findNearestIsland(Po);

        float closest = 0;
        if (b)
        {
            closest = (b->getPos() - Po).magnitude();
            if (closest > 1600 && closest < 2400 && (getStatus() == SailingStatus::SAILING))
            {
                c.registers.thrust = 15.0f;
            }
        }

        // Potential fields from its own Carrier
        Vehicle *cd = findCarrier(getFaction());
        float obstacle = 0;
        if (cd)
        {
            std::vector<Vec3f> vertices = ((Balaenidae*)cd)->getVertices();
            Vec3f cc = findClosestPointOnPolygon(Po, vertices, obstacle);
            obstacle = (cc - Po).magnitude();   // cd->getPos();
            if (obstacle < 200) // check the size
            {
                Vec3f l = cc;
                Vec3f d = Po-l;

                d = d.normalize();

                T = T+d;
                T = T.normalize();

                c.registers.thrust = 15.0f;
            }
        }        

        float e = _acos(  T.dot(F) );

        float signn = T.cross(F) [1];


        CLog::Write(CLog::Debug,"T: %10.3f %10.3f %10.3f %10.3f %10.3f\n", distance, closest, obstacle, e, signn);

        if (abs(e)>=0.5f)
        {
            c.registers.roll = 30.0 * (signn>0?+1:-1) ;
            if (getStatus() == SailingStatus::SAILING) c.registers.thrust = 15.0f;
        } else
        if (abs(e)>=0.4f)
        {
            c.registers.roll = 2.0 * (signn>0?+1:-1) ;
            if (getStatus() == SailingStatus::SAILING) c.registers.thrust = 15.0f;
        } else
        if (abs(e)>=0.2f)
            c.registers.roll = 1.0 * (signn>0?+1:-1) ;
        else {
            c.registers.roll = 0.0f;
        }


    } else
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
            autostatus = AutoStatus::IDLE;
            c.registers.thrust = 0.0f;
            setThrottle(0.0);
            c.registers.roll = 0.0f;
        }

    doControl(c);

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
    if (T.magnitude()<1000)
    {
        Vec3f target = destination - getPos();
        Vec3f aim = toBody(me,target);

        dout << "Target:" << aim <<  ":Loc: " << getPos() << " Target: " << destination<< std::endl;


        float azimuth=getAzimuth(aim);
        float declination=getDeclination(aim);

        //struct controlregister c;
        //c = getControlRegisters();
        c.registers.precesion = azimuth;
        c.registers.pitch = declination;
        c.registers.thrust = 0.0f; // Stop the walrus.
        //setControlRegisters(c);

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

    // Bullet initial speed towards the turret direction.
    Vec3f Ft = forward.normalize()*firepower;

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

    //dout << "Firing height:" << position[1] << "-" << Ft << " - Elevation:" << elevation << " Azimuth:" << azimuth << std::endl;

    dBodySetLinearVel(action->getBodyID(),Ft[0],Ft[1],Ft[2]);
    dBodySetRotation(action->getBodyID(),Re);
    dBodyAddRelTorque(action->getBodyID(),5.0, 4.0, 2.0);

    // @NOTE: Is the artillery affected by wind or not ?
    if (no_damping_on_bullets)
        dBodySetLinearDamping(action->getBodyID(),0);

    // Recoil (excellent for the simulation, cumbersome for playing...)
    //   Ft = Ft.normalize();  Ft=Ft * 0.2;
    //   dBodyAddRelForceAtPos(me,-Ft[0],-Ft[1],-Ft[2], 0.0, firingpos[1], 0.0);
    
    artilleryshot(getPos());
    
    if (enable_heatup)
        setTtl(20);

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

void AdvancedWalrus::setNameByNumber(int number)
{
    setNumber(number);
    setName("Seal", number);
}
