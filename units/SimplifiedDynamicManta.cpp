#include "SimplifiedDynamicManta.h"

#include "../control.h"

#include "../actions/Gunshot.h"

extern dWorldID world;
extern dSpaceID space;
#include "../container.h"
#include "../sounds/sounds.h"

extern container<Vehicle*> entities;

SimplifiedDynamicManta::SimplifiedDynamicManta(int newfaction) : Manta(newfaction)
{

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
    Manta::param[3] = 0;
}

void SimplifiedDynamicManta::doControl()
{
    std::cout << "Number:" << getNumber() << ";" << aistatus << std::endl;

    switch (aistatus) {
    case ATTACK:
        doControlAttack();
        break;
    case LANDING:
        doControlLanding();
        break;
    case DESTINATION:
        doControlControl(destination, 3000);// @FIXME
        break;
    case FREE:
        setDestination(getPos()+getForward().normalize()*100);
        break;
    default:
        break;
    }
}

void SimplifiedDynamicManta::doControlAttack()
{

    std::cout << "Attacking:" << getNumber() << ";" << destination << ":" << flyingstate << std::endl;

    Vec3f target = destination;
    switch (flyingstate) {
        case 0:
        {
            target = Vec3f(destination[0],1000,destination[2]);
            doControlFlipping(target,500);
            Vec3f loc(destination[0],1000,destination[2]);
            Vec3f ploc(getPos()[0],1000,getPos()[2]);
            if ((loc-ploc).magnitude()<8500)
                flyingstate = 1;
        }
        break;
        case 1:
        {
            target = destination;
            doControlFlipping(target, 500);
            if ((destination-getPos()).magnitude()<2500)
                flyingstate = 2;

            if (getTtl() % 23 == 0)
            {
            Vehicle *action = fire(world,space);

            if (action != NULL)
            {
                entities.push_back(action);
                gunshot();
            }
            }
        }
        break;
    case 2:
        waypoint = destination + getForward().normalize()*(17000);
        waypoint[1] = 1000;
        flyingstate = 3;
        break;
    case 3:
        doControlFlipping(waypoint,10000);
        if (((waypoint-getPos()).magnitude()<5000))
                flyingstate = 0;
        break;

    }


    //std::cout << "Azimuth:" << getAzimuth(getForward()) << "- Declination: " << getDeclination(getForward())  << "- Destination:" << (destination-getPos()).magnitude() << std::endl;


}

void SimplifiedDynamicManta::doControlForced(Vec3f target)
{
    Controller c;

    c.registers = myCopy;

    Vec3f Po = getPos();
    Vec3f maptaget(target[0],0,target[2]);
    Vec3f mapPo(Po[0],0,Po[2]);

    float pitch = -20;

    if ((maptaget-mapPo).magnitude()>6500)
    {
        target[1]=900;
        pitch = 0;
    }


    Vec3f T = (target - Po);

    setForward(T);
    release(T);

    std::cout << "Number:" << getNumber() << "-Azimuth:" << getAzimuth(getForward()) << "- Declination: " << getDeclination(getForward())  << "- Destination:" << T.magnitude() << std::endl;

    c.registers.thrust = 400/(10.0);
    c.registers.pitch = pitch;
    c.registers.roll = 0;
    c.registers.yaw = 0;
    setThrottle(400);

    doControl(c);

    setOrientation(T);

}


void SimplifiedDynamicManta::doControlFlipping(Vec3f target, float thrust)
{

    Controller c;

    c.registers = myCopy;

    Vec3f Po = getPos();

    float height = Po[1];

    float declination = getDeclination(getForward());

    float sp2=-0,        sp3 = 720;

    Vec3f T = (target - Po);

    sp2 = getDeclination(T);

    float e1 = acos(  T.normalize().dot(getForward().normalize()) );
    float e2 = sp2 - declination;
    float e3 = sp3 - height;

    // Needs fixing, check azimuth to make a continuos function.

    float Kp1 = 0.42,    Kp2 = 2.98,     Kp3 = 0.2;
    float Ki1 = 1,      Ki2 = 0.11,        Ki3 = 0;
    float Kd1 = 0.8,      Kd2 = 0.03,        Kd3 = 0;

    float I[3]= {0,0,0};

    float e[3] = { e1, e2, e3};

    getIntegrativeTerm(signal,I,e);

    float In[3] = {0,0,0};

    In[0] = I[0] * 0.99 + (e1);
    In[1] = I[1] * 0.99 + (e2);
    In[2] = I[2] * 0.99 + (e3);




    if (e2>0)
        midpointpitch -= 0.1;
    else if (e2<0)
        midpointpitch += 0.1;


    if ((abs(e2))>10.0f)
    {
        c.registers.pitch = Ki2 * (In[1]) + Kd2 * (  (e2)-et2) + midpointpitch+1.0 * (e2>0 ? -1 : +1);
    } else {
        c.registers.pitch = midpointpitch;
    }

    setForward(T);
    release(T);

    std::cout << "Azimuth:" << getAzimuth(getForward()) << "/" << e1 << ":Declination: " << declination << "/" << sp2 << " Destination:" << T.magnitude() << std::endl;

    et1 = e1;
    et2 = e2;
    et3 = e3;

    c.registers.thrust = thrust/(10.0);
    c.registers.yaw = 0;
    c.registers.roll = 0;
    setThrottle(thrust);

    doControl(c);
}


