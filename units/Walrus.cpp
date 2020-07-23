/*
 * Walrus.cpp
 *
 *  Created on: Jan 16, 2011
 *      Author: faturita
 */

#include "Walrus.h"
#include "../md2model.h"

#include "../actions/Gunshot.h"

#include "../engine.h"

extern dWorldID world;
extern dSpaceID space;
#include "../container.h"
#include "../sounds/sounds.h"

extern container<Vehicle*> entities;

extern GLuint _textureSky;

Walrus::Walrus(int newfaction)
{
    setFaction(newfaction);
}

int Walrus::getNumber() const
{
    return number;
}

void Walrus::setNumber(int value)
{
    number = value;
}

BoxIsland *Walrus::getIsland() const
{
    return island;
}

void Walrus::setIsland(BoxIsland *value)
{
    island = value;
}

void Walrus::init()
{
    if (getFaction()==GREEN_FACTION)
        _model = (Model*)MD2Model::loadModel("walrusgood.md2");
    else {
        _model = (Model*)MD2Model::loadModel("walrus.md2");
    }

    if (_model != NULL)
    {
        _model->setAnimation("run");
        _topModel = (Model*)T3DSModel::loadModel("structures/turrettop.3ds",0,0,0,0.4,0.4,0.4,0);
    }
    else
    	printf ("Model has been initialized");

    setForward(0,0,1);

    Walrus::height=4.0f;
    Walrus::width=5.0f;
    Walrus::length=10.0f;

}

int Walrus::getType()
{
    return WALRUS;
}

void Walrus::doMaterial()
{
    GLfloat specref[] = { 1.0f, 1.0f, 1.0f, 1.0f};

    glEnable(GL_COLOR_MATERIAL);

    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    glMaterialfv(GL_FRONT, GL_SPECULAR, specref);
    glMateriali(GL_FRONT, GL_SHININESS,128);
}


void Walrus::drawModel(float yRot, float xRot, float x, float y, float z)
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
        //glRotatef(yRot, 0.0f, 1.0f, 0.0f);

        //glRotatef(xRot, 1.0f, 0.0f, 0.0f);

        doMaterial();
        //drawRectangularBox(width, height, length);

        _model->setTexture(_textureSky);
        glTranslatef(0.0f,0.0f,-2.0f);
        _model->draw();

        glRotatef(90.0f,0.0f,1.0f,0.0f);
        //glScalef(0.4f,0.4f,0.4f);
        _topModel->setTexture(_textureSky);
        _topModel->draw();

        glPopMatrix();
    }
    else
    {
    	printf ("model is null\n");
    }
}

void Walrus::drawModel()
{
	drawModel(0,0,pos[0],pos[1]+1.0,pos[2]);
}

void Walrus::doControl()
{
    switch (aistatus) {
        case ATTACK: doControlAttack();break;
        case DESTINATION: doControlDestination();break;
        default: break;
    }

}

void Walrus::doControlAttack()
{
    Controller c;

    c.registers = myCopy;

    Vec3f Po = getPos();

    Po[1] = 0.0f;

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
            c.registers.thrust = 15.0f;
        }


        float e = acos(  T.dot(F) );

        float signn = T.cross(F) [1];


        //printf("T: %10.3f %10.3f %10.3f %10.3f\n", closest, distance, e, signn);

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
        float distance = T.magnitude();

        Vec3f F = getForward();

        F = F.normalize();
        T = T.normalize();

        float e = acos(  T.dot(F) );

        float signn = T.cross(F) [1];


        //printf("T: %10.3f %10.3f %10.3f %10.3f\n", closest, distance, e, signn);

        if (abs(e)>=0.2f)
            c.registers.roll = 400.0 * (signn>0?+1:-1) ;
        else {
            c.registers.roll = 0.0f;
        }

        c.registers.thrust = 2.0;

        Vehicle *action = fire(world,space);

        if (action != NULL)
        {
            entities.push_back(action,action->getBodyID());
            gunshot();
            setTtl(5);
        }


    }

    doControl(c);

}

void Walrus::doControlDestination()
{
    Controller c;

    c.registers = myCopy;


    Vec3f Po = getPos();

    Po[1] = 0.0f;

    Vec3f Pf = destination;

    Vec3f T = Pf - Po;

    if (!reached && T.magnitude()>500)
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
            c.registers.thrust = 15.0f;
        }


        float e = acos(  T.dot(F) );

        float signn = T.cross(F) [1];


        //printf("T: %10.3f %10.3f %10.3f %10.3f\n", closest, distance, e, signn);

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


    } else {
        if (!reached)
        {
            char str[256];
            sprintf(str, "Walrus has arrived to destination.");
            //messages.insert(messages.begin(), str);
            printf("Walrus has reached its destination.\n");
            reached = true;
            aistatus = FREE;
            c.registers.thrust = 0.0f;
            c.registers.roll = 0.0f;
            disableAuto();
        }
    }

    doControl(c);
}

void Walrus::doControl(Controller controller)
{
    doControl(controller.registers);

}

void Walrus::doControl(struct controlregister conts)
{
    static bool didit = false;

    // Thrust is limited !  It cannot be unbounded !
    if (conts.thrust>200.0f)
        conts.thrust=200.0f;

    setThrottle(conts.thrust);

    if (getThrottle()>=10 && getThrottle()<=20 and !didit)
    {
        didit = true;
        smallenginestart();
    } else if (getThrottle()==0)
        didit = false;
    
    xRotAngle = conts.roll;
    
}


void Walrus::drawDirectModel()
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


void Walrus::embody(dWorldID world, dSpaceID space)
{
    me = dBodyCreate(world);
    embody(me);
    //geom = dCreateSphere( space, 2.64f);
    //geom = dCreateBox( space, 2.64f, 2.64f, 2.64f);
    geom = dCreateBox( space, width, height, length);
    //geom = dCreateBox (space,10.0f,2.0f,30.0f);
    dGeomSetBody(geom, me);
}

void Walrus::embody(dBodyID myBodySelf)
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

void Walrus::doDynamics()
{
    doDynamics(getBodyID());
}

void Walrus::doDynamics(dBodyID body)
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

    if (getTtl()<=0 && getStatus() == Walrus::OFFSHORING)
        setStatus(Walrus::SAILING);
    else if (getTtl()<=0 && getStatus() == Walrus::INSHORING)
        setStatus(Walrus::ROLLING);

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

    //printf("Angle between vectors %10.5f\n", acos(upInBody.dot(Up))*180.0/PI);

    float attitude = acos(upInBody.dot(Up))*180.0/PI;

    //std::cout << "Attitude:" << attitude << std::endl;

    if (attitude>80 || attitude<-80)
    {
        // Walrus has tumbled.
        damage(1);
    }


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


Vehicle* Walrus::fire(dWorldID world, dSpaceID space)
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
    Vec3f Ft = forward*10000.0f;

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

void Walrus::setStatus(int status)
{
    if (status == Walrus::OFFSHORING || status == Walrus::INSHORING)
        setTtl(500);

    Vehicle::status = status;
}


void Walrus::attack(Vec3f target)
{
    aistatus = ATTACK;
    destination = target;
}
