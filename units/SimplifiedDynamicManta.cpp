#include "SimplifiedDynamicManta.h"

#include "../actions/Gunshot.h"

SimplifiedDynamicManta::SimplifiedDynamicManta(int newfaction)
{
    setFaction(newfaction);
}

void SimplifiedDynamicManta::embody(dWorldID world, dSpaceID space)
{
    me = dBodyCreate(world);
    embody(me);
    //geom = dCreateSphere( space, 2.64f);
    geom = dCreateBox( space, 8.0f,1.6f,4.0f);
    dGeomSetBody(geom, me);
}

void SimplifiedDynamicManta::embody(dBodyID myBodySelf)
{
    dMass m;

    float myMass = 10.0f;

    dBodySetPosition(myBodySelf, pos[0], pos[1], pos[2]);
    dMassSetBox(&m,1,8.0f,1.6f,4.0f);
    dMassAdjust(&m, myMass*1.0f);
    dBodySetMass(myBodySelf,&m);

    me = myBodySelf;

    Manta::param[0] = 0.9;
    Manta::param[1] = 2;
}

void SimplifiedDynamicManta::doControl()
{
    doControl(myCopy);
}

void SimplifiedDynamicManta::doControl(Controller controller)
{
    doControl(controller.registers);

    for(int i=0;i<10;i++)
    {
        if (controller.param[i]!=0)
            Manta::param[i] = controller.param[i];
    }
}


