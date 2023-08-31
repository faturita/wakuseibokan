#include "Weapon.h"


Weapon::Weapon(int faction)
{
    setFaction(faction);
}

Weapon::~Weapon()
{
    if (joint)
    {
        dJointDisable(joint);
        dJointDestroy(joint);
    }
}

void Weapon::init()
{

    Image* image = loadBMP("terrain/vtr.bmp");
    _textureBox = loadTexture(image);
    delete image;

    Weapon::width = 7.0;
    Weapon::height = 7.0;
    Weapon::length = 7.0;
    
    setForward(0,0,1);
    
}

int Weapon::getType()
{
    return WEAPON;
}

int Weapon::getSubType()
{
    return STRUCTURE;
}

EntityTypeId Weapon::getTypeId()
{
    return EntityTypeId::TWeapon;
}

void Weapon::setPos(const Vec3f &newpos)
{
    pos[0] = newpos[0];
    pos[1] = newpos[1];
    pos[2] = newpos[2];

    dGeomSetPosition(geom, pos[0], pos[1], pos[2]);
}
void Weapon::setPos(float x, float y, float z)
{
    pos[0] = x;
    pos[1] = y;
    pos[2] = z;

    dGeomSetPosition(geom, pos[0], pos[1], pos[2]);
}

void Weapon::doDynamics(dBodyID body)
{
    me = body;
    doDynamics();
}

void Weapon::doDynamics()
{
    wrapDynamics(me);
}

void Weapon::stop()
{
    // @NOTE: ODE is extraordinary, but if you do not use it exactly as they suggest (using motors to drive forces) there could appear
    //    many issues.  I've found that I need to provide a stop function for the weapons (attached to other units) because if there happen
    //    to be some problem with the unit, ODE halts and the simulation is stopped.
    Vehicle::stop();
}



void Weapon::embody(dWorldID world, dSpaceID space)
{
    me = dBodyCreate(world);
    embody(me);
    geom = dCreateBox(space, width, height, length);
    dGeomSetBody(geom, me);
}


void Weapon::embody(dBodyID myBodySelf)
{
	dMass m;

    float myMass = 1.0f;

	dBodySetPosition(myBodySelf, pos[0], pos[1], pos[2]);
    dMassSetBox(&m,1,width, height, length);
	dMassAdjust(&m, myMass*1.0f);
	dBodySetMass(myBodySelf,&m);
    
    me = myBodySelf;
    
}

void Weapon::attachTo(dWorldID world, Vehicle *attacher, float x, float y, float z)
{
    // @NOTE, it takes the position of the attacher and creates a fixed joint between them.  The joint is created in the world, in the zero group.
    setPos(attacher->getPos()[0]+x, attacher->getPos()[1]+y, attacher->getPos()[2]+z);

    joint = dJointCreateFixed(world,0);
    dJointAttach (joint,attacher->getBodyID(), getBodyID());

    dJointSetFixed(joint);     // @NOTE: SetFixed is mandatory, otherwise the objects share the same center.
    dJointSetFixedParam(joint,dParamSuspensionERP,0.0f );
    dJointSetFixedParam(joint,dParamSuspensionCFM, 0.0f);

}

dBodyID Weapon::getBodyID()
{
    return me;
}


void Weapon::doControl(Controller controller)
{
    Weapon::elevation = controller.registers.pitch;
    Weapon::azimuth = controller.registers.roll;
}

void Weapon::getViewPort(Vec3f &Up, Vec3f &position, Vec3f &fwd)
{
    position = getPos();
    fwd = toVectorInFixedSystem(0, 0, 1,Weapon::azimuth,Weapon::elevation);
    Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    Vec3f orig;

    fwd = fwd.normalize();
    orig = position;
    Up[0]=Up[2]=0;Up[1]=4;// 4 is a good value to be just above the shoulders, like watching somebodys cellphone on the train
    position = position - 40*fwd + Up;
    fwd = orig-position;
}

void Weapon::drawModel()
{
    drawModel(0,0,pos[0],pos[1],pos[2]);
}

void  Weapon::drawModel(float yRot, float xRot, float x, float y, float z)

{

    x=0, y=0, z=0;
    
    float xx = pos[0];
    float yy = pos[1];
    float zz = pos[2];
    
    const float BOX_SIZE = length; //The length of each side of the cube
    
    //glLoadIdentity();
    glPushMatrix();
    glTranslatef(xx,yy,zz);
    
    
    
    // This will Rotate according to the R quaternion (which is a variable in Vehicle).
    float f[3];
    f[0] = 0; f[1] = 0; f[2] = 0;
    
    doTransform(f, R);
    
    glRotatef(0, 0.0f, 0.0f, 1.0f);
    glBegin(GL_QUADS);
    
    //Top face
    glColor3f(1.0f, 1.0f, 0.0f);
    glNormal3f(0.0, 1.0f, 0.0f);
    glVertex3f(-BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, -BOX_SIZE / 2 + z);
    glVertex3f(-BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
    glVertex3f(BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
    glVertex3f(BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, -BOX_SIZE / 2 + z);
    
    //Bottom face
    glColor3f(1.0f, 0.0f, 1.0f);
    glNormal3f(0.0, -1.0f, 0.0f);
    glVertex3f(-BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, -BOX_SIZE / 2 + z);
    glVertex3f(BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, -BOX_SIZE / 2 + z);
    glVertex3f(BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, BOX_SIZE /2 + z);
    glVertex3f(-BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
    
    //Left face
    glNormal3f(-1.0, 0.0f, 0.0f);
    glColor3f(0.0f, 1.0f, 1.0f);
    glVertex3f(-BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, -BOX_SIZE / 2 + z);
    glVertex3f(-BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
    glVertex3f(-BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, -BOX_SIZE / 2 + z);
    
    //Right face
    glNormal3f(1.0, 0.0f, 0.0f);
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, -BOX_SIZE / 2 + z);
    glVertex3f(BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, -BOX_SIZE / 2 + z);
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
    glVertex3f(BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
    
    glEnd();
    
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, _textureBox);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glColor3f(1.0f, 1.0f, 1.0f);
    
    glBegin(GL_QUADS);
    
    //Front face
    glNormal3f(0.0, 0.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(BOX_SIZE / 2 + x, -BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-BOX_SIZE / 2 + x, BOX_SIZE / 2 + y, BOX_SIZE / 2 + z);
    
    //Back face
    glNormal3f(0.0, 0.0f, -1.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-BOX_SIZE / 2+ x, -BOX_SIZE / 2+ y, -BOX_SIZE / 2+ z);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(-BOX_SIZE / 2+ x, BOX_SIZE / 2+ y, -BOX_SIZE / 2+ z);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(BOX_SIZE / 2+ x, BOX_SIZE / 2+ y, -BOX_SIZE / 2+ z);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(BOX_SIZE / 2+ x, -BOX_SIZE / 2+ y, -BOX_SIZE / 2+ z);
    
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
    
    //boxangle += 0.5f;
    
    //if (boxangle >= 360.f)
      //  boxangle = 0.0f;
}

Vec3f Weapon::getForward()
{
    return forward;
}


