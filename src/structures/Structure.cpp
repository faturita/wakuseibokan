#include <unordered_map>
#include "Structure.h"

extern std::unordered_map<std::string, GLuint> textures;

Structure::Structure()
{
    island = NULL;
    azimuth=0;
    elevation=0;
}

Structure::Structure(int faction)
{
    Structure();
    setFaction(faction);
}

Structure::~Structure()
{
    if (_model != NULL)
            delete _model;
}

void Structure::init()
{
    //Load the model
    _model = (Model*)T3DSModel::loadModel("structures/structure.3ds",160.99f,-19.48f,76.36f,1,textures["sky"]);
    if (_model != NULL)
    {

    }

    Structure::height=50;
    Structure::length=8;
    Structure::width=8;

    setName("Tower");

    setForward(0,0,1);
}

void Structure::setPos(const Vec3f &newpos)
{
    pos[0] = newpos[0];
    pos[1] = newpos[1];
    pos[2] = newpos[2];

    dGeomSetPosition(geom, pos[0], pos[1], pos[2]);
}
void Structure::setPos(float x, float y, float z)
{
    pos[0] = x;
    pos[1] = y;
    pos[2] = z;

    dGeomSetPosition(geom, pos[0], pos[1], pos[2]);
}

void  Structure::doDynamics(dBodyID) {
    // Nothing to do
}
void  Structure::doDynamics()
{
    doDynamics(getBodyID());
}



void Structure::onIsland(Island *island)
{
    Structure::island = island;
}



void Structure::embody(dWorldID world, dSpaceID space)
{
    geom = dCreateBox(space, Structure::width, Structure::height, Structure::length);
    dGeomSetPosition(geom, pos[0], pos[1], pos[2]);
}

void Structure::embody(dBodyID myBodySelf)
{
    assert(!"Structures are fixed and do not have a movable body.");
}

void Structure::rotate(float yawangle)
{
    // Use the yawangle to set a quaternion for the geom.  This will rotate yawangle in radians
    // the model.
    dMatrix3 R1;
    dRSetIdentity(R1);

    dRFromAxisAndAngle(R1,0,1,0,yawangle);

    dQuaternion q1;
    dQfromR(q1,R1);

    dGeomSetQuaternion(geom,q1);


    // Now, get the position and orientation from the model (it will be the model ratated)
    const dReal *dBodyPosition = dGeomGetPosition(geom);
    const dReal *dBodyRotation = dGeomGetRotation(geom);

    Vec3f newpos(dBodyPosition[0], dBodyPosition[1], dBodyPosition[2]);

    // Set the position and the orientation matrix.
    setPos(dBodyPosition[0],dBodyPosition[1],dBodyPosition[2]);
    setLocation((float *)dBodyPosition, (float *)dBodyRotation);

    dVector3 result;

    // Set the forward direction.
    dGeomVectorToWorld(geom, 0,0,1,result);
    setForward(result[0],result[1],result[2]);

}




void Structure::doControl(Controller controller)
{
    Structure::elevation = controller.registers.pitch;
    Structure::azimuth = controller.registers.roll;
}

Vec3f Structure::getForward()
{
    //Vec3f forward = toVectorInFixedSystem(0, 0, 1,Structure::azimuth,Structure::elevation);
    return forward;
}

void Structure::getViewPort(Vec3f &Up, Vec3f &position, Vec3f &fwd)
{
    position = getPos();
    fwd = toVectorInFixedSystem(0, 0, 1,Structure::azimuth,Structure::elevation);
    Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    Vec3f orig;

    fwd = fwd.normalize();
    orig = position;
    Up[0]=Up[2]=0;Up[1]=4;// 4 is a good value to be just above the shoulders, like watching somebodys cellphone on the train
    position = position - 5*fwd + Up;
    fwd = orig-position;
}

bool Structure::checkHeightOffset(int heightOffset)
{
    // Return TRUE if the height is valid for this structure
    return (heightOffset >= 4);
}

void Structure::drawModel()
{
    drawModel(0,0,pos[0],pos[1],pos[2]);
}

void Structure::drawModel(float yRot, float xRot, float x, float y, float z)
{
    float f[3];
    f[0] = 0; f[1] = 0; f[2] = 0;

    //Draw the saved model
    if (_model != NULL)
    {
        glPushMatrix();
        glTranslatef(x, y, z);

        glScalef(1.0f,1.0f,1.0f);

        doTransform(f,R);

        _model->draw(textures["sky"]);
        //drawRectangularBox(Structure::width, Structure::height, Structure::length);

        glPopMatrix();
    }
    else
    {
        printf ("model is null\n");
    }
}


TickRecord Structure::serialize()
{
    TickRecord t = Vehicle::serialize();

    t.orientation = getAzimuthRadians(getForward());

    return t;
}

int Structure::getType()
{
    return COLLISIONABLE;
}

int Structure::getSubType()
{
    return STRUCTURE;
}

EntityTypeId Structure::getTypeId()
{
    return EntityTypeId::TStructure;
}