/**
 * 1 Heuristic procedure #1:
 * 2 Set Kp to small value, KD and KI to 0
 * 3 Increase KD until oscillation, then decrease by factor of 2-4
 * 4 Increase Kp until oscillation or overshoot, decrease by factor of 2-4
 * 5 Increase KI until oscillation or overshoot
 * Iterate
 *
 * @brief SimplifiedDynamicManta::doControlAttack
 */
void SimplifiedDynamicManta::doControlControl(Vec3f target, float thrust)
{

    Controller c;

    c.registers = myCopy;

    Vec3f Po = getPos();

    float height = Po[1];
    float declination = getDeclination(getForward());

    float sp2=-0,        sp3 = 720;

    Vec3f T = (target - Po);

    sp2 = getDeclination(T);

    // Needs fixing, check azimuth to make a continuos function.

    float e1 = acos(  T.normalize().dot(getForward().normalize()) );
    float e2 = sp2 - declination;
    float e3 = sp3 - height;



    float Kp1 = 0.42,    Kp2 = 2.98,     Kp3 = 0.2;
    float Ki1 = 0.1,      Ki2 = 0.11,        Ki3 = 0;
    float Kd1 = 0.003,      Kd2 = 0.03,        Kd3 = 0;

    float I[3]= {0,0,0};

    float e[3] = { e1, e2, e3 };

    getIntegrativeTerm(signal,I,e);

    float In[3] = {0,0,0};

    In[0] = I[0] * 0.99 + (e1);
    In[1] = I[1] * 0.99 + (e2);
    In[2] = I[2] * 0.99 + (e3);

    float error1 =  Kp1 * (e1)        + Ki1 * (In[0]) + Kd1 * (  (e1)-et1);
    float error2 =  Kp2 * (e2)        + Ki2 * (In[1]) + Kd2 * (  (e2)-et2);
    float error3 =  Kp3 * (e3)        + Ki3 * (In[1]) + Kd3 * (  (e3)-et3);

    std::cout << "Azimuth:" << getAzimuth(getForward()) << "/" << error1 << "- Declination: " << declination << "/" << sp2 << " Destination:" << T.magnitude() << std::endl;

    float roll = -error1;
    float pitch = -error2 + error3*0;

    //float thrust = (e3>800? e3 :800);

    et1 = e1;
    et2 = e2;
    et3 = e3;


    c.registers.thrust = thrust/(10.0);
    c.registers.pitch = pitch;
    c.registers.roll = roll;
    c.registers.yaw = 0;
    setThrottle(thrust);

    doControl(c);

}



void SimplifiedDynamicManta::doControlLanding()
{
    Controller c;

    c.registers = myCopy;

    Vec3f Po = getPos();

    float height = Po[1];

    Po[1] = 0.0f;

    Vec3f Pf(-100 kmf, 0.0f, 100 kmf);

    Pf = destination;  // This is the carrier or a runway.  If it is the carrier this must be modified during each tick.
    Pf[1] = 0;

    // Go 10 kmf backwards from the carrier, very slowly.
    Vec3f T;
    float H=500, spspeed = 40.0f, TH = 200;

    switch (flyingstate) {
    case 0:
        T = (Pf - attitude.normalize()*(2 kmf)) - Po;
        H=450;spspeed = 40.0f;TH = 150;
        break;
    case 1:
        H=180;spspeed = 27.0f; TH=250;
        T = (Pf - attitude.normalize()*(0 kmf)) - Po;
        break;
    }


    float eh, midpointpitch, distance;

    if (!reached && T.magnitude()>TH)
    {
        float Kp = -0.8;
        float val = ((getAzimuth(getForward())-180.0f)*PI/180.0f - (getAzimuth(T)-180.0f)*PI/180.0f);

        distance =  T.magnitude();
        c.registers.roll = 0;

        setForward(T);
        release(T);


        printf("Landing:T: %10.3f %10.3f %10.3f\n", distance, val, c.registers.roll);


        eh = height-H;
        c.registers.thrust = spspeed;
        setThrottle(spspeed * 10.0f);
        midpointpitch = 2;

        if ((abs(eh))>10.0f)
        {
            c.registers.pitch = midpointpitch+1.0 * (eh>0 ? -1 : +1);
        } else {
            c.registers.pitch = midpointpitch;
        }
    } else
    {
        if (flyingstate<1)
            flyingstate += 1;
        else
        {
            //stop();
            elevator = 0;

            c.registers.thrust = 0.0f/(10.0);
            c.registers.pitch = 0;
            c.registers.roll = 0;
            setThrottle(0.0f);

            if (!reached)
            {
                flyingstate = 0;
                char str[256];
                sprintf(str, "Manta %d has arrived to destination.", getNumber()+1);
                //messages.insert(messages.begin(), str);
                reached = true;
            }
        }
    }

    doControl(c);

}

