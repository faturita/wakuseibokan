#include <iomanip>

#include "SimplifiedDynamicManta.h"
#include "../profiling.h"
#include "../control.h"
#include "../actions/Gunshot.h"
#include "../actions/Bomb.h"
#include "../actions/Missile.h"
#include "../actions/AAM.h"
#include "../actions/Torpedo.h"
#include "../math/yamathutil.h"
#include "../messages.h"

extern dWorldID world;
extern dSpaceID space;
#include "../container.h"
#include "../sounds/sounds.h"

extern container<Vehicle*> entities;
extern std::vector<Message> messages;

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
    switch (autostatus) {
    case AutoStatus::DOGFIGHT:
        doControlDogFight();
        break;
    case AutoStatus::ATTACK:
        doControlAttack();
        break;
    case AutoStatus::LANDING:
        doControlLanding();
        break;
    case AutoStatus::WAYPOINT:
        doControlWaypoint();
        break;
    case AutoStatus::FREE:
        goTo(getPos()+getForward().normalize()*100);
        setAutoStatus(AutoStatus::DESTINATION);
        break;
    default:case AutoStatus::DESTINATION:
        //doControlDestination();// @FIXME
        doControlControl2(destination, 1000, 1000);
        break;
    }
}

Vec3f mp(Vec3f pos)
{
    return Vec3f(pos[0],1000,pos[2]);
}



void SimplifiedDynamicManta::doControlDogFight()
{

    // Approach to target until I am at certain range.
    // Follow the target increasing speed if I am trailing behind or decreasing it if I am too close.
    // When in range, Aim with flipping increasing speed, shooting with all you have.
    // If starts to trail behind go 2
    // Fly away and restarts going to 1.
    dout << "DF:" << std::setw(3) << getName() << std::setw(11) << destination << std::setw(3) << flyingstate << std::endl;

    // @NOTE: Someone will give me, all the time, current target position.
    Vec3f target = destination;
    switch (flyingstate) {
        default:case 0:// Approach
        {
            doControlControl2(target,10000, 720);
            dout << (destination-getPos()).magnitude() << std::endl;
            if ((destination-getPos()).magnitude()<9000)
                flyingstate = 1;
        }
        break;
        case 1:// Engage
        {
            target = destination;
            doControlFlipping(target, 1000);

            // I am trailing behind, chase it.
            if ((destination-getPos()).magnitude()>9000)
                flyingstate = 0;

            // I am too close, restart.
            if ((destination-getPos()).magnitude()<300)
                flyingstate = 2;

            // I am too low, restart
            if (getPos()[1] < 150)
                flyingstate = 2;

            // Open fire copiously
            if (getTtl() % 5 == 0)
            {
                Vehicle *action = fire(0,world,space);

                if (action != NULL)
                {
                    entities.push_at_the_back(action, action->getGeom());
                    gunshot();
                }
            }
        }
        break;
    case 2:// Restart
        waypoint = mp(destination) + mp(getForward().normalize()*(20000));
        waypoint[1] = 1000;
        flyingstate = 3;
        break;
    case 3:
        doControlControl2(waypoint,10000, 720);
        if (((waypoint-getPos()).magnitude()<5000))
                flyingstate = 0;
        break;

    }


    //dout << "Azimuth:" << getAzimuth(getForward()) << "- Declination: " << getDeclination(getForward())  << "- Destination:" << (destination-getPos()).magnitude() << std::endl;


}

