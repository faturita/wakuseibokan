//
//  Weapon.cpp
//

#include "Weapon.h"


Weapon::Weapon(int faction)
{
    setFaction(faction);
}

void Weapon::init()
{

    Image* image = loadBMP("vtr.bmp");
    _textureBox = loadTexture(image);
    delete image;
    
    setForward(0,0,1);
    
}

int Weapon::getType()
{
    return WALRUS;
}


void Weapon::embody(dWorldID world, dSpaceID space)
{
    me = dBodyCreate(world);
    embody(me);
    geom = dCreateBox(space, 7.0f,7.0f,7.0f);
    dGeomSetBody(geom, me);
}


void Weapon::embody(dBodyID myBodySelf)
{
	dMass m;

    float myMass = 1.0f;
    float length = 7.0f;

	dBodySetPosition(myBodySelf, pos[0], pos[1], pos[2]);
    dMassSetBox(&m,1,length,length,length);
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
    
    const float BOX_SIZE = 7.0f; //The length of each side of the cube
    
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

void Weapon::drawDirectModel()
{

}

void Weapon::doDynamics(dBodyID body)
{
    me = body;
    doDynamics();
}

void Weapon::stop()
{

}

void Weapon::doDynamics()
{


    wrapDynamics(me);
    
}
