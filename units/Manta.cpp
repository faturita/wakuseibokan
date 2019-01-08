/*
 * Manta.cpp
 *
 *  Created on: Jan 16, 2011
 *      Author: faturita
 */

#define dSINGLE

#include "Manta.h"
#include "../md2model.h"

void Manta::init()
{
	//Load the model
    _model = MD2Model::loadModel("mantagood.md2");
    if (_model != NULL)
        _model->setAnimation("run");

    setForward(0,0,1);

}

int Manta::getType()
{
    return 3;
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
        drawArrow(V[0],V[1],V[2],0.0,1.0,0.0);

        //drawRectangularBox(16.0f/2.0f, 5.2f/2.0f, 8.0f/2.0f);

        glRotatef(180.0f, 0.0f, 0.0f, 1.0f);
		glRotatef(-180.0f, 1.0f, 0.0f, 0.0f);

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

    float myMass = 1.0f;
    float radius = 2.64f;
    float length = 7.0f;

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

void Manta::doControl(Controller controller)
{
    //engine[0] = -controller.roll;
    //engine[1] = controller.yaw;
    //engine[2] = -controller.pitch;
    //steering = -controller.precesion;
    
    
    setThrottle(-controller.registers.thrust*2*5);
    
    // roll
    if (controller.registers.roll>4) controller.registers.roll = 4;
    if (controller.registers.roll<-4) controller.registers.roll = -4;
    Manta::aileron = controller.registers.roll;
    
    // pitch
    Manta::elevator = controller.registers.pitch*0.1;
    
    //Manta::rudder = controller.precesion*0.1;
    
    Manta::rudder = controller.registers.precesion*0.1;
    
    for(int i=0;i<10;i++)
    {
        if (controller.param[i]!=0)
            Manta::param[i] = controller.param[i];
    }

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





/**
void Manta::doDynamics(dBodyID body)
{
	// Calculate the force
	Vec3f Fa;

	Fa[2] = getThrottle();
	Fa[1] = 0.0f;
	Fa[0] = 0.0f;

	dBodyAddRelForce(body,Fa[0],Fa[1],Fa[2]);

	// Calculate the lift

	dReal *v = (dReal *)dBodyGetLinearVel(body);

	Vec3f vLinearVelocity;
	vLinearVelocity = ToVec3f((dReal *)dBodyGetLinearVel(body),vLinearVelocity);

	speed = vLinearVelocity.magnitude();

    Vec3f vForward;
    vForward = dBodyVectorToWorldVec3f(body,0,0,1,vForward);

    float lift = 0.03*speed*vLinearVelocity.normalize().dot(vForward);


    //dBodyAddRelForce(body,0,S(lift),0);

    dBodyAddForce(body,0,0.5,0);  // Gravity is avoided


	// Calculate the drag
    Vec3f D = -0.5*vLinearVelocity;

	dBodyAddRelForce(body,D[0],D[1],D[2]);



	// Roll and pitch
	dBodyAddRelTorque( body, 0, 0, xRotAngle*0.001 );
	dBodyAddRelTorque( body, -yRotAngle*0.001, 0,0);
	//dBodyAddRelTorque( body[0], 0, -modAngleX*0.0025,0);

	dReal *angulardumping = (dReal *)dBodyGetAngularVel(body);

	dReal dFactor = 0.05;

	// Angular dumping...
	dBodyAddTorque(body,angulardumping[0]*-dFactor,angulardumping[1]*-dFactor,angulardumping[2]*-dFactor );

    // Yaw translation.
	//dBodyAddRelForceAtRelPos(body,0,0,-0.0002,-xRotAngle*100.0/20, 0,0);



	// This should be after the world step
	/// stuff
    dVector3 result;

    dBodyVectorToWorld(body, 0,0,1,result);
    setForward(result[0],result[1],result[2]);

	const dReal *dBodyPosition = dBodyGetPosition(body);
	const dReal *dBodyRotation = dBodyGetRotation(body);

	setPos(dBodyPosition[0],dBodyPosition[1],dBodyPosition[2]);
	setLocation((float *)dBodyPosition, (float *)dBodyRotation);

}**/


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


void Manta::wrapDynamics(dBodyID body)
{
    dVector3 result;

    dBodyVectorToWorld(body, 0,0,1,result);
    setForward(result[0],result[1],result[2]);

    const dReal *dBodyPosition = dBodyGetPosition(body);
    const dReal *dBodyRotation = dBodyGetRotation(body);


    setPos(dBodyPosition[0],dBodyPosition[1],dBodyPosition[2]);
    setLocation((float *)dBodyPosition, (float *)dBodyRotation);
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


    if (linearVel[2]!=0 && speed > 2)
    {
        alpha = -atan2( linearVelInBody[1], linearVelInBody[2] );//atan( VV[1] / VV[2]);
        beta = -atan2( linearVelInBody[0],  linearVelInBody[2]);
    }

    //if (alpha>=PI/2) alpha = PI/2;
    //if (alpha<=-PI/2) alpha = -PI/2;

    //if (beta>=PI/2) beta = PI/2;
    //if (beta<=-PI/2) beta = -PI/2;

    // Airflow parameters.
    float density = 0.1f;   // Air density
    float Sl = 0.9;   // Wing surface [m2]
    float b = 0.01;   // Wing span [m]

    // Airplane control coefficients
    float Cd, CL, Cm, Cl, Cy, Cn;


    const float caileron=0.2f;



    if (speed == 0)
    {
        Cd = CL = 0;

    }
    else
    {
        // Drag
        Cd = 0.9f    + 0.5f * alpha   + 0.3* ih + 0.002 * elevator + 0.2*flaps + 0.02*spoiler;

        // Lift (The independent parameter determines the lift of the airplane)
        CL = 0.1f + 0.8f * alpha + 0.03 * ih + 0.002 * elevator + 0.2*flaps + 0.02*spoiler;
    }

    // Yaw
    Cy = 0.0000f + 0.5f * beta + caileron * aileron + 0.07 * rudder;

    // Rolling
    Cm = 0.0000f + 0.005f * alpha + 0.03 * ih + 0.002 * elevator + 0.2*flaps + 0.02*spoiler;

    Cl = 0.0000f + 0.5f * beta + caileron * aileron + 0.07 * rudder;

    Cn = 0.0000f + 0.5f * beta + caileron * aileron + 0.07 * rudder;

    //printf ("Cd=%10.8f, CL=%10.8f,Cy=%10.8f,Cm=%10.8f,Cl=%10.8f,Cn=%10.8f\n", Cd, CL, Cy, Cm, Cl,Cn);

    float q = density * speed * speed / 2.0f;

    float La;
    float Ma;
    float Na;

    Vec3f Fa;

    float Lt;
    float Mt;
    float Nt;

    Vec3f Ft;

    Ft[0]=0;Ft[1]=0;Ft[2]=getThrottle();

    Lt=Mt=Nt=0;

    float D = Cd * q * Sl;
    float L = CL * q * Sl;

    // Drag from Structure
    Fa[2] = 0;

    // Lateral Force on Structure
    Fa[0] = (- (Cy * q * Sl))*0;

    // Lift on Structure
    Fa[1] = (+ (+L));

    //dBodyAddForce(me,0,9.81f,0);
    //dBodyAddRelForce(me,0,0,getThrottle());
    //dBodyAddRelTorque(body, -Manta::elevator, -Manta::rudder, Manta::aileron);

    // y Rolling rotation
    Ma = +( Cm * q * Sl )*1;

    // x Around lift
    La =  -(Cl * q * Sl * b)*1;

    // z Around yaw
    Na =  (Cn * q * Sl * b)*0.01;

    Vec3f rtWind;
    rtWind[1] = La;
    rtWind[0] = Ma;
    rtWind[2] = Na;


    // Check alpha and beta limits before calculating the forces.
    Vec3f forcesOnBody = Fa;Fa.rotateOnX(alpha).rotateOnY(beta);

    dBodyAddRelForce(body,forcesOnBody[0],forcesOnBody[1],forcesOnBody[2]);
    dBodyAddRelForce(body,Ft[0],Ft[1],Ft[2]);


    // Adding drag which is OPPOSED to linear movment
    Vec3f dragging = linearVel.normalize();
    dragging = linearVel*(-(abs(D*0.01)));

    dBodyAddForce(body,dragging[0],dragging[1],dragging[2]);


    // Airrestoration.
    Vec3f fw = getForward();

    Vec3f restoration = fw.cross(linearVel);
    dBodyAddTorque(body, restoration[0]*Manta::param[1],restoration[1]*Manta::param[1],restoration[2]*Manta::param[1]);

    Vec3f torquesOnBody=rtWind.rotateOnX(alpha).rotateOnY(beta);
    dBodyAddRelTorque(body, torquesOnBody[0], torquesOnBody[1], torquesOnBody[2]*Manta::param[0]);

    printf ("%5.2f/%2.4+f/%2.4+f/Cm=%5.2f/F=(%5.2f,%5.2f,%5.2f),M=(%5.2f,%5.2f,%5.2f)\n", speed,alpha, beta, Cm, forcesOnBody[0],forcesOnBody[1],forcesOnBody[2],torquesOnBody[0],torquesOnBody[1],torquesOnBody[2]);

    wrapDynamics(body);
}

/**
void Manta::doDynamics(dBodyID body)
{
    
    Vec3f Ft,vec3fV;
    
    Ft[0]=0;Ft[1]=0;Ft[2]=getThrottle();
    
    dBodyAddRelForce(body, 0,0,getThrottle());
    dReal *v = (dReal *)dBodyGetLinearVel(body);

    Vec3f linearVelInBody = dBodyGetLinearVelInBody(body);
    setVector((float *)&(Manta::V),linearVelInBody);


    // Airrestoration.
    Vec3f linearVel = dBodyGetLinearVelVec(body);
    Vec3f fw = getForward();

    Vec3f restoration = fw.cross(linearVelInBody);

    dBodyAddTorque(body, restoration[0]*0.01,restoration[1]*0.01,restoration[2]*0.01);




    //dBodyAddRelTorque(body,0,0,Manta::addd);
    dBodyAddRelTorque(body,0,-Manta::addd*0.1,0);
    dBodyAddRelTorque(body, 0,0,xRotAngle*0.05);

    dBodyAddRelTorque(body, -yRotAngle*0.1,0,0);

    Vec3f velResultBody = dBodyGetAngularVelInBody(body);

    //dBodyAddRelTorque(body, 0,0,beta);
    //dBodyAddRelTorque(body,-velResultBody[0]*0.1,-velResultBody[1]*0.1,-velResultBody[2]*0.1);

    float alpha = getlinearforce(body);


    dVector3 vHorizon;
    dBodyVectorToWorld (body, 1, 0, 0, vHorizon);

    dBodyAddTorque(body,0,-vHorizon[1]*0.8,0);

    float lift = speed * speed * 0.5*alpha;
    if (lift<0)
        lift = 0.0;
    if (lift>9.81)
        lift = 9.82;
    dBodyAddForce(me,0,lift,0);


    if (vec3fV[2]!=0)
    {
        //alpha = -atan( vec3fV[1] / vec3fV[2] );//atan( VV[1] / VV[2]);
    }

    printf("%5.2f,%5.2f,%5.2f,%5.2f,(%5.2f,%5.2f,%5.2f),%5.2f\n",lift,vHorizon[1],alpha,beta,velResultBody[0],velResultBody[1],velResultBody[2],speed);

    //if (speed > 60)
    {
        //printf ("Antigravity...\n");
        //dBodyAddForce(me, 0,9.81f,0);
    }



    // This should be after the world step
    /// stuff
    dVector3 result;
    
    dBodyVectorToWorld(body, 0,0,1,result);
    setForward(result[0],result[1],result[2]);
    
    const dReal *dBodyPosition = dBodyGetPosition(body);
    const dReal *dBodyRotation = dBodyGetRotation(body);
    
    setPos(dBodyPosition[0],dBodyPosition[1],dBodyPosition[2]);
    setLocation((float *)dBodyPosition, (float *)dBodyRotation);
}
**/




/**
void Manta::doDynamics(dBodyID body)
{
	dReal *v = (dReal *)dBodyGetLinearVel(body);

	//dBodyAddRelForce (body[0], 0,0.9,0);

	Vec3f V;
	V[0]=v[0];V[1]=v[1];V[2]=v[2];

	dVector3 velResult;
	dBodyVectorFromWorld (body, V[0], V[1], V[2], velResult);

	Vec3f VV;
	VV[0]=velResult[0];VV[1]=velResult[1];VV[2]=velResult[2];

	//VV = (VV)/ VV.magnitude();

	V[0]=VV[0];V[1]=VV[1];V[2]=VV[2];

	Vec3f F;
	F[0]=0;F[1]=0;F[2]=1;

    Vec3f vForward;
    vForward = dBodyVectorToWorldVec3f(body,0,0,1,vForward);

	float vel = V.dot(vForward);

	speed = V.magnitude();

	float elevator=0, alpha=0, rudder=0, ih=0, flaps=0, spoiler=0,aileron=0;
	float beta=0;

	//alpha = modAngleY;
	//beta = modAngleX;

	if (V[2]!=0)
	{
		alpha = -atan( V[1] / V[2] );//atan( VV[1] / VV[2]);
		beta = asin( V[0] / speed );//atan( VV[0] / VV[2]);
	}

	ih = (-yRotAngle);

	aileron = (-xRotAngle);

	rudder = Manta::rudder;//modAngleP;

	elevator = -Manta::elevator;//modAngleZ;


	float Cd, CL, Cm, Cl, Cy, Cn;

	float density = 0.1f;

	float S = 0.9;
	float b = 0.01;



	// Drag
	Cd = 0.9f    + 2.4f * alpha   + 15.0f* ih + 0.002 * elevator + 0.2*flaps + 0.02*spoiler;

	// Lift
	CL = 0.1f + 0.8f * alpha + 0.03 * ih + 0.002 * elevator + 0.2*flaps + 0.02*spoiler;

	// Yaw
	Cy = 0.0000f + 0.5f * beta + 0.8 * aileron + 0.07 * rudder;

	Cm = 0.0000f + 0.005f * alpha + 0.03 * ih + 0.002 * elevator + 0.2*flaps + 0.02*spoiler;

	Cl = 0.0000f + 0.5f * beta + 0.8 * aileron + 0.07 * rudder;

	Cn = 0.0000f + 0.5f * beta + 0.8 * aileron + 0.07 * rudder;


	float q = density * speed * speed / 2.0f;
	float La;
	Vec3f Fa;
	float Ma;
	float Na;

	float Lt;
	float Mt;
	float Nt;

	Vec3f Ft;

	Ft[0]=0;Ft[1]=0;Ft[2]=getThrottle();

	Lt=Mt=Nt=0;

	float D = Cd * q * S;
	float L = CL * q * S;

	//dBodyAddForce(body,0.0f,0.5f,0.0f);

	Fa[2] = (+ (-D));

	Fa[1] = (- (-L));

	Fa[0] = ((Cy * q * S))*1;


	Ma =  Cm * q * S;

	La =  (Cl * -q * S * b)*1;

	Na =  (Cn * q * S * b)*4;


	Vec3f rtWind;
	rtWind[0] = Ma;
	rtWind[1] = Na;
	rtWind[2] = La;

	Vec3f rtPlane = toVectorInFixedSystem(rtWind[0],rtWind[1],rtWind[2], alpha, beta);

	Vec3f fPlane = toVectorInFixedSystem(Fa[0],Fa[1],Fa[2], alpha, beta);


	//Vec3f rtPlane = rtWind.rotateOnY(alpha).rotateOnZ(beta).rotateOnZ(alpha).rotateOnX(alpha);

	///Vec3f fPlane = Fa.rotateOnY(alpha).rotateOnZ(beta).rotateOnZ(alpha).rotateOnX(alpha);

	dBodyAddRelForce(body,fPlane[0],fPlane[1],fPlane[2]);

	dBodyAddRelForce(body,Ft[0],Ft[1],Ft[2]);

	Manta::S[0]=Ft[0]+fPlane[0];Manta::S[1]=Ft[1]+fPlane[1];Manta::S[2]=Ft[2]+fPlane[2];

	Manta::V[0]=V[0];Manta::V[1]=V[1];Manta::V[2]=V[2];


	//printf ("%10.8f/%10.8f/F=(%10.8f,%10.8f,%10.8f)\n", alpha, beta, fPlane[0],fPlane[1],fPlane[2]);

	dBodyAddRelTorque(body, rtPlane[0], rtPlane[1], rtPlane[2]);
	//dBodyAddRelTorque(body[0], Mt, Nt, Lt);

	// Angular dumping...
	dReal *angulardumping = (dReal *)dBodyGetAngularVel(body);
	dBodyAddTorque(body,angulardumping[0]*-0.1,angulardumping[1]*-0.1,angulardumping[2]*-0.1 );

	// This should be after the world step
	/// stuff
    dVector3 result;

    dBodyVectorToWorld(body, 0,0,1,result);
    setForward(result[0],result[1],result[2]);

	const dReal*180.0f/PI *dBodyPosition = dBodyGetPosition(body);
	const dReal *dBodyRotation = dBodyGetRotation(body);


	printf ("%10.8f/%10.8f/F=(%10.8f,%10.8f,%10.8f)\n", alpha, beta, dBodyPosition[0],dBodyPosition[1],dBodyPosition[2]);


	setPos(dBodyPosition[0],dBodyPosition[1],dBodyPosition[2]);
	setLocation((float *)dBodyPosition, (float *)dBodyRotation);
}
**/

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

