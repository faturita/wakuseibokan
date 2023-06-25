/*
 * Manta.cpp
 *
 *  Created on: Jan 16, 2011
 *      Author: faturita
 */

#include <unordered_map>
#include "Manta.h"
#include "../md2model.h"
#include "../actions/Gunshot.h"

extern std::unordered_map<std::string, GLuint> textures;

Manta::Manta(int faction)
{
    setFaction(faction);
}

void Manta::release(Vec3f orientation)
{
    assert(!"Not implemented");
}

void Manta::land(Vec3f landplace, Vec3f placeattitude)
{
    assert(!"Not implemented");
}

void Manta::attack(Vec3f target)
{
    assert(!"Not implemented");
}

void Manta::dogfight(Vec3f target)
{
    assert(!"Not implemented");
}

void Manta::init()
{
    //Load the model
    _model = MD2Model::loadModel("mantagood.md2");
    if (_model != NULL)
        _model->setAnimation("run");

    setForward(0,0,1);

    status = 0;

    height = 10;
    width = 10;
    length = 10;
}

int Manta::getType()
{
    return MANTA;
}

int Manta::getSubType()
{
    return SIMPLEMANTA;
}

EntityTypeId Manta::getTypeId()
{
    return EntityTypeId::TManta;
}

void Manta::doHold(Vec3f target, float thrust)
{
    assert(!"Method not implemented in Manta.");
}

void Manta::drawModel(float yRot, float xRot, float x, float y, float z)
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

void Manta::embody(dWorldID world, dSpaceID space)
{
    me = dBodyCreate(world);
    embody(me);
    //geom = dCreateSphere( space, 2.64f);
    geom = dCreateBox( space,8.0f,1.6f,4.0f);
    dGeomSetBody(geom, me);
}

void Manta::embody(dBodyID myBodySelf)
{
    dMass m;

    float myMass = 10.0f;
    float radius = 2.64f;
    float mylength = 7.0f;

    dBodySetPosition(myBodySelf, pos[0], pos[1], pos[2]);
    dMassSetBox(&m,1,8.0f,1.6f,4.0f);
    //dMassSetSphere(&m,1,radius);
    dMassAdjust(&m, myMass*1.0f);
    dBodySetMass(myBodySelf,&m);

    me = myBodySelf;

    Manta::param[0] = 0.9;
    Manta::param[1] = 2;
}

float restrict(float value, float restriction)
{
    if (value>restriction) return restriction;
    if (value<-restriction) return -restriction;
    return value;
}
void Manta::doControl()
{
    doControl(registers);
}

void Manta::doControl(Controller controller)
{
    doControl(controller.registers);

    for(int i=0;i<10;i++)
    {
        if (controller.param[i]!=0)
            Manta::param[i] = controller.param[i];
    }
}


void Manta::doControl(struct controlregister regs)
{
    if (regs.thrust>150.0f)
        regs.thrust=150.0f;

    setThrottle(regs.thrust*10.0f);  // Use the mass of Manta

    if (getThrottle()>200 && inert)
    {
        Manta::inert = false;
        antigravity = false;
        setStatus(FlyingStatus::TACKINGOFF);
    }

    if (speed>150)
    {
        setStatus(FLYING);
        // @NOTE: Eventually remove island after you takeoff.
    }

    // roll
    Manta::aileron = regs.roll;

    // pitch
    Manta::elevator = regs.pitch;

    //Manta::rudder = controller.precesion*0.1;

    Manta::rudder = regs.precesion;

}

void Manta::getViewPort(Vec3f &Up, Vec3f &position, Vec3f &forward)
{
	position = getPos();
	forward = getForward();
	Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

	Vec3f orig;


	forward = forward.normalize();
	orig = position;
    Up[0]=Up[2]=0;Up[1]=4;// poner en 4 si queres que este un toque arriba desde atras.
    position = position - 20*forward + Up;
	forward = orig-position;
}


void Manta::drawModel()
{
	drawModel(0,0,pos[0],pos[1],pos[2]);
}


void Manta::drawDirectModel()
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



Vec3f windToBody(float dx, float dy, float dz,float alpha, float beta)
{
    //float alpha = xAlpha * PI / 180.0f;
    //float beta  = yBeta  * PI / 180.0f;

    // x y z
    float x = (float)(-1)*( dx*sin(beta)            + dy*cos(beta)                            );
    float y = (float)(-1)*( dx*cos(beta)*cos(alpha) - dy*cos(beta)*sin(alpha) + dz*cos(alpha) );
    float z = (float)(+1)*( dx*cos(beta)*sin(alpha) - dy*sin(beta)*sin(alpha) - dz*sin(alpha) );

    Vec3f vec3f(x,y,z);

    return vec3f;
}


