#include "SimplifiedManta.h"


void SimplifiedManta::doControl(Controller controller)
{
    //engine[0] = -controller.roll;
    //engine[1] = controller.yaw;
    //engine[2] = -controller.pitch;
    //steering = -controller.precesion;


    setThrottle(-controller.registers.thrust*2*5);

    // roll
    Manta::aileron = controller.registers.roll;

    // pitch
    Manta::elevator = controller.registers.pitch;

    //Manta::rudder = controller.precesion*0.1;

    Manta::rudder = controller.registers.precesion;

    for(int i=0;i<10;i++)
    {
        if (controller.param[i]!=0)
            Manta::param[i] = controller.param[i];
    }

}

EntityTypeId SimplifiedManta::getTypeId()
{
    return EntityTypeId::TSimplifiedManta;
}


void SimplifiedManta::doDynamics(dBodyID body)
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


    dBodySetQuaternion(body,q3);
    //dBodySetRotation(body,R);


    speed += getThrottle() / 100.0f;

    pos += speed * forward;
    dBodySetPosition(body,pos[0],pos[1],pos[2]);

    dVector3 result;

    dBodyVectorToWorld(body, 0,0,1,result);
    setForward(result[0],result[1],result[2]);

    const dReal *dBodyPosition = dBodyGetPosition(body);
    const dReal *dBodyRotation = dBodyGetRotation(body);


    setPos(dBodyPosition[0],dBodyPosition[1],dBodyPosition[2]);
    setLocation((float *)dBodyPosition, (float *)dBodyRotation);


}


