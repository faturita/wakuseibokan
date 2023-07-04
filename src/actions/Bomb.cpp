#include <unordered_map>
#include "../ThreeMaxLoader.h"
#include "Bomb.h"

extern std::unordered_map<std::string, GLuint> textures;

Bomb::Bomb(int faction)
{
    Vehicle::setTtl(1500);
    setFaction(faction);
}

Bomb::~Bomb()
{
    dGeomDestroy(geom);
    dBodyDestroy(me);

    assert(0 || !"Destroying bullets from the Gunshot object. This should not happen now.");
}


void Bomb::init()
{
    _model = (Model*)T3DSModel::loadModel("units/bomb2.3ds",0,0,0,1,0);

    length = 4.12f;
    height = 2.06f;
    width  = 2.02f;

    // @NOTE: Hack to allow the bomb to hit many targets at the same time.
    length = 20.12f;
    height = 2.06;
    width  = 20.02f;

    mass = 1.0f;

    setDamage(10000);

    setName("Bomb");

    setForward(0,0,1);

    visible = true;
}

void Bomb::drawModel(float yRot, float xRot, float x, float y, float z)
{
    float f[3];
    f[0] = 0; f[1] = 0; f[2] = 0;

    if (visible)
    {
        glPushMatrix();
        glTranslatef(x,y,z);

        doTransform(f,R);

        glScalef(1.0f,1.0f,1.0f);

        doMaterial();

        //drawRectangularBox(width, height, length);
        _model->setTexture(textures["metal"]);
        _model->draw();

        glPopMatrix();
    }
}

void Bomb::doMaterial()
{
    GLfloat specref[] = { 1.0f, 1.0f, 1.0f, 1.0f};

    glEnable(GL_COLOR_MATERIAL);

    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    glMaterialfv(GL_FRONT, GL_SPECULAR, specref);
    glMateriali(GL_FRONT, GL_SHININESS,128);
}

void  Bomb::getViewPort(Vec3f &Up, Vec3f &position, Vec3f &viewforward)
{
    position = getPos();
    viewforward = toVectorInFixedSystem(0,0,1,0, -0);                         // I dont care the declination for the viewport

    // ViewForward is in body coordinates, I need to convert it to global coordinates.
    viewforward = toWorld(me, viewforward);

    Up = toVectorInFixedSystem(0.0f, 1.0f, 0.0f,0,0);

    Vec3f orig;


    viewforward = viewforward.normalize();
    orig = position;
    Up[0]=Up[2]=0;Up[1]=40;
    position = position - 40*viewforward + Up;
    viewforward = orig-position;
}

int Bomb::getType()
{
    return EXPLOTABLEACTION;
}

EntityTypeId Bomb::getTypeId()
{
    return EntityTypeId::TBomb;
}
