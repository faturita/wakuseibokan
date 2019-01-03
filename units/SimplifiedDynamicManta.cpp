#include "SimplifiedDynamicManta.h"

void SimplifiedDynamicManta::embody(dWorldID world, dSpaceID space)
{
    me = dBodyCreate(world);
    embody(me);
    //geom = dCreateSphere( space, 2.64f);
    geom = dCreateBox( space, 4.0f, 2.64f, 2.0f);
    dGeomSetBody(geom, me);
}

void SimplifiedDynamicManta::embody(dBodyID myBodySelf)
{
    dMass m;

    float myMass = 10.0f;

    dBodySetPosition(myBodySelf, pos[0], pos[1], pos[2]);
    dMassSetBox(&m,1,4.0f,2.64f,10.0f);
    dMassAdjust(&m, myMass*1.0f);
    dBodySetMass(myBodySelf,&m);

    me = myBodySelf;

    Manta::param[0] = 0.9;
    Manta::param[1] = 2;
}

void SimplifiedDynamicManta::doControl(Controller controller)
{
    //engine[0] = -controller.roll;
    //engine[1] = controller.yaw;
    //engine[2] = -controller.pitch;
    //steering = -controller.precesion;


    setThrottle(-controller.thrust*2*5);

    // roll
    Manta::aileron = controller.roll;

    // pitch
    Manta::elevator = controller.pitch;

    //Manta::rudder = controller.precesion*0.1;

    Manta::rudder = controller.precesion;

    for(int i=0;i<10;i++)
    {
        if (controller.param[i]!=0)
            Manta::param[i] = controller.param[i];
    }
}

void SimplifiedDynamicManta::doDynamics(dBodyID body)
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

    dMatrix3 R,R2;
    dRSetIdentity(R);
    dRFromEulerAngles (R, Manta::elevator*0.005,0,
                      -Manta::aileron*0.01);

    angularPos[0] -= (Manta::rudder*0.01);
    angularPos[0] -= (Manta::aileron*0.001);

    dQuaternion q1,q2,q3;
    dQfromR(q1,R);
    dRFromAxisAndAngle(R2,0,1,0,angularPos[0]);

    dQfromR(q2,R2);

    dQMultiply0(q3,q2,q1);


    if (!Vehicle::letMeGo)
        dBodySetQuaternion(body,q3);

    //dBodySetRotation(body,R);

    // Linear Movement.

    // Check alpha and beta limits before calculating the forces.
    Vec3f forcesOnBody = Fa.rotateOnX(alpha).rotateOnY(beta);

    dBodyAddRelForce(body,forcesOnBody[0],forcesOnBody[1],forcesOnBody[2]);
    dBodyAddRelForce(body,Ft[0],Ft[1],Ft[2]);

    // Adding drag which is OPPOSED to linear movment
    Vec3f dragging = linearVel.normalize();
    dragging = linearVel*(-(abs(D*0.01)));

    dBodyAddForce(body,dragging[0],dragging[1],dragging[2]);

    //printf ("%5.2f/%2.4+f/%2.4+f/Cm=%5.2f/F=(%5.2f,%5.2f,%5.2f)\n", speed,alpha, beta, Cm, forcesOnBody[0],forcesOnBody[1],forcesOnBody[2]);

    wrapDynamics(body);

}
