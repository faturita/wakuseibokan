#include "Explosion.h"
#include "../container.h"

extern container<Vehicle*> entities;

Explosion::Explosion()
{
    Vehicle::setTtl(150);
}

void Explosion::expand(float height, float length, float width, float dense, dWorldID world, dSpaceID space)
{
    Vec3f loc = getPos();

    float stride = 0.5;

    for(int i=-dense;i<dense;i++)
    {
        for (int j=-dense;j<dense;j++)
        {
            for (int h=-dense;h<dense;h++)
            {
                Debris* b1 = new Debris();
                b1->init();
                b1->embody(world, space);
                b1->setPos(loc[0]+i*stride,loc[1]+h*stride,loc[2]+j*stride);
                b1->stop();

                Vec3f targ;
                targ = -loc + b1->getPos();
                targ = targ*1;

                //targ = Vec3f(loc[0], loc[1]+(-dense)*stride, loc[2]) - b1->getPos();
                //targ = targ*10;


                dBodyAddForce(b1->getBodyID(), targ[0],targ[1],targ[2]);

                entities.push_back(b1, b1->getGeom());
            }
        }
    }
}