void SimplifiedDynamicManta::doControl(struct controlregister regs)
{
    if (regs.thrust>150.0f)
        regs.thrust=150.0f;

    setThrottle(regs.thrust*10.0f);  // Use the mass of Manta

    if (getThrottle()>200 && inert)
    {
        Manta::inert = false;
        antigravity = false;
        setStatus(Manta::TACKINGOFF);
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

void SimplifiedDynamicManta::flyingCoefficients(float &Cd, float &CL, float &Cm, float &Cl, float &Cy, float &Cn)
{
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

    printf ("Cd=%10.8f, CL=%10.8f,Cy=%10.8f,Cm=%10.8f,Cl=%10.8f,Cn=%10.8f\t", Cd, CL, Cy, Cm, Cl,Cn);
}

void SimplifiedDynamicManta::rotateBody(dBodyID body)
{
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


    if (!Vehicle::inert)
        dBodySetQuaternion(body,q3);



}

void SimplifiedDynamicManta::doDynamics(dBodyID body)
{
    if (!Vehicle::inert)
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


        rotateBody(body);

        if (Manta::antigravity)
            dBodyAddForce(body,0,9.81f*(10.0f),0);

        // Airflow parameters.
        float density = 0.1f;   // Air density
        float Sl = 0.9;   // Wing surface [m2]
        float b = 0.01;   // Wing span [m]

        float Cd = 0.01, CL = 0.01;


        Cd = 0.9f    + 0.5f * alpha + 0.9*beta;
        CL = 0.1f + 0.8f * alpha + 0.03 * ih + 0.002 * elevator + 0.2*flaps + 0.02*spoiler;

        float q = density * speed * speed / 2.0f;
        float D = Cd * q * Sl;
        float L = CL * q * Sl;


        if (D>1) D=0.9;

        Vec3f Fa(0.0f, L, -(D*speed));
        Vec3f forcesOnBody = Fa.rotateOnX(alpha).rotateOnY(beta);


        Vec3f Ft;
        Ft[0]=0;Ft[1]=0;Ft[2]=getThrottle();

        Ft[0] = Ft[0] + forcesOnBody[0];
        Ft[1] = Ft[1] + forcesOnBody[1];
        Ft[2] = Ft[2] + forcesOnBody[2];

        dBodyAddRelForce(body, Ft[0],Ft[1],Ft[2]);

        //printf("%8.4f\t%8.4f\t%6.4f\t",speed, alpha*180.0/PI, beta*180.0/PI);
        //std::cout << Ft << std::endl;
    }

    wrapDynamics(body);

}

/**
void SimplifiedDynamicManta::doDynamics(dBodyID body)
{
    if (!Vehicle::inert)
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


        //rotateBody(body);

        if (Manta::antigravity)
            dBodyAddForce(body,0,9.81f*(10.0f),0);

        // Airflow parameters.
        float density = 0.1f;   // Air density
        float Sl = 0.9;   // Wing surface [m2]
        float b = 0.01;   // Wing span [m]

        float Cd = 0.01, CL = 0.01;


        Cd = 0.9f    + 0.5f * alpha + 0.9*beta;
        CL = 0.1f + 0.8f * alpha + 0.03 * ih + 0.002 * elevator + 0.2*flaps + 0.02*spoiler;

        float q = density * speed * speed / 2.0f;
        float D = Cd * q * Sl;
        float L = CL * q * Sl;


        if (D>1) D=0.9;

        Vec3f Fa(0.0f, L, -(D*speed));
        Vec3f forcesOnBody = Fa.rotateOnX(alpha).rotateOnY(beta);


        Vec3f Ft;
        Ft[0]=0;Ft[1]=0;Ft[2]=getThrottle();

        Ft[0] = Ft[0] + forcesOnBody[0];
        Ft[1] = Ft[1] + forcesOnBody[1];
        Ft[2] = Ft[2] + forcesOnBody[2];

        dBodyAddRelForce(body, Ft[0],Ft[1],Ft[2]);


        Vec3f rot;

        rot[0] = alpha*0.1 - Manta::elevator * 0.01;

        rot[1]= gamma * 1- Manta::rudder * 0.1;

        rot[2]= +gamma*0.1 + beta*0.1 + Manta::aileron * 0.1;

        dBodyAddRelTorque(body, rot[0],rot[1],rot[2]);

        printf("%8.4f\t%8.4f\t%6.4f\t%6.4f\t",speed, alpha*180.0/PI, beta*180.0/PI, gamma*180.0/PI);
        //std::cout << Ft << std::endl;
        std::cout << std::endl;
    }

    wrapDynamics(body);

}**/

/**
void SimplifiedDynamicManta::doDynamics(dBodyID body)
{
    if (!Vehicle::inert)
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

        flyingCoefficients(Cd, CL, Cm, Cl, Cy, Cn);

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
        Fa[2] = -abs(D);

        // Lateral Force on Structure
        Fa[0] = (- (Cy * q * Sl))*0;

        // Lift on Structure
        Fa[1] = (+ (+abs(L)));

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


        if (!Vehicle::inert)
            dBodySetQuaternion(body,q3);

        if (Manta::antigravity)
            dBodyAddForce(body,0,9.81f*(10.0f),0);

        //dBodySetRotation(body,R);

        // Linear Movement.

        // Check alpha and beta limits before calculating the forces.
        Vec3f forcesOnBody = Fa.rotateOnX(alpha).rotateOnY(beta);

        Ft[0] = Ft[0] + Fa[0];
        Ft[1] = Ft[1] + Fa[1];
        Ft[2] = Ft[2] + Fa[2];

        if (isnan(Ft[0])) Ft[0]=0;
        if (isnan(Ft[1])) Ft[1]=0;
        if (isnan(Ft[2])) Ft[2]=0;

        if (abs(Ft[0])>100000) Ft[0]=(Ft[0]>0?1:-1)*100000;
        if (abs(Ft[1])>100000) Ft[1]=(Ft[1]>0?1:-1)*100000;
        if (abs(Ft[2])>100000) Ft[2]=(Ft[2]>0?1:-1)*100000;


        dBodyAddRelForce(body,Ft[0],Ft[1],Ft[2]);

        // Adding drag which is OPPOSED to linear movment
        Vec3f dragging;
        dragging = linearVel*(-(abs(D*0.01)));


        dragging = -linearVel*10;

        //dBodyAddForce(body,dragging[0],dragging[1],dragging[2]);

        printf ("%5.2f/%2.4+f/%2.4+f/Cm=%5.2f/F=(%5.2f,%5.2f,%5.2f)\n", speed,alpha*180/PI, beta*180/PI, Cm, Ft[0],Ft[1],Ft[2]);

    }
    wrapDynamics(body);

}
**/


Vehicle* SimplifiedDynamicManta::fire(dWorldID world, dSpaceID space)
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

