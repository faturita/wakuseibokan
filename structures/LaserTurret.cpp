#include "Laserturret.h"

#include "../actions/LaserBeam.h"

LaserTurret::LaserTurret()
{
    //setEnableAI();

}

void LaserTurret::drawModel(float yRot, float xRot, float x, float y, float z)
{
    float f[3];
    f[0] = 0; f[1] = 0; f[2] = 0;

    //Draw the saved model
    if (_model != NULL)
    {
        glPushMatrix();
        glTranslatef(x, y, z);

        glScalef(1.0f,1.0f,1.0f);
        //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        _model->draw(Structure::texture);
        //drawRectangularBox(Structure::width, Structure::height, Structure::length);

        glTranslatef(0.0f,27.97f-8.140f,0.0f);

        glRotatef(270.0f, 0.0f, 1.0f, 0.0f);

        glRotatef(-Structure::azimuth,0.0f,1.0f,0.0f);
        glRotatef(-Structure::inclination,0.0f,0.0f,1.0f);

        draw3DSModel("structures/turrettop.3ds",0.0f,0.0f,0.0f,1,Structure::texture);

        // Gun shots
        //glTranslatef((firing+=100),0.0f,0.0f);
        //drawArrow(100.0f,0.0f,0.0f,0.0,1.0,0.0);


        // Laser Beam
        //if (firing)
        //    drawArrow(10000.0f,0.0f,0.0f,0.0,1.0,0.0);

        //glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        glPopMatrix();

        /**glPushMatrix();
        glLoadIdentity();
        glLoadMatrixf(modelview);
        glTranslatef((firing+=50),0.0f,0.0f);
        drawArrow(100.0f,0.0f,0.0f,1.0,0.0,0.0);

        glPopMatrix();**/
    }
    else
    {
        printf ("model is null\n");
    }
}
Vehicle* LaserTurret::fire(dWorldID world, dSpaceID space)
{
    LaserBeam *action = new LaserBeam();
    // Need axis conversion.
    action->init();



    Vec3f position = getPos();
    position[1] += 19.0f; // Move upwards to the center of the real rotation.
    forward = getForward();
    Vec3f Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    Vec3f orig;


    forward = forward.normalize();
    orig = position;
    position = position + 20*forward;
    forward = -orig+position;

    Vec3f Ft = forward*100;

    Vec3f f1(0.0,0.0,1.0);
    Vec3f f2 = forward.cross(f1);
    f2 = f2.normalize();
    float alpha = acos( forward.dot(f1)/(f1.magnitude()*forward.magnitude()));

    dMatrix3 Re;
    dRSetIdentity(Re);
    dRFromAxisAndAngle(Re,f2[0],f2[1],f2[2],-alpha);


    action->setPos(position[0],position[1],position[2]);
    action->embody(world,space);
    dBodySetLinearVel(action->getBodyID(),Ft[0],Ft[1],Ft[2]);
    dBodySetRotation(action->getBodyID(),Re);

    // I can set power or something here.
    return (Vehicle*)action;
}

void LaserTurret::doControl()
{
    Controller c;

    c.registers = myCopy;

    c.registers.roll = 1;
    if ((rand() % 100 + 1)<10)
        firing = !firing;

    Turret::doControl(c);
}