void Manta::doDynamics()
{
    doDynamics(getBodyID());
}


void Manta::doDynamics(dBodyID body)
{
    Vec3f linearVelInBody = dBodyGetLinearVelInBody(body);
    setVector((float *)&(Manta::V),linearVelInBody);
    Vec3f linearVel = dBodyGetLinearVelVec(body);

    // Get speed, alpha and beta.
    speed = linearVel.magnitude();

    // Wind Frame angles.
    Manta::alpha = Manta::beta = 0;
    float gamma = 0;


    if (linearVel[2]!=0 && speed > 2)
    {
        alpha = -atan2( linearVelInBody[1], linearVelInBody[2] );//atan( VV[1] / VV[2]);
        beta = -atan2( linearVelInBody[0],  linearVelInBody[2]);

        Vec3f side(1.0f,0.0f,0.0f);

        dVector3 result;
        dBodyVectorToWorld(body, 1,0,0,result);

        Vec3f sideinWorld;
        sideinWorld[0] = result[0];
        sideinWorld[1] = result[1];
        sideinWorld[2] = result[2];

        gamma = -atan2( sideinWorld[1], 1);
    }

    //dBodyAddForce(body,0.0,9.81f*(10.0f),0);

    float L=0;
    float D=0;

    L=0.3 * speed;
    D=0.01 * speed;


    float Ml=0;
    float Mn=0;
    float Md=0;

    Mn = -elevator*0.01;

    Ml = -rudder*0.1  - aileron*0.1;
    Ml = -rudder*0.1  + gamma*0.5;

    Md = aileron*0.1 + gamma*0.5;

    airspeddrarestoration();

    if (L>(9.81*10.0)) L = 9.81f * 10.0f;

    dBodyAddRelForce(body, 0.0f, 0.0f, getThrottle() - D);
    dBodyAddForce(body, 0.0f, L,0.0f );
    dBodyAddRelTorque(body, Mn, 0.0f, Md);
    dBodyAddTorque(body, 0.0f, Ml, 0.0f);

    wrapDynamics(body);
}




void Manta::airspeddrarestoration()
{
	// Airspeed drag restoration  (allign forward with linear velocity)
	dReal *v = (dReal *)dBodyGetLinearVel(me);

	// Equal to gravity
	//dBodyAddForce (body[0], 0,0.5,0);

	Vec3f V;
	V[0]=v[0];V[1]=v[1];V[2]=v[2];

	Vec3f Zero;

	Zero[0]=0.01; Zero[1]=0.01;Zero[2]=0.01;

	dVector3 dv3O;
	dBodyGetRelPointPos( me, 0,0,0, dv3O);

	dVector3 dv3F;
	dBodyGetRelPointPos( me, 0,0,1, dv3F);

	dv3F[0] = (dv3F[0]-dv3O[0]);
	dv3F[1] = (dv3F[1]-dv3O[1]);
	dv3F[2] = (dv3F[2]-dv3O[2]);


	Vec3f F;
	F[0] = dv3F[0];F[1] = dv3F[1]; F[2] = dv3F[2];


	Vec3f R = (F.cross(V)/(V.magnitude()))*0.02 ;

	if ( V.magnitude() > Zero.magnitude() )
	{
		dBodyAddTorque(me,R[0],R[1],R[2]);

		dVector3 velResult;
		dBodyVectorFromWorld (me, V[0], V[1], V[2], velResult);

		Vec3f Vl;
		Vl[0]=velResult[0];Vl[1]=velResult[1];Vl[2]=velResult[2];

		Vec3f P;
		P[0]=0;P[1]=P[2]=1.0f/sqrt(2);

		Vec3f K;
		K[0]=0;K[1]=1;K[2]=0;

		Vec3f L;
		L = (fabs(P.dot(Vl))*(1.0f/(Vl.magnitude())))*K;

		dBodyAddRelForce(me,L[0],L[1],L[2]);

		S[0]=L[0];
		S[1]=L[1];
		S[2]=L[2];

		Manta::V[0]=V[0];
		Manta::V[1]=V[1];
		Manta::V[2]=V[2];
	}

}



Vehicle* Manta::fire(int weapon, dWorldID world, dSpaceID space)
{
    Gunshot *action = new Gunshot();
    // Need axis conversion.
    action->init();
    action->setOrigin(me);


    Vec3f position = getPos();
    position[1] += 0.0f; // Move upwards to the center of the real rotation.
    forward = getForward();
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

void Manta::setNameByNumber(int number)
{
    setNumber(number);
    setName("Manta", number);
}