void SimplifiedDynamicManta::doControlAttack()
{

    dout << "A:" << std::setw(3) << getName() << std::setw(11) << destination << std::setw(3) << flyingstate << std::endl;

    Vec3f target = destination;
    switch (flyingstate) {
        default:case 0:
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
                Vehicle *action = fire(0,world,space);

                if (action != NULL)
                {
                    entities.push_at_the_back(action,action->getGeom());
                    gunshot();
                }
            }
        }
        break;
    case 2:
        waypoint = mp(destination) + mp(getForward().normalize()*(20000));
        waypoint[1] = 1000;
        flyingstate = 3;
        break;
    case 3:
        doControlControl2(waypoint,10000, 720);
        if (((waypoint-getPos()).magnitude()<5000))
                flyingstate = 4;
        break;
    case 4:
        waypoint = mp(target);
        waypoint[1] = 1000;
        flyingstate = 5;
        break;
    case 5:
        doControlControl2(waypoint,10000, 720);
        if (((waypoint-getPos()).magnitude()<12000))
                flyingstate = 0;
        break;

    }


    //dout << "Azimuth:" << getAzimuth(getForward()) << "- Declination: " << getDeclination(getForward())  << "- Destination:" << (destination-getPos()).magnitude() << std::endl;


}

void SimplifiedDynamicManta::doControlForced(Vec3f target)
{
    Controller c;

    c.registers = registers;

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

    dout << "Number:" << getName() << "-Azimuth:" << getAzimuth(getForward()) << "- Declination: " << getDeclination(getForward())  << "- Destination:" << T.magnitude() << std::endl;

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

    c.registers = registers;

    Vec3f Po = getPos();

    float height = Po[1];

    float declination = getDeclination(getForward());

    float sp2=-0,        sp3 = 720;

    Vec3f T = (target - Po);

    sp2 = getDeclination(T);

    float e1 = _acos(  T.normalize().dot(getForward().normalize()) );
    float e2 = sp2 - declination;
    float e3 = sp3 - height;

    if (isnan(e1)) e1=0.0;

    // Needs fixing, check azimuth to make a continuos function.

    float Kp1 = 0.42,    Kp2 = 2.98,     Kp3 = 0.2;
    float Ki1 = 1,      Ki2 = 0.11,        Ki3 = 0;
    float Kd1 = 0.8,      Kd2 = 0.03,        Kd3 = 0;

    float I[3]= {0,0,0};

    float e[3] = { e1, e2, e3};

    getIntegrativeTerm(errserie,I,e);

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

//    dout << "T:Az:"
//              << std::setw(10) << getAzimuth(getForward())
//              << std::setw(10) << getAzimuth(T)
//              << "(" << std::setw(12) << e1 << ")"
//              << std::setw(10) << declination
//              << std::setw(10) << sp2
//              << " Destination:"
//              << std::setw(10) << T.magnitude() << std::endl;

    et1 = e1;
    et2 = e2;
    et3 = e3;

    c.registers.thrust = thrust/(10.0);
    c.registers.yaw = 0;
    c.registers.roll = 0;
    setThrottle(thrust);

    doControl(c);
}

void SimplifiedDynamicManta::doHold(Vec3f target, float thrust)
{
    Controller c;

    c.registers = registers;

    float height = getPos()[1];

    elevator = 35;

    c.registers.thrust = 300.0f/(10.0);

    midpointpitch = 36;
    float eh=height-500.0f;

    c.registers.roll = -13;

    if ((abs(eh))>10.0f)
    {
        c.registers.pitch = midpointpitch+1.0 * (eh>0 ? -1 : +1);
    } else {
        c.registers.pitch = midpointpitch;
    }

    setThrottle(30.0f);

    setStatus(FlyingStatus::HOLDING);

    if (dst_status != DestinationStatus::REACHED)
    {   
        char msg[256];
        Message mg;
        sprintf(msg, "%s has arrived to destination.", getName().c_str());
        mg.faction = getFaction();
        mg.msg = std::string(msg);
        messages.insert(messages.begin(), mg);
        dst_status = DestinationStatus::REACHED;
        setStatus(FlyingStatus::HOLDING);
    }

    c.registers.yaw = 0;

    doControl(c);

}

