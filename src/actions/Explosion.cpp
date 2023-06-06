#include "../math/yamathutil.h"
#include "../profiling.h"
#include "../container.h"
#include "Explosion.h"

extern container<Vehicle*> entities;

Explosion::Explosion()
{
    Vehicle::setTtl(150);
}

void Explosion::expand(float height, float length, float width, float dense, dWorldID world, dSpaceID space)
{
    Vec3f loc = getPos();

    float stride = 0.5;

    // Calculate the strides to distribuite all the explosion along the size of the bounding box of the exploding object.
    float stride_width =  max( (width / 2.0) / dense , stride );
    float stride_height = max( (height / 2.0) / dense , stride );
    float stride_length = max( (length / 2.0) / dense, stride );

    //dout << "Dimensions:" << Vec3f(width, height, length) << std::endl;
    //dout << "Strides: " << stride_width << "," << stride_height << "," << stride_length << std::endl;

    Vec3f particledim(0.5f,0.5f,0.5f);

    // This is a performance restriction to limit the size of each particle to 10^3.
    if (stride_width>=10 && stride_height >=10 && stride_length>=10)
        particledim = Vec3f(10.0f, 10.0f, 10.0f);

    for(int w=-dense;w<dense;w++)
    {
        for (int l=-dense;w<dense;w++)
        {
            for (int h=-dense;h<dense;h++)
            {
                Debris* b1 = new Debris();
                b1->init(particledim);
                b1->setTexture(texture);
                b1->embody(world, space);
                Vec3f randloc = Vec3f((rand() % 10 -5 +1)*0.1,(rand() % 10 -5 +1)*0.1,(rand() % 10 -5 +1)*0.1);
                Vec3f strideloc = Vec3f(loc[0]+w*stride_width,loc[1]+h*stride_height,loc[2]+l*stride_length);
                b1->setPos(randloc + strideloc);
                b1->stop();

                Vec3f targ;
                targ = -loc + b1->getPos();
                targ = targ*1;

                // This is another alternative.
                //targ = Vec3f(loc[0], loc[1]+(-dense)*stride, loc[2]) - b1->getPos();
                //targ = targ*10;

                // Set a radial force.
                dBodyAddForce(b1->getBodyID(), targ[0],targ[1],targ[2]);

                entities.push_back(b1, b1->getGeom());
            }
        }
    }
}


EntityTypeId Explosion::getTypeId()
{
    return EntityTypeId::TExplosion;
}
