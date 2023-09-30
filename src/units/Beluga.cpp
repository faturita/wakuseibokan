#include "Beluga.h"
#include "SimplifiedDynamicManta.h"
#include "Stingray.h"
#include "Walrus.h"
#include "Cephalopod.h"
#include "../profiling.h"
#include "../ThreeMaxLoader.h"
#include "../sounds/sounds.h"
#include "../keplerivworld.h"

extern std::unordered_map<std::string, GLuint> textures;
extern std::vector<Message> messages;
extern std::vector<BoxIsland*> islands;


Beluga::Beluga(int faction) : Balaenidae(faction)
{

}

void Beluga::init()
{
    //Load the model
    _model = (Model*)T3DSModel::loadModel("units/beluga.3ds",-1.4f,0.0f,0.0f,1,textures["boat"]);

    setForward(0,0,1);

    setName("Beluga");


    width = 100;
    height = 58;
    length = 500;
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
    Vehicle *v = NULL;

    if (type == CEPHALOPOD)
    {
        Cephalopod *c = new Cephalopod(getFaction());
        c->init();
        c->setNameByNumber(number);
        c->embody(world, space);
        c->setPos(pos[0],pos[1]+38, pos[2]);
        c->setStatus(FlyingStatus::ON_DECK);
        c->inert = false;
        alignToMe(c->getBodyID());
        v = (Vehicle*)c;

    } else if (type == MANTA)
    {
        Stingray *_manta1 = new Stingray(getFaction());
        _manta1->init();
        _manta1->setNameByNumber(number);
        _manta1->embody(world, space);
        _manta1->setPos(pos[0],pos[1]+28, pos[2]);
        _manta1->setStatus(FlyingStatus::ON_DECK);
        _manta1->inert = true;
        alignToMe(_manta1->getBodyID());
        v = (Vehicle*)_manta1;
    } else if (type == WALRUS)
    {
        Walrus *_walrus = new Walrus(getFaction());
        _walrus->init();
        _walrus->setNameByNumber(number);
        _walrus->embody(world,space);
        Vec3f p;
        p = getForward().normalize()*(getDimensions()[2]/2.0+30.0);
        _walrus->setPos(pos[0]-p[0],pos[1]-p[1]+1,pos[2]-p[2]);
        //_walrus->setPos(pos[0]-p[0]-140*(number+1),pos[1]-p[1]+1,pos[2]-p[2]);
        _walrus->setStatus(SailingStatus::SAILING);
        _walrus->stop();
        _walrus->inert = true;
        dBodyAddRelForce(me,10.0f,0.0f,0.0f);

        p = getForward().normalize()*(1000);
        p = p.rotateOnY(getRandom(-PI/2.0+PI/4.0, PI/2.0 - PI/4));
        p = getPos()-p;

        _walrus->goTo(p);
        _walrus->enableAuto();

        dMatrix3 Re1,Re2,Re3;
        dQuaternion q1,q2,q3;
        dBodyCopyRotation(me,Re1);
        dRFromAxisAndAngle(Re2,0.0,1.0,0.0,PI);
        dQfromR(q1,Re1);
        dQfromR(q2,Re2);
        dQMultiply0(q3,q1,q2);
        dRfromQ(Re3,q3);
        dBodySetRotation(_walrus->getBodyID(),Re3);

        v = (Vehicle*)_walrus;
    }

    return v;
}

int Beluga::getSubType()
{
    return BELUGA;
}

EntityTypeId Beluga::getTypeId()
{
    return EntityTypeId::TBeluga;
}

//draw3DSModel("units/beluga.3ds",1200.0+100,15.0,700.0+300.0,1,_textureBox);