void SimplifiedDynamicManta::doControlControl2(Vec3f target, float thrust, float sp_height, float threshold)
{

    Controller c;

    c.registers = registers;

    Vec3f Po = getPos();

    float height = Po[1];
    float declination = getDeclination(getForward());

    float sp2=-0,        sp3 = sp_height;

    Vec3f T = (target - Po);

    CLog::Write(CLog::Debug,"T: %10.3f\t\t", T.magnitude());dout << target << std::endl;


    if (!(dst_status != DestinationStatus::REACHED && (T.magnitude()>threshold && map(T).magnitude()>threshold)))
    {
        doHold(target, thrust);
        return;
    }

    if (map(T).magnitude()<3500) thrust = 400.0;


    sp2 = getDeclination(T);

    // This function returns principal arc cosine of x, in the interval [0, pi] radians.
    float e1 = _acos(  T.normalize().dot(getForward().normalize()) );
    float e2 = sp2 - declination;
    float e3 = sp3 - height;


    printf( "E1: %10.5f\n" , e1 );


    // Set the sign of e1 in relation to rolling encoding.
    // Negative: Clockwise
    // Positive: Counterclockwise
    // The arccos of the dot product is always positive (@NOTE: verify me) so I need to add
    // the correct sign for the controller to work.  So
    // if T(arget) is at the left is negative, as long as the difference between them is not greater than 180
    // if Forward is in the fourth quadrant, only do it negative is T is around the first quadrant.
    if (getAzimuth(getForward())>270 && getAzimuth(T)<(getAzimuth(getForward())-180))
        e1 = e1 * (-1);
    else if (getAzimuth(getForward()) < getAzimuth(T) && (getAzimuth(T) - getAzimuth(getForward()))<180)
        e1 = e1 * (-1);


    float Kp1 = 1.2,        Kp2 = 0.3,          Kp3 = 1.2;
    float Ki1 = 0.9,        Ki2 = 0.6,          Ki3 = 0.6;
    float Kd1 = 2.3,        Kd2 = 0,            Kd3 = 11.8;


    float e[3] = { e1, e2, e3 };


    float r1 =  rt1 + Kp1 * (e1 - et1)        + Ki1 * (e1 + et1)/2.0 + Kd1 * (e1 - 2 * et1 + ett1);
    float r2 =  rt2 + Kp2 * (e2 - et2)        + Ki2 * (e2 + et2)/2.0 + Kd2 * (e2 - 2 * et2 + ett2);
    float r3 =  rt3 + Kp3 * (e3 - et3)        + Ki3 * (e3 + et3)/2.0 + Kd3 * (e3 - 2 * et3 + ett3);

    if (isnan(e1)) e1=0.0;

//        dout << "--T:Az:"
//                  << std::setw(10) << getAzimuth(getForward())
//                  << std::setw(10) << getAzimuth(T)
//                  << "(" << std::setw(12) << e1 << ")"
//                  << std::setw(10) << declination
//                  << std::setw(10) << sp2
//                  << " Destination:"
//                  << std::setw(10) << T.magnitude() << std::endl;

    r1 = clipmax(r1, 5);
    r1 = clipmin(r1, -5);
    r2 = clipmax(r2, 40);
    r2 = clipmin(r2, -40);
    r3 = clipmax(r3, 20);
    r3 = clipmin(r3, -20);



    rt1 = r1;
    rt2 = r2;
    rt3 = r3;


    float roll = -r1;
    float pitch = -r2*0 + r3;

    //float thrust = (e3>800? e3 :800);

    ett1 = et1;
    ett2 = et2;
    ett3 = et3;

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

    c.registers = registers;

    Vec3f Po = getPos();

    float height = Po[1];
    float declination = getDeclination(getForward());

    float sp2=-0,        sp3 = 720;

    Vec3f T = (target - Po);

    sp2 = getDeclination(T);

    // Needs fixing, check azimuth to make a continuos function.

    float e1 = _acos(  T.normalize().dot(getForward().normalize()) );
    float e2 = sp2 - declination;
    float e3 = sp3 - height;



    float Kp1 = 0.42,    Kp2 = 2.98,     Kp3 = 0.2;
    float Ki1 = 0.1,      Ki2 = 0.11,        Ki3 = 0;
    float Kd1 = 0.003,      Kd2 = 0.03,        Kd3 = 0;

    float I[3]= {0,0,0};

    float e[3] = { e1, e2, e3 };

    getIntegrativeTerm(errserie,I,e);

    float In[3] = {0,0,0};

    In[0] = I[0] * 0.99 + (e1);
    In[1] = I[1] * 0.99 + (e2);
    In[2] = I[2] * 0.99 + (e3);

    float error1 =  Kp1 * (e1)        + Ki1 * (In[0]) + Kd1 * (  (e1)-et1);
    float error2 =  Kp2 * (e2)        + Ki2 * (In[1]) + Kd2 * (  (e2)-et2);
    float error3 =  Kp3 * (e3)        + Ki3 * (In[1]) + Kd3 * (  (e3)-et3);

    dout << "Azimuth:" << getAzimuth(getForward()) << "/" << error1 << "- Declination: " << declination << "/" << sp2 << " Destination:" << T.magnitude() << std::endl;

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

void SimplifiedDynamicManta::doControlWaypoint(float threshold)
{
    if (dst_status == DestinationStatus::STILL)
    {
        if (!waypoints.empty()) destination = waypoints.front();waypoints.pop();
        dst_status = DestinationStatus::TRAVELLING;
    }
    if (dst_status == DestinationStatus::REACHED && !waypoints.empty())
    {
        destination = waypoints.front();waypoints.pop();
        dst_status = DestinationStatus::TRAVELLING;
    }

    doControlControl2(destination, 500, destination[1], threshold);

}

void SimplifiedDynamicManta::land(Vec3f landplace, Vec3f placelattitude)
{
    autostatus = AutoStatus::LANDING;
    setStatus(FlyingStatus::FLYING);
    setDestination(landplace);
    setAttitude(placelattitude);
    flyingstate=0;


    Vec3f Po = getPos();

    float height = Po[1];

    Po[1] = 0.0f;

    Vec3f Pf;

    Pf = landplace;  // This is the carrier or a runway.@NOTE: The carrier DOES NOT move while Mantas are landing.
    Pf[1] = 0;

    placelattitude[1]=0;

    Vec3f trans = placelattitude.rotateOnY(PI/2.0);

    Vec3f w5 = (Pf);
    Vec3f w4 = (Pf - placelattitude.normalize()*(05 kmf)) ;
    Vec3f w3 = (Pf - placelattitude.normalize()*(10 kmf)) ;
    Vec3f w2 = (Pf - (placelattitude.normalize()*(20 kmf) + trans.normalize()*(5 kmf))) ;
    Vec3f w1 = (Pf - (placelattitude.normalize()*(10 kmf) + trans.normalize()*(10 kmf))) ;


    w1[1] = 500;
    w2[1] = 250;
    w3[1] = 200;
    w4[1] = 190;
    w5[1] = 180;

    dout << w1 << w2 << w3 << w4 << std::endl;

    clearWaypoints();

    addWaypoint(w1);
    addWaypoint(w2);
    addWaypoint(w3);
    addWaypoint(w4);
    addWaypoint(w5);
    dst_status = DestinationStatus::STILL;
    enableAuto();
}

void SimplifiedDynamicManta::doControlLanding()
{
    if (waypoints.empty() && dst_status == DestinationStatus::REACHED)
    {
        Controller c;

        c.registers = registers;

        c.registers.thrust = 25.0f/(10.0);
        c.registers.pitch = 0;
        c.registers.roll = 0;
        setThrottle(0.0f);

        doControl(c);
    } else {
        doControlWaypoint(300.0);
    }
}

/**
void SimplifiedDynamicManta::doControlLanding()
{
    Controller c;

    c.registers = registers;

    Vec3f Po = getPos();

    float height = Po[1];

    Po[1] = 0.0f;

    Vec3f Pf(-100 kmf, 0.0f, 100 kmf);

    Pf = destination;  // This is the carrier or a runway.  If it is the carrier this must be modified during each tick.
    Pf[1] = 0;

    // Go 10 kmf backwards from the carrier, very slowly.
    Vec3f T;
    float H=500, spspeed = 40.0f, TH = 200;

    attitude[1]=0;

    switch (flyingstate) {
    case 0:default:
        T = (Pf - attitude.normalize()*(5 kmf)) - Po;
        H=450;spspeed = 40.0f;TH = 150;
        break;
    case 1:
        H=180;spspeed = 35.0f; TH=250;
        T = (Pf - attitude.normalize()*(0 kmf)) - Po;
        break;
    }


    float eh, midpointpitch, distance;

    if (dst_status != DestinationStatus::REACHED && T.magnitude()>TH)
    {
        float Kp = -0.8;
        float val = ((getAzimuth(getForward())-180.0f)*PI/180.0f - (getAzimuth(T)-180.0f)*PI/180.0f);

        distance =  T.magnitude();
        c.registers.roll = 0;

        setForward(T);
        release(T);


        CLog::Write(CLog::Debug,"Landing:T: %10.3f %10.3f %10.3f %10.3f vs %10.3f: %10.3f\n", distance, val, c.registers.roll, getAzimuth(attitude), getAzimuth(getForward()),_acos(  attitude.normalize().dot(getForward().normalize()) ));


        eh = height-H;
        float spd = spspeed;
        midpointpitch = 2;

        if ((abs(eh))>5.0f)
        {
            c.registers.pitch = midpointpitch+2.0 * (eh>0 ? -1 : +1);
            spd = spspeed+5.0 * (eh>0 ? -1 : +1);
        } else {
            c.registers.pitch = midpointpitch;
            spd = spspeed;
        }
        setThrottle(spd * 10.0f);
        c.registers.thrust = spd;
    } else
    {
        if (flyingstate<1)
            flyingstate += 1;
        else
        {
            float e1 = _acos(  attitude.normalize().dot(getForward().normalize()) );
            if (e1>0.2)
            {
                // Pull back, wrong attitude.
                flyingstate = 0;
                doControl(c);
                return;
            }
            //stop();
            elevator = 0;

            c.registers.thrust = 0.0f/(10.0);
            c.registers.pitch = 0;
            c.registers.roll = 0;
            setThrottle(0.0f);

            if (dst_status != DestinationStatus::REACHED)
            {
                flyingstate = 0;
                char msg[256];
                //Message mg;
                //sprintf(msg, "Manta %d has arrived to destination.", getNumber()+1);
                //mg.faction = getFaction();
                //mg.msg = std::string(msg);
                //messages.insert(messages.begin(), mg);
                dst_status = DestinationStatus::REACHED;
                setStatus(FlyingStatus::HOLDING);
            }
        }
    }

    doControl(c);

}
**/

void SimplifiedDynamicManta::doControlDestination()
{
    Controller c;

    c.registers = registers;

    Vec3f Po = getPos();

    float height = Po[1];

    Po[1] = 0.0f;

    Vec3f Pf(-100 kmf, 0.0f, 100 kmf);

    Pf = destination;

    Vec3f T = Pf - Po;

    float eh, midpointpitch;


    if (dst_status != DestinationStatus::REACHED && T.magnitude()>precission)
    {
        float distance = T.magnitude();

        Vec3f F = getForward();

        F = F.normalize();
        T = T.normalize();


        float e = _acos(  T.dot(F) );

        float signn = T.cross(F) [1];


        CLog::Write(CLog::Debug,"T: %10.3f %10.3f %10.3f\n", distance, e, signn);

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
        CLog::Write(CLog::Debug,"Manta arrived to destination...\n");

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

        setStatus(FlyingStatus::HOLDING);

        if (dst_status != DestinationStatus::REACHED)
        {
            flyingstate = 0;
            char msg[256];
            Message mg;
            sprintf(msg, "%s has arrived to destination.", getName().c_str());
            mg.faction = getFaction();
            mg.msg = std::string(msg);
            messages.insert(messages.begin(), mg);
            dst_status = DestinationStatus::REACHED;
            setStatus(FlyingStatus::HOLDING);
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
        setStatus(FlyingStatus::TACKINGOFF);
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

    registers = regs;

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

/**void SimplifiedDynamicManta::land()
{
    autostatus = AutoStatus::LANDING;
    setStatus(FlyingStatus::FLYING);
    flyingstate=0;
}**/

void SimplifiedDynamicManta::attack(Vec3f target)
{
    autostatus = AutoStatus::ATTACK;
    destination = target;
}

void SimplifiedDynamicManta::hold()
{
    autostatus = AutoStatus::FREE;
}

void SimplifiedDynamicManta::enableAuto()
{
    Vehicle::enableAuto();
    flyingstate=0;
    dst_status == DestinationStatus::STILL;
}

void SimplifiedDynamicManta::dogfight(Vec3f target)
{
    autostatus = AutoStatus::DOGFIGHT;
    destination = target;
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

    //dout << "Bearing:" << getBearing() << "," << x << " Anglular Pos:" << angularPos[0] << std::endl;

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
//dout << "Azimuth:" << getAzimuth(getForward()) << "- Declination: " << getDeclination(getForward()) << std::endl;

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

        //CLog::Write(CLog::Debug,"%8.4f\t%8.4f\t%6.4f\t",speed, alpha*180.0/PI, beta*180.0/PI);
        //dout << Ft << std::endl;
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

        CLog::Write(CLog::Debug,"%8.4f\t%8.4f\t%6.4f\t%6.4f\t",speed, alpha*180.0/PI, beta*180.0/PI, gamma*180.0/PI);
        //dout << Ft << std::endl;
        dout << std::endl;
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


Vehicle* SimplifiedDynamicManta::fire(int weapon, dWorldID world, dSpaceID space)
{

    switch (weapon) {
        case 0:default:return fireAmmo(world,space);
            break;
        case 1:return fireMissile(world, space);
            break;
        case 2:return fireBomb(world,space);
            break;
        case 3:return fireAAM(world, space);
            break;
        case 4:return fireTorpedo(world, space);
            break;

    }

}

Vehicle* SimplifiedDynamicManta::fireTorpedo(dWorldID world, dSpaceID space)
{
    if (getTtl()>0)
        return NULL;

    Torpedo *action = new Torpedo(getFaction());
    // Need axis conversion.
    action->init();
    action->setOrigin(me);

    Vec3f position = getPos();
    position[1] += .5f; // Move upwards to the center of the real rotation.
    Vec3f fw = getForward();
    Vec3f Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    Vec3f orig;

    fw = fw.normalize();
    orig = position;
    position = position + 80.0f*fw;
    fw = -orig+position;

    position[1] =1.0;
    action->embody(world,space);
    action->setPos(position[0],position[1],position[2]);


    dMatrix3 R,R2;
    dRSetIdentity(R);
    dRFromEulerAngles (R, 0,0,0);

    dQuaternion q1,q2,q3;
    dQfromR(q1,R);
    dRFromAxisAndAngle(R2,0,1,0,getAzimuthRadians(getForward()));

    dQfromR(q2,R2);


    dQMultiply0(q3,q2,q1);


    Vec3f Ft=fw + Vec3f(0,20,0);
    Ft=Ft*60;

    //dBodyAddForce(action->getBodyID(), Ft[0],Ft[1],Ft[2]);
    dBodyAddRelForce(action->getBodyID(),0,0,60);
    dBodySetQuaternion(action->getBodyID(),q3);

    setTtl(1000);

    // I can set power or something here.
    return (Vehicle*)action;
}


Vehicle* SimplifiedDynamicManta::fireAAM(dWorldID world, dSpaceID space)
{
    if (getTtl()>0)
        return NULL;

    AAM *action = new AAM(getFaction());
    // Need axis conversion.
    action->init();
    action->setOrigin(me);

    Vec3f position = getPos();
    position[1] += .5f; // Move upwards to the center of the real rotation.
    Vec3f fw = getForward();
    Vec3f Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    Vec3f orig;

    fw = fw.normalize();
    orig = position;
    position = position + 60.0f*fw;
    fw = -orig+position;


    position = orig;
    position[1] += 40;
    action->embody(world,space);
    action->setPos(position[0],position[1],position[2]);


    dMatrix3 R,R2;
    dRSetIdentity(R);
    dRFromEulerAngles (R, 0,0,
                      0);

    dQuaternion q1,q2,q3;
    dQfromR(q1,R);
    dRFromAxisAndAngle(R2,0,1,0,getAzimuthRadians(getForward()));

    dQfromR(q2,R2);


    dQMultiply0(q3,q2,q1);


    Vec3f Ft=fw + Vec3f(0,20,0);
    Ft=Ft*1;

    dBodyAddForce(action->getBodyID(), Ft[0],Ft[1],Ft[2]);
    dBodySetQuaternion(action->getBodyID(),q3);

    //setTtl(1000);

    // I can set power or something here.
    return (Vehicle*)action;
}

Vehicle* SimplifiedDynamicManta::fireMissile(dWorldID world, dSpaceID space)
{
    if (getTtl()>0)
        return NULL;

    Missile *action = new Missile(getFaction());
    // Need axis conversion.
    action->init();
    action->setOrigin(me);

    Vec3f position = getPos();
    position[1] += .5f; // Move upwards to the center of the real rotation.
    Vec3f fw = getForward();
    Vec3f Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    Vec3f orig;

    fw = fw.normalize();
    orig = position;
    position = position + 60.0f*fw;
    fw = -orig+position;


    position = orig;
    //position[1] += 40;
    action->embody(world,space);
    action->setPos(position[0],position[1],position[2]);


    dMatrix3 R,R2;
    dRSetIdentity(R);
    dRFromEulerAngles (R, 0,0,
                      0);

    dQuaternion q1,q2,q3;
    dQfromR(q1,R);
    dRFromAxisAndAngle(R2,0,1,0,getAzimuthRadians(getForward()));

    dQfromR(q2,R2);


    dQMultiply0(q3,q2,q1);


    Vec3f Ft=fw + Vec3f(0,20,0);
    Ft = fw;
    Ft=Ft*250;

    dBodyAddForce(action->getBodyID(), Ft[0],Ft[1],Ft[2]);
    dBodySetQuaternion(action->getBodyID(),q3);

    //setTtl(1000);

    // I can set power or something here.
    return (Vehicle*)action;
}

Vehicle* SimplifiedDynamicManta::fireBomb(dWorldID world, dSpaceID space)
{
    Bomb *action = new Bomb(GREEN_FACTION);
    action->init();
    action->setOrigin(me);

    Vec3f position = getPos();
    position[1] -= 20;

    action->embody(world, space);
    action->setPos(position[0], position[1], position[2]);

    forward = getForward();
    forward = forward.normalize();

    Vec3f f1(0.0,0.0,1.0);
    Vec3f f2 = forward.cross(f1);
    f2 = f2.normalize();
    float alpha = _acos( forward.dot(f1)/(f1.magnitude()*forward.magnitude()));

    dMatrix3 Re;
    dRSetIdentity(Re);
    dRFromAxisAndAngle(Re,f2[0],f2[1],f2[2],-alpha);

    Vec3f v = dBodyGetLinearVelVec(me);

    dBodySetLinearVel(action->getBodyID(),v[0],v[1],v[2]);
    dBodySetRotation(action->getBodyID(),Re);

    // I can set power or something here.
    return (Vehicle*)action;

}
Vehicle* SimplifiedDynamicManta::fireAmmo(dWorldID world, dSpaceID space)
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

EntityTypeId SimplifiedDynamicManta::getTypeId()
{
    return EntityTypeId::TSimplifiedDynamicManta;
}