void SimplifiedDynamicManta::doControlDestination()
{
    Controller c;

    c.registers = myCopy;

    Vec3f Po = getPos();

    float height = Po[1];

    Po[1] = 0.0f;

    Vec3f Pf(-100 kmf, 0.0f, 100 kmf);

    Pf = destination;

    Vec3f T = Pf - Po;

    float eh, midpointpitch;


    if (!reached && T.magnitude()>precission)
    {
        float distance = T.magnitude();

        Vec3f F = getForward();

        F = F.normalize();
        T = T.normalize();


        float e = acos(  T.dot(F) );

        float signn = T.cross(F) [1];


        printf("T: %10.3f %10.3f %10.3f\n", distance, e, signn);

        if (abs(e)>=0.5f)
        {
            c.registers.roll = 3.0 * (signn>0?+1:-1) ;
        } else
        if (abs(e)>=0.4f)
        {
            c.registers.roll = 2.0 * (signn>0?+1:-1) ;
        } else
        if (abs(e)>=0.2f)
            c.registers.roll = 1.0 * (signn>0?+1:-1) ;
        else {
            c.registers.roll = 0.0f;
        }

        eh = height-200.0f;
        midpointpitch = -15;
        c.registers.thrust = 150.0f;

        if (distance<10000.0f)
        {
            c.registers.thrust = 30.0f;
            midpointpitch = 17;
        }


        if ((abs(eh))>10.0f)
        {
            c.registers.pitch = midpointpitch+1.0 * (eh>0 ? -1 : +1);
        } else {
            c.registers.pitch = midpointpitch;
        }

    } else
    {
        printf("Manta arrived to destination...\n");

        elevator = 35;

        c.registers.thrust = 300.0f/(10.0);

        midpointpitch = 36;
        eh=height-500.0f;

        c.registers.roll = -13;

        if ((abs(eh))>10.0f)
        {
            c.registers.pitch = midpointpitch+1.0 * (eh>0 ? -1 : +1);
        } else {
            c.registers.pitch = midpointpitch;
        }

        setThrottle(30.0f);

                    setStatus(Manta::HOLDING);

        if (!reached)
        {
            char str[256];
            sprintf(str, "Manta %d has arrived to destination.", getNumber()+1);
            //messages.insert(messages.begin(), str);
            reached = true;
            setStatus(Manta::HOLDING);
        }


    }

    doControl(c);
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

    if (speed>150 && getStatus()!=HOLDING)
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

    myCopy = regs;

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

void SimplifiedDynamicManta::land()
{
    aistatus = LANDING;
}

void SimplifiedDynamicManta::attack(Vec3f target)
{
    aistatus = ATTACK;
    destination = target;
    flyingstate=0;
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

    float x = getAzimuthRadians(getForward());

    //std::cout << "Bearing:" << getBearing() << "," << x << " Anglular Pos:" << angularPos[0] << std::endl;

    if (!Vehicle::inert)
        dBodySetQuaternion(body,q3);

}

void SimplifiedDynamicManta::release(Vec3f orientation)
{
    Vehicle::inert = false;

    angularPos[0] = getAzimuthRadians(orientation);
}


void SimplifiedDynamicManta::setOrientation(Vec3f orientation)
{
    dMatrix3 R1,R2;
    dRSetIdentity(R1);
    dRSetIdentity(R2);

    dRFromAxisAndAngle(R1,0,1,0,getAzimuthRadians(orientation));

    dRFromAxisAndAngle(R2,1,0,0,getDeclination(orientation)*PI/180.0f);

    dQuaternion q1,q2,q3;
    dQfromR(q1,R1);

    dQfromR(q2,R2);

    dQMultiply0(q3,q2,q1);

    if (!Vehicle::inert)
        dBodySetQuaternion(me,q3);
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

        float gamma = 0;
//std::cout << "Azimuth:" << getAzimuth(getForward()) << "- Declination: " << getDeclination(getForward()) << std::endl;

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

        float Ml = -rudder*0.1  + gamma*0.1;

        Vec3f Ft;
        Ft[0]=0;Ft[1]=0;Ft[2]=getThrottle();

        Ft[0] = Ft[0] + forcesOnBody[0];
        Ft[1] = Ft[1] + forcesOnBody[1];
        Ft[2] = Ft[2] + forcesOnBody[2];

        dBodyAddRelForce(body, Ft[0],Ft[1],Ft[2]);

        dBodyAddTorque(body, 0.0f, Ml, 0.0f);

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

