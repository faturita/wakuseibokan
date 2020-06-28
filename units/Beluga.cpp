#include "Beluga.h"
#include "SimplifiedDynamicManta.h"
#include "Walrus.h"
#include "../ThreeMaxLoader.h"
#include "../sounds/sounds.h"

extern GLuint _textureRoad;

extern std::vector<std::string> messages;
extern std::vector<BoxIsland*> islands;


Beluga::Beluga(int faction) : Balaenidae(faction)
{

}

void Beluga::init()
{
    //Load the model
    _model = (Model*)T3DSModel::loadModel("units/beluga.3ds",-1.4f,0.0f,0.0f,1,_textureRoad);

    setForward(0,0,1);

}


void Beluga::doControl()
{
    Controller c;

    c.registers = myCopy;


    Vec3f Po = getPos();

    Po[1] = 0.0f;

    Vec3f Pf(145 kmf, -1.0f, 89 kmf - 3.5 kmf);

    Pf = destination;

    Vec3f T = Pf - Po;

    float eh, midpointpitch;


    if (!reached && T.magnitude()>500)
    {
        float distance = T.magnitude();

        Vec3f F = getForward();

        F = F.normalize();
        T = T.normalize();


        // Potential fields from the islands (to avoid them)

        int nearesti = 0;
        float closest = 0;
        for(int i=0;i<islands.size();i++)
        {
            BoxIsland *b = islands[i];
            Vec3f l(b->getX(),0.0f,b->getZ());

            if ((l-Po).magnitude()<closest || closest ==0) {
                closest = (l-Po).magnitude();
                nearesti = i;
            }
        }

        c.registers.thrust = 1500.0f;

        if (distance<10000.0f)
        {
            c.registers.thrust = 800.0f;
        }

        if (distance<2000.0f)
        {
            c.registers.thrust = 150.0f;
        }

        if (closest > 1800 && closest < 4000)
        {
            BoxIsland *b = islands[nearesti];
            Vec3f l = b->getPos();
            Vec3f d = Po-l;

            d = d.normalize();

            T = T+d;
            T = T.normalize();

            c.registers.thrust = 45.0f;
        }



        float e = acos(  T.dot(F) );

        float signn = T.cross(F) [1];


        printf("T: %10.3f, %10.3f %10.3f %10.3f\n", closest, distance, e, signn);

        if (abs(e)>=0.5f)
        {
            c.registers.roll = 30.0 * (signn>0?+1:-1) ;
        } else
        if (abs(e)>=0.4f)
        {
            c.registers.roll = 20.0 * (signn>0?+1:-1) ;
        } else
        if (abs(e)>=0.2f)
            c.registers.roll = 10.0 * (signn>0?+1:-1) ;
        else {
            c.registers.roll = 0.0f;
        }


    } else {
        if (!reached)
        {
            char str[256];
            sprintf(str, "Balaenidae has arrived to destination.");
            messages.insert(messages.begin(), str);
            reached = true;
            c.registers.thrust = 0.0f;
            c.registers.roll = 0.0f;
            disableAuto();
        }
    }

    Balaenidae::doControl(c);
}


void Beluga::embody(dWorldID world, dSpaceID space)
{
    me = dBodyCreate(world);
    embody(me);
    geom = dCreateBox( space, 100.0f, 58.0f, 500.0f);   // scale 50
    dGeomSetBody(geom, me);

}

void Beluga::embody(dBodyID myBodySelf)
{
    dMass m;

    float myMass = 250.0f;

    dBodySetPosition(myBodySelf, pos[0], pos[1], pos[2]);
    //dMassSetBox(&m,1,1.0f, 4, 5.0f);
    dMassSetBox(&m, 1,100.0f, 58.0f, 500.0f);
    //dMassSetSphere(&m,1,radius);
    dMassAdjust(&m, myMass*1.0f);
    dBodySetMass(myBodySelf,&m);

    me = myBodySelf;

}

void Beluga::drawModel(float yRot, float xRot, float x, float y, float z)
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
        //drawArrow(V[0],V[1],V[2],0.0,1.0,0.0);

        //drawRectangularBox(100.0f/2.0f, 58.0f/2.0f, 500.0f/2.0f);

        glRotatef(-180.0f, 1.0f, 0.0f, 0.0f);
        glRotatef(-180.0f, 0.0f, 0.0f, 1.0f);
        glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
        glRotatef(-180.0f, 0.0f, 1.0f, 0.0f);
        glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);

        //glRotatef(yRot, 0.0f, 0.0f, 1.0f);

        //glRotatef(xRot, 1.0f, 0.0f, 0.0f);

        _model->draw();

        //glTranslatef(-0.4f,0.62f,-0.5f);
        //drawTheRectangularBox(_textureRoad,8.0f, 1.0f, 1.0f);



        glPopMatrix();
    }
    else
    {
        printf ("model is null\n");
    }
}

Vehicle* Beluga::spawn(dWorldID  world,dSpaceID space,int type, int number)
{
    Vehicle *v;

    if (type == MANTA)
    {
        SimplifiedDynamicManta *_manta1 = new SimplifiedDynamicManta(getFaction());
        _manta1->init();
        _manta1->setNumber(number);
        _manta1->embody(world, space);
        _manta1->setPos(pos[0],pos[1]+28, pos[2]);
        _manta1->setStatus(Manta::ON_DECK);
        _manta1->inert = true;
        alignToMe(_manta1->getBodyID());
        v = (Vehicle*)_manta1;
    } else if (type == WALRUS)
    {
        Walrus *_walrus = new Walrus(getFaction());
        _walrus->init();
        _walrus->setNumber(number);
        _walrus->embody(world,space);
        Vec3f p;
        p = p.normalize();
        p = getForward()*450;
        _walrus->setPos(pos[0]-p[0]-140*(number+1),pos[1]-p[1]+1,pos[2]-p[2]);
        _walrus->setStatus(Walrus::SAILING);
        _walrus->stop();
        _walrus->inert = true;
        dBodyAddRelForce(me,10.0f,0.0f,0.0f);

        alignToMe(_walrus->getBodyID());
        v = (Vehicle*)_walrus;
    }

    return v;
}

//draw3DSModel("units/beluga.3ds",1200.0+100,15.0,700.0+300.0,1,_textureBox);
